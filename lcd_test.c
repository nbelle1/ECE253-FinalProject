///*
// *lcd_test.c: simple starter application for Lab 2B
// *
// */
//
//#include <stdio.h>		// Used for printf()
//#include "xparameters.h"	// Contains hardware addresses and bit masks
//#include "xil_cache.h"		// Cache Drivers
//#include "xintc.h"		// Interrupt Drivers
//#include "xtmrctr.h"		// Timer Drivers
//#include "xtmrctr_l.h" 		// Low-level timer drivers
//#include "xil_printf.h" 	// Used for xil_printf()
////#include "extra.h" 		// Provides a source of bus contention
//#include "xgpio.h" 		// LED driver, used for General purpose I/i
//#include "xspi.h"
//#include "xspi_l.h"
//#include "lcd.h"
//
//volatile int timerTrigger = 0;
//
//void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
//{
//	timerTrigger = 100;
//}
//
//
//int LCD_init()
//{
//    Xil_ICacheInvalidate();
//	Xil_ICacheEnable();
//	Xil_DCacheInvalidate();
//	Xil_DCacheEnable();
//
//	static XIntc intc;
//	static XTmrCtr axiTimer;
//	static XGpio dc;
//	static XSpi spi;
//
//	XSpi_Config *spiConfig;	/* Pointer to Configuration data */
//
//	u32 status;
//	u32 controlReg;
//
//	int sec = 0;
//	int secTmp;
//	char secStr[4];
//
//	Xil_ICacheEnable();
//	Xil_DCacheEnable();
//	print("---Entering main (CK)---\n\r");
//
//	status = XTmrCtr_Initialize(&axiTimer, XPAR_AXI_TIMER_0_DEVICE_ID);
//	if (status != XST_SUCCESS) {
//		xil_printf("Initialize timer fail!\n");
//		return XST_FAILURE;
//	}
//
//	status = XIntc_Initialize(&intc, XPAR_INTC_0_DEVICE_ID);
//	if (status != XST_SUCCESS) {
//		xil_printf("Initialize interrupt controller fail!\n");
//		return XST_FAILURE;
//	}
//
//	status = XIntc_Connect(&intc,
//				XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR,
//				(XInterruptHandler)XTmrCtr_InterruptHandler,
//				(void *)&axiTimer);
//	if (status != XST_SUCCESS) {
//		xil_printf("Connect IHR fail!\n");
//		return XST_FAILURE;
//	}
//
//	status = XIntc_Start(&intc, XIN_REAL_MODE);
//	if (status != XST_SUCCESS) {
//		xil_printf("Start interrupt controller fail!\n");
//		return XST_FAILURE;
//	}
//
//	// Enable interrupt
//	XIntc_Enable(&intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
//	microblaze_enable_interrupts();
//
//	/*
//	 * Setup the handler for the timer counter that will be called from the
//	 * interrupt context when the timer expires, specify a pointer to the
//	 * timer counter driver instance as the callback reference so the handler
//	 * is able to access the instance data
//	 */
//	XTmrCtr_SetHandler(&axiTimer, TimerCounterHandler, &axiTimer);
//
//	/*
//	 * Enable the interrupt of the timer counter so interrupts will occur
//	 * and use auto reload mode such that the timer counter will reload
//	 * itself automatically and continue repeatedly, without this option
//	 * it would expire once only
//	 */
//	XTmrCtr_SetOptions(&axiTimer, 0,
//				XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
//
//	/*
//	 * Set a reset value for the timer counter such that it will expire
//	 * eariler than letting it roll over from 0, the reset value is loaded
//	 * into the timer counter when it is started
//	 */
//	XTmrCtr_SetResetValue(&axiTimer, 0, 0xFFFF0000);
//
//	/*
//	 * Start the timer counter such that it's incrementing by default,
//	 * then wait for it to timeout a number of times
//	 */
//	XTmrCtr_Start(&axiTimer, 0);
//	xil_printf("Timer start!\n");
//
//	/*
//	 * Initialize the GPIO driver so that it's ready to use,
//	 * specify the device ID that is generated in xparameters.h
//	 */
//	status = XGpio_Initialize(&dc, XPAR_SPI_DC_DEVICE_ID);
//	if (status != XST_SUCCESS)  {
//		xil_printf("Initialize GPIO dc fail!\n");
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Set the direction for all signals to be outputs
//	 */
//	XGpio_SetDataDirection(&dc, 1, 0x0);
//
//
//
//	/*
//	 * Initialize the SPI driver so that it is  ready to use.
//	 */
//	spiConfig = XSpi_LookupConfig(XPAR_SPI_DEVICE_ID);
//	if (spiConfig == NULL) {
//		xil_printf("Can't find spi device!\n");
//		return XST_DEVICE_NOT_FOUND;
//	}
//
//	status = XSpi_CfgInitialize(&spi, spiConfig, spiConfig->BaseAddress);
//	if (status != XST_SUCCESS) {
//		xil_printf("Initialize spi fail!\n");
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Reset the SPI device to leave it in a known good state.
//	 */
//	XSpi_Reset(&spi);
//
//	/*
//	 * Setup the control register to enable master mode
//	 */
//	controlReg = XSpi_GetControlReg(&spi);
//	XSpi_SetControlReg(&spi,
//			(controlReg | XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK) &
//			(~XSP_CR_TRANS_INHIBIT_MASK));
//
//	// Select 1st slave device
//	XSpi_SetSlaveSelectReg(&spi, ~0x01);
//
//	initLCD();
//
//	clrScr();
//
////	setColor(255, 0, 0);
////	fillRect(20, 20, 220, 300);
////
////	setColor(0, 50, 100);
////	fillRect(40, 180, 200, 250);
////
////	setColor(0, 255, 0);
////	setColorBg(255, 0, 0);
////	lcdPrint("Hello !!!!!", 40, 60);
////
////	setFont(BigFont);
////	lcdPrint("<# WORLD #>", 40, 80);
////
////	setFont(SevenSegNumFont);
////	setColor(238, 64, 0);
////	setColorBg(0, 50, 100);
////	while (1) {
////		if (timerTrigger > 0) {
////			secTmp = sec++ % 1000;
////			secStr[0] = secTmp / 100 + 48;
////			secTmp -= (secStr[0] - 48) * 100;
////			secStr[1] = secTmp / 10 + 48;
////			secTmp -= (secStr[1] - 48) * 10;
////			secStr[2] = secTmp + 48;
////			secStr[3] = '\0';
////
////			lcdPrint(secStr, 70, 190);
////
////			timerTrigger = 0;
////		}
////	}
////
////	xil_printf("End\n");
//
////	lcdTriangleBackground();
//	return 0;
//}
//
//
