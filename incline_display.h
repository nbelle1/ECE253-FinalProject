/*****************************************************************************
* incline_display.h for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#ifndef InclineDisplay_h
#define InclineDisplay_h

#define I2C_READ_MAX = 10

enum InclineDisplaySignals {
	READ_I2C = Q_USER_SIG,
	TOGGLE_VIEW,
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
    float raw_accel_array[I2C_READ_MAX];
    float raw_gyro_array[I2C_READ_MAX];
} MpuData;


extern struct InclineDisplayTag AO_InclineDisplay;


void InclineDisplay_ctor(void);
void GpioHandler(void *CallbackRef);
void TwistHandler(void *CallbackRef);

#endif  
