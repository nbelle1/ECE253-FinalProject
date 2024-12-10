

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
#include "mpu_6050/driver_mpu6050_read_test.h"
#include "mpu_6050/driver_mpu6050.h"
#include "mpu_6050/driver_mpu6050_basic.h"
#include "incline_display.h"



/************************** Constant Definitions ******************************/


#define IIC_DEVICE_ID	   XPAR_IIC_0_DEVICE_ID


/**************************** Type Definitions ********************************/


/***************** Macros (Inline Functions) Definitions **********************/


/************************** Function Prototypes *******************************/

int gather_MPU_data(u16 DeviceId);

/************************** Variable Definitions ******************************/


XIic Iic; /* The driver instance for IIC Device */



// int main(void)
// {
// 	int Status;

// 	/*
// 	 * Run the example, specify the device ID that is generated in
// 	 * xparameters.h.
// 	 */

// 	Status = gather_MPU_data(IIC_DEVICE_ID);

// 	if (Status != XST_SUCCESS) {
// 		xil_printf("Gather Data Failed\r\n");
// 		return XST_FAILURE;
// 	}

// 	xil_printf("Successfully ran MPU6050 data gathering\r\n");
// 	return XST_SUCCESS;

// }


/*****************************************************************************/
/**
*
* This function does a selftest on the IIC device and XIic driver as an
* example.
*
* @param	DeviceId is the XPAR_<IIC_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
****************************************************************************/

int getMpuData_Test(u16 DeviceId)

{

	float accel_g[3]; /**< Acceleration data in g */
	float gyro_dps[3]; /**< Gyroscope data in dps */
	uint8_t res;

	/* initialize the device */
	res = mpu6050_basic_init(MPU6050_ADDRESS_AD0_LOW);
	if (res != 0)
	{
		mpu6050_interface_debug_print("mpu6050: init failed.\n");
		return 1;
	}

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

		/* delay 10 milliseconds */
		mpu6050_interface_delay_ms(1);
	}

	/* deinitialize the device */
	res = mpu6050_basic_deinit();
	if (res != 0)
	{
		mpu6050_interface_debug_print("mpu6050: deinit failed.\n");
		return 1;
	}

	mpu6050_interface_debug_print("mpu6050: test completed successfully.\n");

	return XST_SUCCESS;
}

void getMpuData(MpuData *mpu_data){
	index = mpu_data->i2c_read_counter;
	if (index >= 0 && index < I2C_READ_MAX) {
        float accel = getMpuAccel(); // Replace with your actual function
        float gyro = getMpuGyro();  // Replace with your actual function

        mpu_data->raw_accel_array[index] = accel;
        mpu_data->raw_gyro_array[index] = gyro;
		
    } else {
        xil_printf("Setting Raw Mpu Data Index out of bounds.\n");
    }
}

float getMpuAccel(){
	xil_printf("\nTODO: getMpuAccel");
	return 0.0;

}
float getMpuGyro(){
	xil_printf("\nTODO: getMpuGyro");
	return 0.0;
}
float computeIncline(MpuData mpu_data){
	xil_printf("\nTODO: computeIncline");
	return 0.0;
}