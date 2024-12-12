/*****************************************************************************
* incline_display.c for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

//#define AO_InclineDisplay
#include <stdint.h>
#include <stdbool.h>
#include "qpn_port.h"
#include "bsp.h"
#include "incline_display.h"
#include "lcd.h"
#include "mpu6050.h"
#include "xtmrctr.h"

//Range of Degrees for cur_incline
#define INCLINE_MAX 90
#define INCLINE_MIN -90

// I2C MPU data and Kalman Filter to Incline
float cur_incline;
mpu_data_t mpu_data_raw;
volatile uint32_t last_compute_time = 0; // Time of the last computation in timer ticks
extern XTmrCtr timer;

//Ride Variables
enum RideState ride_state;
RideInfo ride_info;

//Filter Variables
enum FilterState filter_state;

// Used To update Ride Plot with total Inclin History Array
#define QUEUE_SIZE 256
#define RIDE_ARRAY_SIZE 2 * ARRAY_PLOT_LENGTH
static float incline_queue[QUEUE_SIZE];
static float ride_array[RIDE_ARRAY_SIZE];
static int queue_head = 0, queue_count = 0;
static int ride_array_count = 0, temp_counter = 0;
static int insert_array_count = 0, insert_array_interval = 1;
static int update_time = 50;
float display_ride_array[ARRAY_PLOT_LENGTH];

// Used To update Ride Plot with Current Incoming Incline
float current_inclines[ARRAY_PLOT_LENGTH];
int current_incline_index = 0;

//Encoder Sensitivity Control
#define SENSITIVITY_LUT_LENGTH 6
int sensitivity_num = 3;
float sensitivity_alpha;
float sensitivity_lut[SENSITIVITY_LUT_LENGTH] = {
	0.0005f,
	0.005f,
	0.05f,
	0.25f,
	0.5f,
	1.0f,
};


//Helper Functions
void handle_get_incline();
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
	filter_state = FILTER_ON;
	cur_incline = 0.0;
	sensitivity_alpha = sensitivity_lut[sensitivity_num -1];

	for(int i = 0; i < ARRAY_PLOT_LENGTH; i++){
		current_inclines[i] = 0.0;
	}

	for(int i = 0; i < RIDE_ARRAY_SIZE; i++) {
		ride_array[i] = 0.0;
	}



	xil_printf("\nVariables Initialized");

}

QState InclineDisplay_initial(InclineDisplay *me) {
	xil_printf("\n\rInitialization");
	// Initialize MPU 6050
	int Status = mpu_init();

	if (Status != XST_SUCCESS) {
		xil_printf("MPU Initialization Failed\r\n");
	}
	//ride_state = RIDE_OFF;
	//xil_printf("RIDE_STATE\r\n");


    return Q_TRAN(&InclineDisplay_on);
}

QState InclineDisplay_on(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("\n\Display on");
	        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_INCLINE);


			return Q_HANDLED();
		}
		case TOGGLE_RIDE: {
			//xil_printf("\n\rRAN TOGGLE");
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
			displayRideState(rideStateToString(ride_state));

//			//Show The History Of The Last ride
//			for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
//				xil_printf("\n\r %d: %d", i, (int)ride_array[i]);
//			}
//			for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
//				int idx = ride_array_count - ARRAY_PLOT_LENGTH + i;
//				display_ride_array[i] = (idx >= 0) ? ride_array[idx] : 0;
//			}


			memset(&display_ride_array, 0, sizeof(display_ride_array));

			//Delete and Reset Current Ride information
			memset(&ride_info, 0, sizeof(RideInfo));

			//Reset Ride Plot Variables
			memset(&incline_queue, 0, sizeof(incline_queue));
			memset(&ride_array, 0, sizeof(ride_array));
			memset(&current_inclines, 0, sizeof(current_inclines));

			queue_head = 0;
			queue_count = 0;
			ride_array_count = 0;
			temp_counter = 0;
			insert_array_count = 0;
			insert_array_interval = 1;
			update_time = 50;

			current_incline_index = 0;

			QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
			return Q_HANDLED();
		}
//		case GET_INCLINE: {
//			//xil_printf("\n\In Get Incline");
//
//			//cur_incline = mpu_data_raw.accel_x;
//
//			// Added by NB to track actual time between Kalman Filter operations
//			// Get the current time in timer ticks
//			uint32_t current_time = XTmrCtr_GetValue(&timer, 0);
//
//			// Calculate the time difference in seconds (as a float)
//			float dt = (float)(current_time - last_compute_time) / (float)XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ;
//
//			// Update the last computation time
//			last_compute_time = current_time;
//
//
//			//Get and compute the incline from the MPU
//			mpu_data_raw = get_mpu_data();
//			float raw_incline = computeIncline(mpu_data_raw, dt);
//
//			if(raw_incline > INCLINE_MAX){
//				raw_incline = INCLINE_MAX;
//			}
//			else if(raw_incline < INCLINE_MIN){
//				raw_incline = INCLINE_MIN;
//			}
//
//            // Calculate smoothed incline
//			cur_incline = sensitivity_alpha * raw_incline + (1 - sensitivity_alpha) * cur_incline;
//
//			//If in a ride currently add the incline to ride
//			if (ride_state == RIDE_ON){
//				UpdateRideArray(cur_incline);
//				UpdateRideInfo(cur_incline);
//			}
//
//
//			QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_INCLINE);
//			return Q_HANDLED();
//		}
		case ENCODER_DOWN: {
			if(filter_state == FILTER_ON){
				if (sensitivity_num > 1){
					sensitivity_num -= 1;
					sensitivity_alpha = sensitivity_lut[sensitivity_num -1 ];
				}
				displayInclineSensitivity(sensitivity_num);
			}
			return Q_HANDLED();
		}
		case ENCODER_UP: {
			if(filter_state == FILTER_ON){
				if (sensitivity_num < SENSITIVITY_LUT_LENGTH){
					sensitivity_num += 1;
					sensitivity_alpha = sensitivity_lut[sensitivity_num - 1];
				}
				displayInclineSensitivity(sensitivity_num);
			}
			return Q_HANDLED();
		}
		case TOGGLE_FILTER: {
			if (filter_state == FILTER_ON){
				filter_state = FILTER_OFF;
				displayUnfiltered();
			}
			else{
				filter_state = FILTER_ON;
				displayInclineSensitivity(sensitivity_num);
			}
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
			xil_printf("\nEntry: Home State");
			displayHomeBackground();
			displayHomeIncline(cur_incline);
			displayRideState(rideStateToString(ride_state));
			displayInclineSensitivity(sensitivity_num);
			displayInclineSlopeStart(cur_incline);

			//NB testing incline display
			//displayInclineSlope(cur_incline);
			return Q_HANDLED();
		}
		case TOGGLE_VIEW: {
			return Q_TRAN(&InclineDisplay_Ride_View);
		}
		case UPDATE_INCLINE: {
			//xil_printf("\n\IN UPDATE INCLINE");
			handle_get_incline();
			displayHomeIncline(cur_incline);
			displayInclineSlope(cur_incline);
	        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_INCLINE);

			return Q_HANDLED();
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}

QState InclineDisplay_Ride_View(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("\nENTRY: Ride State");
			displayRideBackground();
			displayRideInfo(ride_info);
			displayRideArrayPlotStart(display_ride_array, ride_info);
			displayRideCurIncline(cur_incline);
			displayRideState(rideStateToString(ride_state));
			displayInclineSensitivity(sensitivity_num);
			return Q_HANDLED();
		}
		case TOGGLE_VIEW: {
			return Q_TRAN(&InclineDisplay_Home_View);
		}
		case UPDATE_INCLINE: {
			handle_get_incline();
			displayRideCurIncline(cur_incline);
	        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_INCLINE);
			return Q_HANDLED();
		}
		case UPDATE_RIDE: {
			//xil_printf("\n\In Update Ride");
			if(ride_state == RIDE_OFF){
				displayRideArrayPlotStart(display_ride_array, ride_info);
			}
			else{
				displayRideArrayPlot(display_ride_array, current_incline_index);
			}

			updateRideInfo(ride_info);
			displayRideCurIncline(cur_incline);
			return Q_HANDLED();
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}

/* Helper Functions */
/**********************************************************************/
void handle_get_incline(){
	//cur_incline = mpu_data_raw.accel_x;

	// Added by NB to track actual time between Kalman Filter operations
	// Get the current time in timer ticks
	uint32_t current_time = XTmrCtr_GetValue(&timer, 0);

	// Calculate the time difference in seconds (as a float)
	float dt = (float)(current_time - last_compute_time) / (float)XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ;

	// Update the last computation time
	last_compute_time = current_time;


	//Get and compute the incline from the MPU
	mpu_data_raw = get_mpu_data();

	if(filter_state == FILTER_ON){
		float raw_incline = computeIncline(mpu_data_raw, dt);
		// Calculate smoothed incline
		cur_incline = sensitivity_alpha * raw_incline + (1 - sensitivity_alpha) * cur_incline;
	}
	else{
		cur_incline = computeUnfilteredIncline(mpu_data_raw);
	}

	if(cur_incline > INCLINE_MAX){
		cur_incline = INCLINE_MAX;
	}
	else if(cur_incline < INCLINE_MIN){
		cur_incline = INCLINE_MIN;
	}




	//If in a ride currently add the incline to ride
	if (ride_state == RIDE_ON){
		UpdateRideArray(cur_incline);
		UpdateRideInfo(cur_incline);
	}

}


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


//float insert_current_array_interval
int cur_interval = 1;
int ride_interval = 1;
//void UpdateRideArray(float incline) {
//    temp_counter++;
//
//
//    //Update Current Incline Plot
//    if(current_incline_index == ARRAY_PLOT_LENGTH){
//    	current_incline_index = 0;
//    }
//    current_inclines[current_incline_index] = incline;
//    current_incline_index += 1;
////
////
//    if (ride_state == RIDE_ON) {
//        temp_counter = 0;
//
//
//        for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
//        	int idx = current_incline_index + i;
//        	if (idx >= ARRAY_PLOT_LENGTH){
//        		idx = idx - ARRAY_PLOT_LENGTH;
//        	}
//        	//display_ride_array[i] = current_inclines[idx];
//        	//display_ride_array[i] = current_inclines[i];
//
//		}
////
////        // Send update to UI (replace with actual function call)
//        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
//    }
//}
void UpdateRideArray(float incline) {
    temp_counter++;

    // Update Current Incline Plot

    if (ride_state == RIDE_ON) {

    	current_inclines[current_incline_index] = incline;
        display_ride_array[current_incline_index] = incline;

    	current_incline_index = (current_incline_index + 1) % ARRAY_PLOT_LENGTH;
        temp_counter = 0;

        // Send update to UI (replace with actual function call)
        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
    }
}



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
	ride_info.average_incline = ((ride_info.average_incline * (ride_info.insert_array_count - 1)) + cur_incline) / ride_info.insert_array_count;

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

/*
 * void UpdateRideArray(float incline) {
    temp_counter++;

    //bool update_UI = (temp_counter > update_time);
    bool update_UI = 0 == 0;

//    //Calculate and Store Entire Ride History
//    if (insert_array_count < insert_array_interval) {
//        insert_array_count++;
//        enqueue(incline_queue, &queue_head, &queue_count, QUEUE_SIZE, incline);
//    }
//    if (insert_array_count >= insert_array_interval) {
//
//        insert_array_count = 0;
//        float new_incline = calculate_average(incline_queue, queue_count);
//        queue_count = 0; // Reset queue
//
//        // Add to ride array
//        if (ride_array_count < RIDE_ARRAY_SIZE) {
//            ride_array[ride_array_count++] = new_incline;
//        }
//
//        // Check if ride_array needs downsampling
//        if (ride_array_count == RIDE_ARRAY_SIZE) {
//            int step = 2;
//            float new_ride_array[ARRAY_PLOT_LENGTH];
//            for (int i = 0, j = 0; j < ARRAY_PLOT_LENGTH; i += step, j++) {
//                new_ride_array[j] = calculate_average(&ride_array[i], step);
//            }
//            for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
//                //ride_array[i] = new_ride_array[i];
//            	//Save Ride for History
//            	ride_array[i] = new_ride_array[i];
//            }
//            ride_array_count = ARRAY_PLOT_LENGTH;
//            insert_array_interval *= 2;
//            ride_info.insert_array_interval = insert_array_interval;
//        }
//    }

//    if (ride_array_count < RIDE_ARRAY_SIZE) {
//		ride_array[ride_array_count++] = new_incline;
//	}
//
//	// Check if ride_array needs downsampling
//	if (ride_array_count == RIDE_ARRAY_SIZE) {
//		//Average New Values
//		for (int i = 0, j = 0; j < ARRAY_PLOT_LENGTH; i += step, j++) {
//			ride_array[j] = calculate_average(&ride_array[i], step);
//		}
//
//
//		ride_array_count = ARRAY_PLOT_LENGTH;
//	}

    //Update Current Incline Plot
    if(current_incline_index == ARRAY_PLOT_LENGTH){
    	current_incline_index = 0;
    }
    current_inclines[current_incline_index] = incline;
    current_incline_index += 1;


    if (update_UI && ride_state == RIDE_ON) {
        temp_counter = 0;


        // Update the display ride array logic
        // Use only the last ARRAY_PLOT_LENGTH elements of ride_array
        /*
		if(ride_array_count < ARRAY_PLOT_LENGTH){
			//ARRAY Lenght 180, points count 25, mult 9
			int mult = ARRAY_PLOT_LENGTH % ride_array_count + 1;
			int idx = 0;
			int mult_counter = 0;
			for (int i = ARRAY_PLOT_LENGTH; i > 0; i--) {
				mult_counter ++;
				display_ride_array[i] = ride_array[(-1) * idx];
				if (mult_counter > mult){
					idx += 1;
					mult_counter = 0;
				}
			}
		} else {

		//for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
		//	int idx = ride_array_count - ARRAY_PLOT_LENGTH + i;
		//	display_ride_array[i] = (idx >= 0) ? ride_array[idx] : 0;
		//}
        for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
        	int idx = current_incline_index + i;
        	if (idx >= ARRAY_PLOT_LENGTH){
        		idx = idx - ARRAY_PLOT_LENGTH;
        	}
        	display_ride_array[i] = current_inclines[idx];
		}

        // Send update to UI (replace with actual function call)
        QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
    }
}
*/

