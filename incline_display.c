/*****************************************************************************
* InclineDisplay.c for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#define AO_InclineDisplay

#include "qpn_port.h"
#include "bsp.h"
#include "incline_display.h"
#include "lcd.h"


typedef struct InclineDisplayTag  {               //InclineDisplay State machine
	QActive super;
}  InclineDisplay;

/* Setup state machines */
/**********************************************************************/
static QState InclineDisplay_initial (InclineDisplay *me);
static QState InclineDisplay_on      (InclineDisplay *me);
static QState PlaceHolder  (InclineDisplay *me);


/**********************************************************************/


InclineDisplay AO_InclineDisplay;


void InclineDisplay_ctor(void)  {
	InclineDisplay *me = &AO_InclineDisplay;
	QActive_ctor(&me->super, (QStateHandler)&InclineDisplay_initial);
}


QState InclineDisplay_initial(InclineDisplay *me) {
	xil_printf("\n\rInitialization");
    return Q_TRAN(&InclineDisplay_on);
}

QState InclineDisplay_on(InclineDisplay *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("\n\rPlaceHolder");
			}
			
		case Q_INIT_SIG: {
			return Q_TRAN(&PlaceHolder);
			}
	}
	
	return Q_SUPER(&QHsm_top);
}


/* Create InclineDisplay_on state and do any initialization code if needed */
/******************************************************************/


