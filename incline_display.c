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

volatile float cur_incline;
int raw_incline_index;
float raw_incline_array[NUM_INCLINE_AVG];

float incline_queue[];
float ride_array[];

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
	cur_incline_index = 0;

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
			float raw_incline = computeIncline(mpu_data_raw, 0.1);

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
			displayRideInfo(ride_info);
			displayRideCurIncline(ride_state);
			return Q_HANDLED();
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}


/* Helper Functions */
/**********************************************************************/



int temp_counter = 0;
void UpdateRideArray(float incline) {
	xil_printf("\nTODO: UpdateRideArray");
	temp_counter += 1;
	if(temp_counter > 10){
		temp_counter = 0;
		QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_RIDE);
	}
	return;
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
	//ride_info.average_incline = ((ride_info.average_incline * (ride_info.insert_array_count - 1)) + cur_incline) / ride_info.insert_array_count;


	xil_printf("\nTODO: UpdateRideInfo");
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

