/*****************************************************************************
* incline_display.c for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#define AO_InclineDisplay

#include "qpn_port.h"
#include "bsp.h"
#include "incline_display.h"
#include "lcd.h"
#include "mpu6050.h"


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

RideState rideState;
volatile RideInfo rideInfo;
volatile MpuData mpuData;

volatile float curIncline;
volatile float rideArray[];
volatile float displayRideArray[ARRAY_PLOT_LENGTH];



void InclineDisplay_ctor(void)  {
	InclineDisplay *me = &AO_InclineDisplay;
	QActive_ctor(&me->super, (QStateHandler)&InclineDisplay_initial);
}


QState InclineDisplay_initial(InclineDisplay *me) {
	xil_printf("\n\rInitialization");
	initializeVariables();
	rideState = RIDE_OFF;
    return Q_TRAN(&InclineDisplay_on);
}

QState InclineDisplay_on(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("\n\rPlaceHolder");
			}
		case READ_I2C: {
			getMpuData(&mpuData);
			int read_count = mpuData.i2c_read_counter += 1; // Increment counter
			if (read_count > I2C_READ_MAX){
				dispatch(GET_INCLINE);
			}
		}
		case TOGGLE_RIDE: {
			if (rideState == RIDE_ON){
				rideState = RIDE_PAUSE;
			}
			else {
				rideState = RIDE_ON;
			}
		}
		case RESET_RIDE: {
			rideState = RIDE_OFF
			// Clear all values to 0
			memset(&rideInfo, 0, sizeof(RideInfo));
			displayRideState(rideState);
		}
		case GET_INCLINE: {
			curIncline = computeIncline(mpuData);
			if (rideState == RIDE_ON){
				UpdateRideArray(curIncline);
				UpdateRideInfo(curIncline)
			}
			dispatch(UPDATE_INCLINE);
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
			xil_printf("Startup State Home\n");
			displayHomeBackground();
			displayHomeIncline(curIncline);
			return Q_HANDLED();
		}
		case TOGGLE_VIEW: {
			return Q_TRAN(&InclineDisplay_Ride_View);
		}
		case UPDATE_INCLINE: {
			displayHomeIncline(curIncline);
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}

QState InclineDisplay_Ride_View(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("Startup State Ride\n");
			displayRideBackground();
			displayRideInfo(rideInfo);
			displayRideArrayPlot(displayRideArray);
			displayRideCurIncline(curIncline)

			return Q_HANDLED();
		}
		case TOGGLE_VIEW: {
			return Q_TRAN(&InclineDisplay_Home_View);
		}
		case UPDATE_INCLINE: {
			displayRideCurIncline(curIncline);
		}
		case UPDATE_RIDE: {
			displayRideArrayPlot(displayRideArray);
			displayRideCurIncline(curIncline)
		}
	}

	return Q_SUPER(&InclineDisplay_on);
}


/* Helper Functions */
/**********************************************************************/

void initializeVariables() {

    // Initialize RideInfo (volatile struct)
    rideInfo.min_incline = 0.0f;
    rideInfo.max_incline = 0.0f;
    rideInfo.average_incline = 0.0f;
    rideInfo.insert_array_count = 0;
    rideInfo.insert_array_interval = 0;

    // Initialize MpuData (volatile struct)
    mpuData.i2c_read_counter = 0;
    mpuData.raw_accel_array = {0};
    mpuData.raw_gyro_array = {0};

    // Initialize curIncline (volatile float)
    curIncline = 0.0f;

	// Size Unknown, so skipped for now
    // Initialize rideArray (volatile float array, assuming a fixed size)
    //for (int i = 0; i < ARRAY_PLOT_LENGTH; i++) {
    //    rideArray[i] = 0.0f;
    //}

    // Initialize displayRideArray (volatile float array with known size)
    displayRideArray = {0};
	xil_printf("\nVariables Initialized");

}

int temp_counter = 0;
void UpdateRideArray(float cur_incline) {	
	xil_printf("\nTODO: UpdateRideArray");
	temp_counter += 1;
	if(temp_counter > 10){
		temp_counter = 0;
		dispatch(UPDATE_RIDE);
	}
	return;
}


void UpdateRideInfo(float cur_incline){
	xil_printf("\nTODO: UpdateRideInfo");
	return;
}

