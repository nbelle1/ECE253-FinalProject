/*****************************************************************************
* lab2a.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#define AO_LAB2A

#include "qpn_port.h"
#include "bsp.h"
#include "lab2a.h"
#include "lcd.h"


volatile int volume;
volatile int mode = -1;
volatile int muted = 0;

typedef struct Lab2ATag  {               //Lab2A State machine
	QActive super;
}  Lab2A;

/* Setup state machines */
/**********************************************************************/
static QState Lab2A_initial (Lab2A *me);
static QState Lab2A_on      (Lab2A *me);
static QState Lab2A_Active  (Lab2A *me);
static QState Lab2A_Mute  (Lab2A *me);
static QState Lab2A_Sleep (Lab2A * me);


/**********************************************************************/


Lab2A AO_Lab2A;


void Lab2A_ctor(void)  {
	Lab2A *me = &AO_Lab2A;
	QActive_ctor(&me->super, (QStateHandler)&Lab2A_initial);
}


QState Lab2A_initial(Lab2A *me) {
	xil_printf("\n\rInitialization");
    return Q_TRAN(&Lab2A_on);
}

QState Lab2A_on(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			lcdTriangleBackground();
			volume = 0;
			xil_printf("\n\rOn");
			}
			
		case Q_INIT_SIG: {
			return Q_TRAN(&Lab2A_Active);
			}
	}
	
	return Q_SUPER(&QHsm_top);
}


/* Create Lab2A_on state and do any initialization code if needed */
/******************************************************************/

QState Lab2A_Active(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			muted = 0;
			xil_printf("Startup State A\n");
			displayVolume(volume);
			printMode(mode);
			return Q_HANDLED();
		}
		case ENCODER_UP: {
			if(volume < 63){
				increaseVolume(volume);
				volume++;
			}
			xil_printf("New Volume: %d\n", volume);
			xil_printf("Encoder Up from State A\n");
			return Q_HANDLED();
		}
		case ENCODER_DOWN: {
			if(volume > 0){
				//volume--;
				decreaseVolume(volume);
				volume--;
			}
			xil_printf("New Volume: %d\n", volume);
			//printVolumeNumber(volume);
			xil_printf("Encoder Down from State A\n");
			return Q_HANDLED();
		}
		case ENCODER_CLICK:  {
			xil_printf("Changing State\n");
			return Q_TRAN(&Lab2A_Mute);
		}
		case TOP_BUTTON:
		case LEFT_BUTTON:
		case RIGHT_BUTTON:
		case BOTTOM_BUTTON:
		case MIDDLE_BUTTON: {
			mode = Q_SIG(me) - 7;
			printMode(mode);
			//xil_printf("Middle Button Press from State A: Mode 5\n");
			return Q_HANDLED();
		}
		case TIMER_SLEEP: {
			return Q_TRAN(&Lab2A_Sleep);
		}
	}

	return Q_SUPER(&Lab2A_on);

}

QState Lab2A_Mute(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			muted = 1;
			xil_printf("Startup State B\n");
			printMode(mode);
			eraseVolume(volume);
			return Q_HANDLED();
		}
		
		case ENCODER_UP:
		case ENCODER_DOWN:
		case ENCODER_CLICK:  {
			xil_printf("Changing State\n");
			return Q_TRAN(&Lab2A_Active);
		}
		case TOP_BUTTON:
		case LEFT_BUTTON:
		case RIGHT_BUTTON:
		case BOTTOM_BUTTON:
		case MIDDLE_BUTTON: {
			mode = Q_SIG(me) - 7;
			printMode(mode);
			//xil_printf("Middle Button Press from State A: Mode 5\n");
			return Q_HANDLED();
		}
		case TIMER_SLEEP: {
			return Q_TRAN(&Lab2A_Sleep);
		}

	}

	return Q_SUPER(&Lab2A_on);

}

QState Lab2A_Sleep(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("Startup State - Sleep\n");
			eraseVolume(volume);
			eraseMode();
			return Q_HANDLED();
		}
		case ENCODER_UP:
		case ENCODER_DOWN:
		case ENCODER_CLICK:
		case TOP_BUTTON:
		case LEFT_BUTTON:
		case RIGHT_BUTTON:
		case BOTTOM_BUTTON:
		case MIDDLE_BUTTON: {
			xil_printf("Changing State - Sleep to Active\n");
			if(muted){
				return Q_TRAN(&Lab2A_Mute);
			}
			else{
				return Q_TRAN(&Lab2A_Active);
			}
		}
	}

	return Q_SUPER(&Lab2A_on);

}

