/*****************************************************************************
* incline_display.c for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

//#define AO_InclineDisplay

#include "qpn_port.h"
#include "bsp.h"
#include "incline_display.h"
#include "lcd.h"
#include "mpu6050.h"

enum RideState ride_state;
volatile RideInfo ride_info;

//float incline_queue[];
//float ride_array[];
//float display_ride_array[ARRAY_PLOT_LENGTH];

volatile float cur_incline;
int raw_incline_index;
float raw_incline_array[NUM_INCLINE_AVG];


mpu_data_t mpu_data_raw;

const char* rideStateToString(enum RideState state);
void UpdateRideArray(float incline);
void UpdateRideInfo(float incline);




typedef struct InclineDisplayTag  {               //InclineDisplay State machine
	QActive super;
}  InclineDisplay;


/* Setup state machines */
/**********************************************************************/
static QState InclineDisplay_initial (InclineDisplay *me);
static QState InclineDisplay_on      (InclineDisplay *me);
static QState InclineDisplay_Home_View  (InclineDisplay *me);
static QState InclineDisplay_Ride_View  (InclineDisplay *me);

/**********************************************************************/


InclineDisplay AO_InclineDisplay;



void InclineDisplay_ctor(void)  {
	InclineDisplay *me = &AO_InclineDisplay;
	QActive_ctor(&me->super, (QStateHandler)&InclineDisplay_initial);

	//Initialize Variables
	ride_state = RIDE_OFF;
	cur_incline = 0.0;
	raw_incline_index = 0;

	xil_printf("\nVariables Initialized");

}


QState InclineDisplay_initial(InclineDisplay *me) {
	xil_printf("\n\rInitialization");
	// Initialize MPU 6050
	int Status = mpu_init();

	if (Status != XST_SUCCESS) {
		xil_printf("MPU Initialization Failed\r\n");
	}

    return Q_TRAN(&InclineDisplay_on);
}

QState InclineDisplay_on(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("\n\rPlaceHolder");
			}
		// case READ_I2C: {
		// 	mpu_data_raw = get_mpu_data();
		// 	//cur_incline = raw_data.accel_x;
		// 	QActive_postISR((QActive *)&AO_InclineDisplay, GET_INCLINE);


		// 	// int read_count = mpu_raw_data_struct.i2c_read_counter += 1; // Increment counter
		// 	// if (read_count > NUM_INCLINE_AVG){
		// 	// 	QActive_postISR((QActive *)&AO_InclineDisplay, GET_INCLINE);
		// 	// }
		// 	return Q_HANDLED();
		// }
		case TOGGLE_RIDE: {
			if (ride_state == RIDE_ON){
				ride_state = RIDE_PAUSE;
			}
			else {
				ride_state = RIDE_ON;
			}
			displayRideState(rideStateToString(ride_state));
			return Q_HANDLED();

		}
		case RESET_RIDE: {
			ride_state = RIDE_OFF;
			// Clear all values to 0
			displayRideState(rideStateToString(ride_state));
			memset(&ride_info, 0, sizeof(RideInfo));
			QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
			return Q_HANDLED();
		}
		case GET_INCLINE: {
			//cur_incline = mpu_data_raw.accel_x;

			//Get and compute the incline from the MPU
			mpu_data_raw = get_mpu_data();
			float raw_incline = computeIncline(mpu_data_raw, 0.12);

			//Save Incline to array
			raw_incline_array[raw_incline_index] = raw_incline;
			raw_incline_index += 1;

			//Return If not ready to take average
			if(raw_incline_index < NUM_INCLINE_AVG){
				return Q_HANDLED();
			}

			//Set cur_incline based on average all raw_incline_array of size NUM_INCLINE_AVG
			cur_incline = 0.0f;
            for (int i = 0; i < NUM_INCLINE_AVG; i++) {
                cur_incline += raw_incline_array[i];
            }
            cur_incline /= NUM_INCLINE_AVG;		

			//If in a ride currently add the incline to ride
			if (ride_state == RIDE_ON){
				UpdateRideArray(cur_incline);
				UpdateRideInfo(cur_incline);
			}


			QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_INCLINE);
			return Q_HANDLED();
		}
			
		case Q_INIT_SIG: {
			return Q_TRAN(&InclineDisplay_Home_View);
		}
	}
	
	return Q_SUPER(&QHsm_top);
}


/* Create InclineDisplay_on state and do any initialization code if needed */
/******************************************************************/

QState InclineDisplay_Home_View(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("ENTRY: Startup State Home\n");
			displayHomeBackground();
			displayHomeIncline(cur_incline);
			displayRideState(rideStateToString(ride_state));
			return Q_HANDLED();
		}
		case TOGGLE_VIEW: {
			return Q_TRAN(&InclineDisplay_Ride_View);
		}
		case UPDATE_INCLINE: {
			displayHomeIncline(cur_incline);
			return Q_HANDLED();
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}

QState InclineDisplay_Ride_View(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("ENTRY: Startup State Ride\n");
			displayRideBackground();
			displayRideInfo(ride_info);
			displayRideArrayPlot(display_ride_array, ride_info);
			displayRideCurIncline(cur_incline);
			displayRideState(rideStateToString(ride_state));

			return Q_HANDLED();
		}
		case TOGGLE_VIEW: {
			return Q_TRAN(&InclineDisplay_Home_View);
		}
		case UPDATE_INCLINE: {
			displayRideCurIncline(cur_incline);
			return Q_HANDLED();
		}
		case UPDATE_RIDE: {
			displayRideArrayPlot(display_ride_array, ride_info);
			//displayRideInfo(ride_info);
			updateRideInfo(ride_info);
			displayRideCurIncline(ride_state);
			return Q_HANDLED();
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}


/* Helper Functions */
/**********************************************************************/

#include <stdint.h>
#include <stdbool.h>

#define QUEUE_SIZE 20
#define RIDE_ARRAY_SIZE 2 * ARRAY_PLOT_LENGTH

// Global variables
float display_ride_array[ARRAY_PLOT_LENGTH];

static float incline_queue[QUEUE_SIZE];
static float ride_array[RIDE_ARRAY_SIZE];
static int queue_head = 0, queue_count = 0;
static int ride_array_count = 0, temp_counter = 0;
static int insert_array_count = 0, insert_array_interval = 5;
static int update_time = 50;

// Circular buffer enqueue
void enqueue(float *queue, int *head, int *count, int size, float value) {
    queue[*head] = value;
    *head = (*head + 1) % size;
    *count = (*count < size) ? (*count + 1) : size; // Cap count at max size
}

// Calculate average of a fixed-size buffer
float calculate_average(float *array, int count) {
    float sum = 0;
    for (int i = 0; i < count; i++) {
        sum += array[i];
    }
    return (count > 0) ? (sum / count) : 0;
}

// Main function to update ride array
void UpdateRideArray(float incline) {
    temp_counter++;

    bool update_UI = (temp_counter > update_time);

    if (insert_array_count < insert_array_interval) {
        insert_array_count++;
        enqueue(incline_queue, &queue_head, &queue_count, QUEUE_SIZE, incline);
    } else {
        insert_array_count = 0;
        float new_incline = calculate_average(incline_queue, queue_count);
        queue_count = 0; // Reset queue

        // Add to ride array
        if (ride_array_count < RIDE_ARRAY_SIZE) {
            ride_array[ride_array_count++] = new_incline;
        }

        // Check if ride_array needs downsampling
        if (ride_array_count > 2 * ARRAY_PLOT_LENGTH) {
            int step = ride_array_count / ARRAY_PLOT_LENGTH;
            float new_ride_array[ARRAY_PLOT_LENGTH];
            for (int i = 0, j = 0; j < ARRAY_PLOT_LENGTH; i += step, j++) {
                new_ride_array[j] = calculate_average(&ride_array[i], step);
            }
            for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
                ride_array[i] = new_ride_array[i];
            }
            ride_array_count = ARRAY_PLOT_LENGTH;
            insert_array_interval++;
        }
    }

    if (update_UI) {
        temp_counter = 0;

        // Update the display ride array logic
        // Use only the last ARRAY_PLOT_LENGTH elements of ride_array
        float display_ride_array[ARRAY_PLOT_LENGTH];
        for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
            int idx = ride_array_count - ARRAY_PLOT_LENGTH + i;
            display_ride_array[i] = (idx >= 0) ? ride_array[idx] : 0;
        }

        // Send update to UI (replace with actual function call)
        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
    }
}

/*
int temp_counter = 0;
int update_time = 10;
void UpdateRideArray(float incline) {
	//xil_printf("\nTODO: UpdateRideArray");
	//xil_printf("\nTODO: UpdateRideArray");
	temp_counter += 1;
	int update_UI = temp_counter > update_time;

	//If insert_array_count <= insert_array_interval
		//insert_array_count += 1
		//add incline to incline_queue

	//else
		//insert_array_count = 0
		//new_incline = average of incline_queue

		//add new_incline to ride_array

		//if ride_array size > ARRAY_PLOT_LENGTH

			//if ride_array size > x2 ARRAY_PLOT_LENGTH
				//average all ride_array to size ARRAY_PLOT_LENGTH
				//insert_array_interval += 1

			//if update_ui
				//set display_ride_array to the ARRAY_PLOT_LENGTH values at the end of the array

		//else if ride_array size < ARRAY_PLOT_LENGTH

			//mult = ARRAY_PLOT_LENGTH % ride_array_size + 1

			//if update_ui
				//for size of ride_array
					//add ride_array to display_ride_array mult times until run out of space in display_ride_array (working backwards)

		//Note** Make sure to convert floats to int

	//If enough time has passed, to update ride UI
	if(update_UI){
		temp_counter = 0;

		//Create display_ride_away with correct history of ride

		//Send To Update UI
		QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
	}
	return;
}
*/

void UpdateRideInfo(float incline){
	// Update the minimum incline
	if (cur_incline < ride_info.min_incline || ride_info.insert_array_count == 0) {
		ride_info.min_incline = cur_incline;
	}

	// Update the maximum incline
	if (cur_incline > ride_info.max_incline || ride_info.insert_array_count == 0) {
		ride_info.max_incline = cur_incline;
	}

	// Update the average incline
	ride_info.insert_array_count++; // Increment the count
	//ride_info.average_incline = ((ride_info.average_incline * (ride_info.insert_array_count - 1)) + cur_incline) / ride_info.insert_array_count;

	//xil_printf("\nTODO: UpdateRideInfo");
	return;
}

const char* rideStateToString(enum RideState state) {
    switch (state) {
        case RIDE_OFF:
            return "OFF";
        case RIDE_ON:
            return "ON";
        case RIDE_PAUSE:
            return "PAUSE";
        default:
            return "UNKNOWN";
    }
}

