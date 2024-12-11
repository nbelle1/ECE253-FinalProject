/*
 * lcd.c
 *
 *  Created on: Oct 21, 2015
 *      Author: atlantis
 */

/*
 UTFT.cpp - Multi-Platform library support for Color TFT LCD Boards
 Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved

 This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
 and RGB_GLCD libraries for Arduino and chipKit. As the number of
 supported display modules and controllers started to increase I felt
 it was time to make a single, universal library as it will be much
 easier to maintain in the future.

 Basic functionality of this library was origianlly based on the
 demo-code provided by ITead studio (for the ITDB02 modules) and
 NKC Electronics (for the RGB GLCD module/shield).

 This library supports a number of 8bit, 16bit and serial graphic
 displays, and will work with both Arduino, chipKit boards and select
 TI LaunchPads. For a full list of tested display modules and controllers,
 see the document UTFT_Supported_display_modules_&_controllers.pdf.

 When using 8bit and 16bit display modules there are some
 requirements you must adhere to. These requirements can be found
 in the document UTFT_Requirements.pdf.
 There are no special requirements when using serial displays.

 You can find the latest version of the library at
 http://www.RinkyDinkElectronics.com/

 This library is free software; you can redistribute it and/or
 modify it under the terms of the CC BY-NC-SA 3.0 license.
 Please see the included documents for further information.

 Commercial use of this library requires you to buy a license that
 will allow commercial use. This includes using the library,
 modified or not, as a tool to sell products.

 The license applies to all part of the library including the
 examples and tools supplied with the library.
 */

#include "lcd.h"

// Global variables
int fch;
int fcl;
int bch;
int bcl;
volatile RGB currentBackgroundColor = {255, 255, 255}; // Default white


struct _current_font cfont;

// Write command to LCD controller
void LCD_Write_COM(char VL) {
	Xil_Out32(SPI_DC, 0x0);
	Xil_Out32(SPI_DTR, VL);

	while (0 == (Xil_In32(SPI_IISR) & XSP_INTR_TX_EMPTY_MASK))
		;
	Xil_Out32(SPI_IISR, Xil_In32(SPI_IISR) | XSP_INTR_TX_EMPTY_MASK);
}

// Write 8-bit data to LCD controller
void LCD_Write_DATA(char VL) {
	Xil_Out32(SPI_DC, 0x01);
	Xil_Out32(SPI_DTR, VL);

	while (0 == (Xil_In32(SPI_IISR) & XSP_INTR_TX_EMPTY_MASK))
		;
	Xil_Out32(SPI_IISR, Xil_In32(SPI_IISR) | XSP_INTR_TX_EMPTY_MASK);
}

// Initialize LCD controller
void initLCD(void) {
	int i;

	xil_printf("Entered LCD INIT\n");

	// Reset
	LCD_Write_COM(0x01);
	for (i = 0; i < 500000; i++)
		; //Must wait > 5ms

	LCD_Write_COM(0xCB);
	LCD_Write_DATA(0x39);
	LCD_Write_DATA(0x2C);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x34);
	LCD_Write_DATA(0x02);

	LCD_Write_COM(0xCF);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0XC1);
	LCD_Write_DATA(0X30);

	LCD_Write_COM(0xE8);
	LCD_Write_DATA(0x85);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x78);

	LCD_Write_COM(0xEA);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x00);

	LCD_Write_COM(0xED);
	LCD_Write_DATA(0x64);
	LCD_Write_DATA(0x03);
	LCD_Write_DATA(0X12);
	LCD_Write_DATA(0X81);

	LCD_Write_COM(0xF7);
	LCD_Write_DATA(0x20);

	LCD_Write_COM(0xC0);   //Power control
	LCD_Write_DATA(0x23);  //VRH[5:0]

	LCD_Write_COM(0xC1);   //Power control
	LCD_Write_DATA(0x10);  //SAP[2:0];BT[3:0]

	LCD_Write_COM(0xC5);   //VCM control
	LCD_Write_DATA(0x3e);  //Contrast
	LCD_Write_DATA(0x28);

	LCD_Write_COM(0xC7);   //VCM control2
	LCD_Write_DATA(0x86);  //--

	LCD_Write_COM(0x36);   // Memory Access Control
	LCD_Write_DATA(0x48);

	LCD_Write_COM(0x3A);
	LCD_Write_DATA(0x55);

	LCD_Write_COM(0xB1);
	LCD_Write_DATA(0x00);
	LCD_Write_DATA(0x18);

	LCD_Write_COM(0xB6);   // Display Function Control
	LCD_Write_DATA(0x08);
	LCD_Write_DATA(0x82);
	LCD_Write_DATA(0x27);

	LCD_Write_COM(0x11);   //Exit Sleep
	for (i = 0; i < 100000; i++)
		;

	LCD_Write_COM(0x29);   //Display on
	LCD_Write_COM(0x2c);

	//for (i = 0; i < 100000; i++);

	// Default color and fonts
	fch = 0xFF;
	fcl = 0xFF;
	bch = 0x00;
	bcl = 0x00;
	setFont(SmallFont);
	xil_printf("Finished LCD INIT\n");
}

// Set boundary for drawing
void setXY(int x1, int y1, int x2, int y2) {
	LCD_Write_COM(0x2A);
	LCD_Write_DATA(x1 >> 8);
	LCD_Write_DATA(x1);
	LCD_Write_DATA(x2 >> 8);
	LCD_Write_DATA(x2);
	LCD_Write_COM(0x2B);
	LCD_Write_DATA(y1 >> 8);
	LCD_Write_DATA(y1);
	LCD_Write_DATA(y2 >> 8);
	LCD_Write_DATA(y2);
	LCD_Write_COM(0x2C);
}

// Remove boundry
void clrXY(void) {
	setXY(0, 0, DISP_X_SIZE, DISP_Y_SIZE);
}

// Set foreground RGB color for next drawing
void setColor(u8 r, u8 g, u8 b) {
	// 5-bit r, 6-bit g, 5-bit b
	fch = (r & 0x0F8) | g >> 5;
	fcl = (g & 0x1C) << 3 | b >> 3;
}

// Set background RGB color for next drawing
void setColorBg(u8 r, u8 g, u8 b) {
	// 5-bit r, 6-bit g, 5-bit b
	bch = (r & 0x0F8) | g >> 5;
	bcl = (g & 0x1C) << 3 | b >> 3;
}

// Clear display
void clrScr(void) {
	// Black screen
	setColor(0, 0, 0);

	fillRect(0, 0, DISP_X_SIZE, DISP_Y_SIZE);
}

// Draw horizontal line
void drawHLine(int x, int y, int l) {
	int i;

	if (l < 0) {
		l = -l;
		x -= l;
	}

	setXY(x, y, x + l, y);
	for (i = 0; i < l + 1; i++) {
		LCD_Write_DATA(fch);
		LCD_Write_DATA(fcl);
	}

	clrXY();
}

// Fill a rectangular 
void fillRect(int x1, int y1, int x2, int y2) {
	int i;

	if (x1 > x2)
		swap(int, x1, x2);

	if (y1 > y2)
		swap(int, y1, y2);

	setXY(x1, y1, x2, y2);
	for (i = 0; i < (x2 - x1 + 1) * (y2 - y1 + 1); i++) {
		LCD_Write_DATA(fch);
		LCD_Write_DATA(fcl);
	}

	clrXY();
}


// Select the font used by print() and printChar()
void setFont(u8* font) {
	cfont.font = font;
	cfont.x_size = font[0];
	cfont.y_size = font[1];
	cfont.offset = font[2];
	cfont.numchars = font[3];
}

// Print a character
void printChar(u8 c, int x, int y) {
	u8 ch;
	int i, j, pixelIndex;

	setXY(x, y, x + cfont.x_size - 1, y + cfont.y_size - 1);

	pixelIndex = (c - cfont.offset) * (cfont.x_size >> 3) * cfont.y_size + 4;
	for (j = 0; j < (cfont.x_size >> 3) * cfont.y_size; j++) {
		ch = cfont.font[pixelIndex];
		for (i = 0; i < 8; i++) {
			if ((ch & (1 << (7 - i))) != 0) {
				LCD_Write_DATA(fch);
				LCD_Write_DATA(fcl);
			} else {
				LCD_Write_DATA(bch);
				LCD_Write_DATA(bcl);
			}
		}
		pixelIndex++;
	}

	clrXY();
}

// Print string
void lcdPrint(char *st, int x, int y) {
	int i = 0;
	while (*st != '\0')
		printChar(*st++, x + cfont.x_size * i++, y);
}


// Funcitons for Incline Display 

//Home View
void displayHomeBackground(){
	//Set background color
	// Store current background color
	currentBackgroundColor.r = 69;
	currentBackgroundColor.g = 208;
	currentBackgroundColor.b = 223;

	setColor(69, 208, 223);
	setColorBg(69, 208, 223);
	fillRect(0,0,DISP_X_SIZE,DISP_Y_SIZE);

	//Print Font
	setColor(0,0,0);
	setFont(SmallFont);
	lcdPrint("Home View",150,10);

	return;
}
void displayHomeIncline(float incline){
	char st[16]; // Buffer to hold the converted string
	//sprintf(st, "%.2f", incline); // Convert float to string with 2 decimal places

	float roundedIncline = ((int)(incline * 10 + (incline >= 0 ? 0.5 : -0.5))) / 10.0;

	// Format the string to align the tenths place
	if (roundedIncline >= 0) {
		sprintf(st, "%6.1f", roundedIncline); // Align with 6 characters, space for positive numbers
	} else {
		sprintf(st, "%6.1f", roundedIncline); // Align negative numbers similarly
	}

	//Print Font
	setColor(0,0,0);
	setFont(BigFont);
	lcdPrint(st,60,50);

	//Print Current Incline
	setColor(0,0,0);
	//fillRect(50,200,DISP_X_SIZE-50,200 + (incline * 10));

	//Print Chart With Current Incline As Well
	return;
}


//Ride View
void displayRideBackground(){
	//Set whole background

	currentBackgroundColor.r = 208;
	currentBackgroundColor.g = 69;
	currentBackgroundColor.b = 223;

	setColor(208, 69, 223);
	setColorBg(208, 69, 233);
	fillRect(0,0,DISP_X_SIZE,DISP_Y_SIZE);


	//Print Font
	setColor(0,0,0);
	setFont(SmallFont);
	setColorBg(currentBackgroundColor.r, currentBackgroundColor.g, currentBackgroundColor.b);
	lcdPrint("Ride View",150,10);

	return;
}
void displayRideInfo(RideInfo ride_info) {
    setFont(SmallFont);
    setColorBg(currentBackgroundColor.r, currentBackgroundColor.g, currentBackgroundColor.b);
    char buffer[32]; // Temporary buffer for converting values to strings
    int text_start_x = 20;
    int text_start_y = 90;
    int line_spacing = 20;

    // Print minimum incline
    sprintf(buffer, "Min Incline:");
    lcdPrint(buffer, text_start_x, text_start_y); // Label
    sprintf(buffer, "%6.1f", ((int)(ride_info.min_incline * 10 + (ride_info.min_incline >= 0 ? 0.5 : -0.5))) / 10.0);
    lcdPrint(buffer, 140, text_start_y); // Value aligned

    // Print maximum incline
    sprintf(buffer, "Max Incline:");
    lcdPrint(buffer, text_start_x, text_start_y + line_spacing); // Label
    sprintf(buffer, "%6.1f", ((int)(ride_info.max_incline * 10 + (ride_info.max_incline >= 0 ? 0.5 : -0.5))) / 10.0);
    lcdPrint(buffer, 140, text_start_y + line_spacing); // Value aligned

    // Print average incline
    sprintf(buffer, "Avg Incline:");
    lcdPrint(buffer, text_start_x, text_start_y + 2 * line_spacing); // Label
    sprintf(buffer, "%6.1f", ((int)(ride_info.average_incline * 10 + (ride_info.average_incline >= 0 ? 0.5 : -0.5))) / 10.0);
    lcdPrint(buffer, 140, text_start_y + 2 * line_spacing); // Value aligned

    // Print insert array count
    sprintf(buffer, "Array Count:");
    lcdPrint(buffer, text_start_x, text_start_y + 3 * line_spacing); // Label
    sprintf(buffer, "%6d", ride_info.insert_array_count); // Fixed-width integer formatting
    lcdPrint(buffer, 140, text_start_y + 3 * line_spacing); // Value aligned

    // Print insert array interval
    sprintf(buffer, "Array Interval:");
    lcdPrint(buffer, text_start_x, text_start_y + 4 * line_spacing); // Label
    sprintf(buffer, "%6d", ride_info.insert_array_interval); // Fixed-width integer formatting
    lcdPrint(buffer, 140, text_start_y + 4 * line_spacing); // Value aligned

    return;
}
void updateRideInfo(RideInfo ride_info) {
    setFont(SmallFont);
    char buffer[16]; // Temporary buffer for converting numbers to strings

    // Update minimum incline value
    sprintf(buffer, "%6.1f", ((int)(ride_info.min_incline * 10 + (ride_info.min_incline >= 0 ? 0.5 : -0.5))) / 10.0);
    lcdPrint(buffer, 140, 90); // Value aligned

    // Update maximum incline value
    sprintf(buffer, "%6.1f", ((int)(ride_info.max_incline * 10 + (ride_info.max_incline >= 0 ? 0.5 : -0.5))) / 10.0);
    lcdPrint(buffer, 140, 110); // Value aligned

    // Update average incline value
    sprintf(buffer, "%6.1f", ((int)(ride_info.average_incline * 10 + (ride_info.average_incline >= 0 ? 0.5 : -0.5))) / 10.0);
    lcdPrint(buffer, 140, 130); // Value aligned

    // Update array count value
    sprintf(buffer, "%6d", ride_info.insert_array_count); // Fixed-width integer formatting
    lcdPrint(buffer, 140, 150); // Value aligned

    // Update array interval value
    sprintf(buffer, "%6d", ride_info.insert_array_interval); // Fixed-width integer formatting
    lcdPrint(buffer, 140, 170); // Value aligned

    return;
}

//void displayRideInfo(RideInfo ride_info) {
//    setFont(SmallFont);
//    setColorBg(currentBackgroundColor.r, currentBackgroundColor.g, currentBackgroundColor.b);
//    char buffer[32]; // Temporary buffer for converting values to strings
//    int text_start_x = 20;
//    int text_start_y = 90;
//    int line_spacing = 20;
//
//    // Print minimum incline
//    sprintf(buffer, "Min Incline:");
//    lcdPrint(buffer, text_start_x, text_start_y); // Display at position (10, 90)
//    sprintf(buffer, "%s%.3f", (ride_info.min_incline >= 0 ? " " : ""), ride_info.min_incline);
//    lcdPrint(buffer, 140, 90); // X position matches the value column from displayRideInfo
//    //xil_printf("\n%s", buffer);
//
//    // Print maximum incline
//    sprintf(buffer, "Max Incline:");
//    lcdPrint(buffer, text_start_x, text_start_y + line_spacing); // Display below the previous line
//    sprintf(buffer, "%s%.3f", (ride_info.max_incline >= 0 ? " " : ""), ride_info.max_incline);
//    lcdPrint(buffer, 140, 110); // X position matches the value column from displayRideInfo
//    //xil_printf("\n%s", buffer);
//
//    // Print average incline
//    sprintf(buffer, "Avg Incline:");
//    lcdPrint(buffer, text_start_x, text_start_y + 2 * line_spacing); // Display below the previous line
//    sprintf(buffer, "%s%.3f", (ride_info.average_incline >= 0 ? " " : ""), ride_info.average_incline);
//    lcdPrint(buffer, 140, 130); // X position matches the value column from displayRideInfo
//    //xil_printf("\n%s", buffer);
//
//    // Print insert array count
//    sprintf(buffer, "Array Count:");
//    lcdPrint(buffer, text_start_x, text_start_y + 3 * line_spacing); // Display below the previous line
//    sprintf(buffer, " %d", ride_info.insert_array_count);
//    lcdPrint(buffer, 140, 150); // X position matches the value column from displayRideInfo
//    //xil_printf("\n%s", buffer);
//
//    // Print insert array interval
//    sprintf(buffer, "Array Interval:");
//    lcdPrint(buffer, text_start_x, text_start_y + 4 * line_spacing); // Display below the previous line
//    sprintf(buffer, " %d", ride_info.insert_array_interval);
//    lcdPrint(buffer, 140, 170); // X position matches the value column from displayRideInfo
//    //xil_printf("\n%s", buffer);
//
//    return;
//}
//
//void updateRideInfo(RideInfo ride_info) {
//    setFont(SmallFont);
//    char buffer[16]; // Temporary buffer for converting numbers to strings
//
//    // Update minimum incline value
//    sprintf(buffer, "%s%.3f", (ride_info.min_incline >= 0 ? " " : ""), ride_info.min_incline);
//    lcdPrint(buffer, 140, 90); // X position matches the value column from displayRideInfo
//
//    // Update maximum incline value
//    sprintf(buffer, "%s%.3f", (ride_info.max_incline >= 0 ? " " : ""), ride_info.max_incline);
//    lcdPrint(buffer, 140, 110); // X position matches the value column from displayRideInfo
//
//    // Update average incline value
//    sprintf(buffer, "%s%.3f", (ride_info.average_incline >= 0 ? " " : ""), ride_info.average_incline);
//    lcdPrint(buffer, 140, 130); // X position matches the value column from displayRideInfo
//
//    // Update array count value
//    sprintf(buffer, " %d", ride_info.insert_array_count);
//    lcdPrint(buffer, 140, 150); // X position matches the value column from displayRideInfo
//
//    // Update array interval value
//    sprintf(buffer, " %d", ride_info.insert_array_interval);
//    lcdPrint(buffer, 140, 170); // X position matches the value column from displayRideInfo
//
//    return;
//}




void displayRideArrayPlot(float ride_array[ARRAY_PLOT_LENGTH], RideInfo ride_info){

	//Make a box that represetnts the ride_info.averge_incline in bounds 20,200,DISP_X_SIZE-20,DISP_Y_SIZE-20
	int offset = 20;
	int x_min = offset;
	int x_max = offset + (ARRAY_PLOT_LENGTH * ARRAY_PLOT_POINT_WIDTH);
	int y_min = 200;
	int y_max = DISP_Y_SIZE - offset;
	int y_mid = (y_max + y_min) / 2;
	float mult = 0.5;


	for(int i = 0; i < ARRAY_PLOT_LENGTH; i++){
		float y_val = ride_array[i] * mult;
		if(y_val > 0){
			setColor(50,0,0);
		}
		else {
			setColor(0,50,0);
		}
		int x1 = x_min + (i * ARRAY_PLOT_POINT_WIDTH);
		int x2 = x1 + ARRAY_PLOT_POINT_WIDTH;
		int y2 = int(y_val) + y_mind

		fillRect(x1, x2, y1, y2);
	}

	//fillRect(20,200,DISP_X_SIZE-20,DISP_Y_SIZE-20);
	return;
}

void displayRideCurIncline(float incline){

	// Convert float to string
	char st[16]; // Buffer to hold the converted string
	float roundedIncline = ((int)(incline * 10 + (incline >= 0 ? 0.5 : -0.5))) / 10.0;

	// Format the string to align the tenths place
	if (roundedIncline >= 0) {
		sprintf(st, "%6.1f", roundedIncline); // Align with 6 characters, space for positive numbers
	} else {
		sprintf(st, "%6.1f", roundedIncline); // Align negative numbers similarly
	}


	//Print Font
	setColor(0,0,0);
	setFont(BigFont);
	lcdPrint(st,60,50);

	return;
}

//Shared
void displayRideState(char *st){
	//Print Background
	setColor(currentBackgroundColor.r, currentBackgroundColor.g, currentBackgroundColor.b);
	fillRect(10,10,58,22);

	//Print Font
	setColor(0,0,0);
	setFont(SmallFont);
	lcdPrint(st,10,10);
	return;
}


