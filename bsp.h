/*****************************************************************************
* bsp.h for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/
#ifndef bsp_h
#define bsp_h

#define RESET_VALUE 100000


/* bsp functions ..........................................................*/

void BSP_init(void);
void ISR_gpio(void);
void ISR_timer(void);

// Timer functions
void timer_handler();

// Button functions
void button_handler();

// Encoder functions
void encoder_handler();
int current_pin();

void printDebugLog(void);

#define BSP_showState(prio_, state_) ((void)0)


#endif                                                             /* bsp_h */


