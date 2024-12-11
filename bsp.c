/*****************************************************************************
* bsp.c for InclineDisplay of ECE 153a at UCSB
* Date of the Last Update:  October 27,2019
*****************************************************************************/

/**/
#include "qpn_port.h"
#include "bsp.h"
#include "incline_display.h"
#include "xintc.h"
#include "xtmrctr.h"
#include "xil_exception.h"
#include <unistd.h>


#include "xspi.h"
#include "xspi_l.h"
#include "lcd.h"

/*****************************/

/* Define all variables and Gpio objects here  */

#define GPIO_CHANNEL1 1

void debounceInterrupt(); // Write This function

// Create ONE interrupt controllers XIntc
XIntc sys_intc;

// Create two static XGpio variables
XGpio EncoderGpio;
XGpio BtnGpio;

//Setup LCD SPI Bus
XGpio dc;
XSpi spi;
XSpi_Config *spiConfig;	/* Pointer to Configuration data */
u32 controlReg;

// Timer Interrupt (Added by NB and DB)
XTmrCtr sys_tmrctr;

//Debouncing timer  (Added by NB)
XTmrCtr timer;

volatile int sleep_count = 0;

// Suggest Creating two int's to use for determining the direction of twist

/*..........................................................................*/
void BSP_init(void) {
/* Setup LED's, etc */
/* Setup interrupts and reference to interrupt handler function(s)  */

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 *
	 * Initialize GPIO and connect the interrupt controller to the GPIO.
	 *
	 */

	// Press Knob

	// Twist Knob

	// Interrupt Controller Initialization
	XStatus Status;
	Status = XST_SUCCESS;
	Status = XIntc_Initialize(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		if (Status == XST_DEVICE_NOT_FOUND) {
			xil_printf("XST_DEVICE_NOT_FOUND...\r\n");
		} else {
			xil_printf("a different error from XST_DEVICE_NOT_FOUND...\r\n");
		}
		xil_printf("Interrupt controller driver failed to be initialized...\r\n");
		//return XST_FAILURE;
	}
	xil_printf("Interrupt controller driver initialized!\r\n");


	// Timer Interrupt Initialization
	Status = XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR,
			(XInterruptHandler) timer_handler, &sys_tmrctr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to connect the application handlers to the interrupt controller...\r\n");
		return;
	}
	xil_printf("Connected to Timer Interrupt Controller!\r\n");



	// Encoder Interrupt Initialization
	Status = XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_ENCODER_IP2INTC_IRPT_INTR,
			(XInterruptHandler) encoder_handler, &EncoderGpio);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to connect the application handlers to the interrupt controller...\r\n");
		return;
	}
	xil_printf("Connected to Encoder Interrupt Controller!\r\n");


	// Button Interrupt Initialization
	Status = XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR,
			(XInterruptHandler) button_handler, &BtnGpio);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to connect the application handlers to the interrupt controller...\r\n");
		return;
	}
	xil_printf("Connected to Button Interrupt Controller!\r\n");

	// Initialize Encoder GPIO
	XGpio_Initialize(&EncoderGpio, XPAR_ENCODER_DEVICE_ID);
	XGpio_InterruptEnable(&EncoderGpio, 1);
	XGpio_InterruptGlobalEnable(&EncoderGpio);

	// Initialize Button GPIO
	XGpio_Initialize(&BtnGpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	XGpio_InterruptEnable(&BtnGpio, 1);
	XGpio_InterruptGlobalEnable(&BtnGpio);

	// Initialization Timer
	Status = XTmrCtr_Initialize(&sys_tmrctr, XPAR_AXI_TIMER_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Timer initialization failed...\r\n");
		return;
	}
	//xil_printf("Initialized Timer!\r\n");
	XTmrCtr_SetOptions(&sys_tmrctr, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(&sys_tmrctr, 0, 0xFFFFFFFF - RESET_VALUE);// 1000 clk cycles @ 100MHz = 10us
	XTmrCtr_Start(&sys_tmrctr, 0);

	// Debouncing Timer Initialization
	XTmrCtr_Initialize(&timer, XPAR_AXI_TIMER_1_DEVICE_ID);
	uint32_t Control = XTmrCtr_GetOptions(&timer, 0) | XTC_CAPTURE_MODE_OPTION | XTC_INT_MODE_OPTION;
	//XTmrCtr_SetResetValue(&timer, 0, 0xFFFFFFFF - 1000);
	XTmrCtr_SetOptions(&timer, 0, Control);
	XTmrCtr_Start(&timer, 0);

	//Initialize SPI DC GPIO
	Status = XGpio_Initialize(&dc, XPAR_SPI_DC_DEVICE_ID);
	if (Status != XST_SUCCESS)  {
		xil_printf("Initialize GPIO dc fail!\n");
		return;
	}
	XGpio_SetDataDirection(&dc, 1, 0x0);

	//Initialize SPI Driver
	spiConfig = XSpi_LookupConfig(XPAR_SPI_DEVICE_ID);
	if (spiConfig == NULL) {
		xil_printf("Can't find spi device!\n");
		return;
	}
	Status = XSpi_CfgInitialize(&spi, spiConfig, spiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialize spi fail!\n");
		return;
	}

	//Setup SPI Control Register in Master Mode and Select 1st Slave Device
	XSpi_Reset(&spi);
	controlReg = XSpi_GetControlReg(&spi);
	XSpi_SetControlReg(&spi,
			(controlReg | XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK) &
			(~XSP_CR_TRANS_INHIBIT_MASK));
	XSpi_SetSlaveSelectReg(&spi, ~0x01);
	initLCD();
	clrScr();

	//TODO: Initialize I2C And Accelerometer


	// Register interrupts
	microblaze_register_handler(
			(XInterruptHandler) XIntc_DeviceInterruptHandler,
			(void*) XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);

	// Enable all interrupts on Microblaze
	//microblaze_enable_interrupts();
	//xil_printf("Interrupts enabled!\r\n");

	return;
		
}
/*..........................................................................*/
void QF_onStartup(void) {                 /* entered with interrupts locked */

/* Enable interrupts */
	xil_printf("\n\rQF_onStartup\n"); // Comment out once you are in your complete program


	 //Start interrupt controller
	XStatus Status = XIntc_Start(&sys_intc, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf("Interrupt controller driver failed to start...\r\n");
		return;
	}
	xil_printf("Started Interrupt Controller!\r\n");



	// Enable timer, button, and encoder interrupts
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_ENCODER_IP2INTC_IRPT_INTR);
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR);

}


void QF_onIdle(void) {        /* entered with interrupts locked */

    QF_INT_UNLOCK();                       /* unlock interrupts */

    {

    }
}

/* Q_onAssert is called only when the program encounters an error*/
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                   /* name of the file that throws the error */
    (void)line;                                   /* line of the code that throws the error */
    QF_INT_LOCK();
    printDebugLog();
    for (;;) {
    }
}

/* Interrupt handler functions here.  Do not forget to include them in incline_display.h!
To post an event from an ISR, use this template:
QActive_postISR((QActive *)&AO_InclineDisplay, SIGNALHERE);
Where the Signals are defined in incline_display.h  */

/******************************************************************************
*
* This is the interrupt handler routine for the GPIO for this example.
*
******************************************************************************/

// Timer Interrupt Handler and Related Functions
void timer_handler() {
	// This is the interrupt handler function
	// Do not print inside of this function.
	Xuint32 ControlStatusReg;
	/*
	 * Read the new Control/Status Register content.
	 */
	ControlStatusReg = XTimerCtr_ReadReg(sys_tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET);


	QActive_postISR((QActive *)&AO_InclineDisplay, UPDATE_INCLINE);

	/*
	 * Acknowledge the interrupt by clearing the interrupt
	 * bit in the timer control status register
	 */
	XTmrCtr_WriteReg(sys_tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET,
			ControlStatusReg |XTC_CSR_INT_OCCURED_MASK);

}


// Encoder Interrupt Handler and Related Functions
unsigned current_encoder_state;
enum EVENT {
	P1LOW_P2LOW = 0,
	P1HIGH_P2LOW = 1,
	P1LOW_P2HIGH = 2,
	P1HIGH_P2HIGH = 3,
	BUTTON_DOWN = 7

};
enum STATE {
	S0_START,
	S1_CCW_1,
	S2_CW_1,
	S3_CCW_2,
	S4_CW_2,
	S5_CCW_3,
	S6_CW_3,
};
void encoder_handler(void *CallbackRef){
	//xil_printf("Encoder Trigger\n");
	XGpio *GpioPtr = (XGpio *)CallbackRef;
	int cur_pin = XGpio_DiscreteRead(&EncoderGpio, 1);
	sleep_count = 0;

	if(cur_pin == BUTTON_DOWN){
		//Send Encoder Down Signal
		//xil_printf("Encoder Click Signal\n");
    	QActive_postISR((QActive *)&AO_InclineDisplay, TOGGLE_VIEW);

		//Time Based De-bouncing
		usleep(200000);
		//reset_pin();
		//continue;
	}
	else{
		switch(current_encoder_state){
			case S0_START:
				switch (cur_pin) {
					case P1HIGH_P2LOW:
						current_encoder_state = S2_CW_1;
						break;
					case P1LOW_P2HIGH:
						current_encoder_state = S1_CCW_1;
						break;
				}
				break;
			case S1_CCW_1:
				switch (cur_pin) {
					case P1HIGH_P2HIGH:
						current_encoder_state = S0_START;
						break;
					case P1LOW_P2LOW:
						current_encoder_state = S3_CCW_2;
						break;
				}
				break;
			case S2_CW_1:
				switch (cur_pin) {
					case P1HIGH_P2HIGH:
						current_encoder_state = S0_START;
						break;
					case P1LOW_P2LOW:
						current_encoder_state = S4_CW_2;
						break;
				}
				break;
			case S3_CCW_2:
				switch (cur_pin) {
					case P1LOW_P2HIGH:
						current_encoder_state = S1_CCW_1;
						break;
					case P1HIGH_P2LOW:
						current_encoder_state = S5_CCW_3;
						break;
				}
				break;
			case S4_CW_2:
				switch (cur_pin) {
					case P1HIGH_P2LOW:
						current_encoder_state = S2_CW_1;
						break;
					case P1LOW_P2HIGH:
						current_encoder_state = S6_CW_3;
						break;
				}
				break;
			case S5_CCW_3:
				switch (cur_pin) {
					case P1LOW_P2LOW:
						current_encoder_state = S3_CCW_2;
						break;
					case P1HIGH_P2HIGH:
						//Update LED CCW
						//xil_printf("Encoder Down Signal\n");
						QActive_postISR((QActive *)&AO_InclineDisplay, ENCODER_DOWN);

						//current_position = led_left(current_position);
						current_encoder_state = S0_START;
						break;
				}
				break;
			case S6_CW_3:
				switch (cur_pin) {
					case P1LOW_P2LOW:
						current_encoder_state = S4_CW_2;
						break;
					case P1HIGH_P2HIGH:
						//Update LED CW
						//xil_printf("Encoder Right Signal\n");
						QActive_postISR((QActive *)&AO_InclineDisplay, ENCODER_UP);

						current_encoder_state = S0_START;
						//current_position = led_right(current_position);
						break;
				}
				break;
		}
	}


	XGpio_InterruptClear(GpioPtr, 1);
}



uint32_t last_interrupt_time = 0; // Store last debounce time in timer ticks

// Button Interrupt Handler and Related Functions
void button_handler(void *CallbackRef){
	XGpio *GpioPtr = (XGpio *)CallbackRef;

	// Read the current timer value
	uint32_t current_time = XTmrCtr_GetValue(&timer, 0);

	// Check if the debounce period has elapsed (200 ms in ticks)
	if ((current_time - last_interrupt_time) < (XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ / 2)) { // 200 ms debounce
		XGpio_InterruptClear(GpioPtr, 1);
		return;
	}

	// Update the last interrupt time
	last_interrupt_time = current_time;

	unsigned int btn = XGpio_DiscreteRead(&BtnGpio, 1);
	//xil_printf("Button interrupt\n");
	sleep_count = 0;

	//Handle Top Button
	if(btn == 1){
		QActive_postISR((QActive *)&AO_InclineDisplay, TOGGLE_VIEW);
	}

	// Handle Left Button
	else if(btn == 2){
		QActive_postISR((QActive *)&AO_InclineDisplay, TOGGLE_RIDE);
	}

	//Handle Right Button
	else if(btn == 4){
		QActive_postISR((QActive *)&AO_InclineDisplay, RESET_RIDE);
	}

//	// Handle Count Down
//	else if(btn == 8){
//		QActive_postISR((QActive *)&AO_InclineDisplay, RESET_INCLINE);
//	}

	// Handle Reset
//	else if(btn == 16){
//		QActive_postISR((QActive *)&AO_InclineDisplay, MIDDLE_BUTTON);
//	}

	//usleep(200000);
	XGpio_InterruptClear(GpioPtr, 1);
}
