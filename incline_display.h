/*****************************************************************************
* incline_display.h for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#ifndef InclineDisplay_h
#define InclineDisplay_h

#include "qepn.h"
#include "mpu6050.h"


enum InclineDisplaySignals {
	TOGGLE_VIEW = Q_USER_SIG,
	TOGGLE_RIDE,
	RESET_RIDE,
	//GET_INCLINE,
	UPDATE_INCLINE,
	UPDATE_RIDE,
	ENCODER_DOWN,
	ENCODER_UP,
	
};

enum RideState {
	RIDE_OFF,
	RIDE_ON,
	RIDE_PAUSE,
};

typedef struct {
    float min_incline;
    float max_incline;
    float average_incline;
	int insert_array_count;
	int insert_array_interval;
} RideInfo;


extern struct InclineDisplayTag AO_InclineDisplay;


void InclineDisplay_ctor(void);
void GpioHandler(void *CallbackRef);
void TwistHandler(void *CallbackRef);

#endif  
