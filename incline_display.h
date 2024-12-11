/*****************************************************************************
* incline_display.h for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#ifndef InclineDisplay_h
#define InclineDisplay_h

#include "qepn.h"
#include "mpu6050.h"

#define NUM_INCLINE_AVG 1

enum InclineDisplaySignals {
	TOGGLE_VIEW = Q_USER_SIG,
	TOGGLE_RIDE,
	RESET_RIDE,
	GET_INCLINE,
	UPDATE_INCLINE,
	UPDATE_RIDE,
	
};

enum RideState {
	RIDE_OFF = 0,
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


typedef struct {
    int i2c_read_counter;
    mpu_data_t data_array[NUM_INCLINE_AVG];
} MpuRawDataStruct;


extern struct InclineDisplayTag AO_InclineDisplay;


void InclineDisplay_ctor(void);
void GpioHandler(void *CallbackRef);
void TwistHandler(void *CallbackRef);

#endif  
