/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xiic_selftest_example.c
*
* This file contains a example for using the IIC hardware device and
* XIic driver.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   05/09/05 Initial release for TestApp integration.
* 2.00a sdm  09/22/09 Updated to use the HAL APIs, replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize. Minor changes made as per
*		      coding guidelines.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 3.10  gm   07/09/23 Added SDT support.
* </pre>
*
*******************************************************************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
#include <math.h>
#include "mpu_6050/driver_mpu6050_read_test.h"
#include "mpu_6050/driver_mpu6050.h"
#include "mpu_6050/driver_mpu6050_basic.h"
#include "mpu6050.h"


/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define IIC_DEVICE_ID	   XPAR_IIC_0_DEVICE_ID


/**************************** Type Definitions ********************************/


/***************** Macros (Inline Functions) Definitions **********************/


/************************** Function Prototypes *******************************/



int mpu_data_read_test();
int mpu_init();
int mpu_deinit();
mpu_data_t get_mpu_data();


/************************** Variable Definitions ******************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger.
 */
XIic Iic; /* The driver instance for IIC Device */


Kalman_t KalmanX = {.Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f};
Kalman_t KalmanY = {.Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f};




/******************************************************************************/
/**
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/

//int main(void)
//{
//	int Status;
//
//	/*
//	 * Run the example, specify the device ID that is generated in
//	 * xparameters.h.
//	 */
//
//	// Initialize MPU 6050
//	Status = mpu_init();
//
//	if (Status != XST_SUCCESS) {
//		xil_printf("MPU Initialization Failed\r\n");
//		return XST_FAILURE;
//	}
//
//
//
//	// Get MPU data
//	mpu_data_t data = get_mpu_data();
//
//	// Display the data
//	printf("Acceleration (g): X: %0.2f, Y: %0.2f, Z: %0.2f\n",
//		   data.accel_x, data.accel_y, data.accel_z);
//	printf("Gyroscope (dps): X: %0.2f, Y: %0.2f, Z: %0.2f\n",
//		   data.gyro_x, data.gyro_y, data.gyro_z);
//
//
//
////	// DATA GATHERING TEST
////	Status = mpu_data_read_test();
////	if (Status != XST_SUCCESS) {
////		xil_printf("Gather Data Failed\r\n");
////		return XST_FAILURE;
////	}
////	xil_printf("Successfully ran MPU6050 data gathering\r\n");
//
//
//	/* deinitialize the device */
//	Status = mpu_deinit();
//	if (Status != XST_SUCCESS)
//	{
//		xil_printf("MPU De-initialization Failed\r\n");
//		return XST_FAILURE;
//	}
//
//
//
//
//	return XST_SUCCESS;
//
//}

// Call MPU 6050 driver initialization
int mpu_init(){
	/* initialize the device */
	uint8_t res;
	res = mpu6050_basic_init(MPU6050_ADDRESS_AD0_LOW);
	if (res != 0)
	{
		mpu6050_interface_debug_print("mpu6050: init failed.\n");
		return 1;
	}
	return 0;
}

// Call MPU 6050 driver de-initialization
int mpu_deinit(){
	/* deinitialize the device */
	uint8_t res;
	res = mpu6050_basic_deinit();
	if (res != 0)
	{
		mpu6050_interface_debug_print("mpu6050: deinit failed.\n");
		return 1;
	}
	return 0;
}



// Function to get MPU6050 gyroscope and accelerometer data
mpu_data_t get_mpu_data() {
    mpu_data_t data;
    float accel_g[3]; // Acceleration data in g
    float gyro_dps[3]; // Gyroscope data in dps
    uint8_t res;

    // Read data from MPU6050
    res = mpu6050_basic_read(accel_g, gyro_dps);
    if (res != 0) {
        mpu6050_interface_debug_print("mpu6050: read failed.\n");
        // Return zeroed struct in case of error
        return (mpu_data_t){0};
    }

    // Populate the struct with the readings
    data.accel_x = accel_g[0];
    data.accel_y = accel_g[1];
    data.accel_z = accel_g[2];
    data.gyro_x = gyro_dps[0];
    data.gyro_y = gyro_dps[1];
    data.gyro_z = gyro_dps[2];

    return data;
}




// TEST FUNCTIONS


//This function performs a read data test on the MPU 6050, gathering 60 samples of
//gyro and accel reads at a rate of 1 sample per second
int mpu_data_read_test()

{

	float accel_g[3]; /**< Acceleration data in g */
	float gyro_dps[3]; /**< Gyroscope data in dps */
	uint8_t res;

	mpu6050_interface_debug_print("mpu6050: start reading data.\n");

	for (int i = 0; i < 60; i++) /* Read data for 1 minute */
	{
		/* read data */
		res = mpu6050_basic_read(accel_g, gyro_dps);
		if (res != 0)
		{
			mpu6050_interface_debug_print("mpu6050: read failed.\n");
			(void)mpu6050_basic_deinit();
			return 1;
		}

		/* display the data */
		mpu6050_interface_debug_print("Acceleration (g): X: %0.2f, Y: %0.2f, Z: %0.2f\n",
									   accel_g[0], accel_g[1], accel_g[2]);
		mpu6050_interface_debug_print("Gyroscope (dps): X: %0.2f, Y: %0.2f, Z: %0.2f\n",
									   gyro_dps[0], gyro_dps[1], gyro_dps[2]);

		/* delay 1 milliseconds */
		mpu6050_interface_delay_ms(1000);
	}


	mpu6050_interface_debug_print("mpu6050: test completed successfully.\n");

	return XST_SUCCESS;
}

//float computeIncline(mpu_data_t mpu_data, float dt){
//	xil_printf("Compute Incline TODO\r\n");
//	mpu6050_interface_debug_print("\n  Accel_X: %.2f, Accel_Y: %.2f, Accel_Z: %.2f\r\n", mpu_data.accel_x, mpu_data.accel_y, mpu_data.accel_z);
//	return mpu_data.accel_x;
//}


float computeIncline(mpu_data_t data, float dt)
{
	xil_printf("\n[DEBUG] Raw MPU Data:\r\n");
	mpu6050_interface_debug_print("Accel_X: %.3f, Accel_Y: %.3f, Accel_Z: %.3f\r\n", data.accel_x, data.accel_y, data.accel_z);
	mpu6050_interface_debug_print("Gyro_X: %.3f, Gyro_Y: %.3f\r\n", data.gyro_x, data.gyro_y);

	double roll = atan2(data.accel_y, sqrt(data.accel_x * data.accel_x + data.accel_z * data.accel_z)) * RAD_TO_DEG;
	mpu6050_interface_debug_print("\n[DEBUG] Computed Roll: %.3f degrees\r\n", roll);

	double pitch = atan2(-data.accel_x, data.accel_z) * RAD_TO_DEG;
	mpu6050_interface_debug_print("\n[DEBUG] Computed Pitch: %.3f degrees\r\n", pitch);

	double filteredRoll = Kalman_getAngle(&KalmanX, roll, data.gyro_y, dt);
	mpu6050_interface_debug_print("\n[DEBUG] Filtered Roll (Kalman): %.3f degrees\r\n", filteredRoll);

	// might need to add in logic for MPU flipping past 90 degrees
	double filteredPitch = Kalman_getAngle(&KalmanY, pitch, data.gyro_x, dt);
	mpu6050_interface_debug_print("\n[DEBUG] Filtered Pitch (Kalman): %.3f degrees\r\n", filteredPitch);

	return (float)filteredPitch;
	//float newVal = 50.0;
	//return newVal;

	//return data.accel_x;
}

double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, float dt)
{
	//mpu6050_interface_debug_print("\n[DEBUG] Kalman Filter Input:\r\n");
	//mpu6050_interface_debug_print("\n  New Angle: %.3f, New Rate: %.3f, dt: %.3f\r\n", newAngle, newRate, dt);
	double rate = newRate - Kalman->bias;
	Kalman->angle += dt * rate;

	Kalman->P[0][0] += dt * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0] + Kalman->Q_angle);
	Kalman->P[0][1] -= dt * Kalman->P[1][1];
	Kalman->P[1][0] -= dt * Kalman->P[1][1];
	Kalman->P[1][1] += Kalman->Q_bias * dt;

	double S = Kalman->P[0][0] + Kalman->R_measure;
	double K[2];
	K[0] = Kalman->P[0][0] / S;
	K[1] = Kalman->P[1][0] / S;

	double y = newAngle - Kalman->angle;
	Kalman->angle += K[0] * y;
	Kalman->bias += K[1] * y;

	double P00_temp = Kalman->P[0][0];
	double P01_temp = Kalman->P[0][1];

	Kalman->P[0][0] -= K[0] * P00_temp;
	Kalman->P[0][1] -= K[0] * P01_temp;
	Kalman->P[1][0] -= K[1] * P00_temp;
	Kalman->P[1][1] -= K[1] * P01_temp;

	return Kalman->angle;
}




