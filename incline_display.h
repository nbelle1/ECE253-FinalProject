/*****************************************************************************
* incline_display.h for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#ifndef InclineDisplay_h
#define InclineDisplay_h

enum InclineDisplaySignals {
	ENCODER_UP = Q_USER_SIG,
	ENCODER_DOWN,
	ENCODER_CLICK,
	TOP_BUTTON,
	LEFT_BUTTON,
	RIGHT_BUTTON,
	BOTTOM_BUTTON,
	MIDDLE_BUTTON,
	TIMER_SLEEP
};


extern struct InclineDisplayTag AO_InclineDisplay;


void InclineDisplay_ctor(void);
void GpioHandler(void *CallbackRef);
void TwistHandler(void *CallbackRef);

#endif  
