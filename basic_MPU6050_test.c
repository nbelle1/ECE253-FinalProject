/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define IIC_BASE_ADDRESS	XPAR_AXI_IIC_0_BASEADDR

// Define the MPU6050's I2C address
#define MPU6050_I2C_ADDR 0x68 // Adjust if AD0 pin is high (0x69)

// Define WHO_AM_I register
#define MPU6050_WHO_AM_I 0x75

XIic IicInstance;


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int init_iic();
int read_who_am_i();
int read_accelerometer();

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the IIC level 0
* driver to read the temperature.
*
*
* @return	Always 0
*
* @note
*
* The main function is returning an integer to prevent compiler warnings.
*
****************************************************************************/
//int main() {
//    int Status;
//
//    // Initialize IIC
//    Status = init_iic();
//    if (Status != XST_SUCCESS) {
//        return XST_FAILURE;
//    }
//
//    // Read WHO_AM_I register
//    Status = read_who_am_i();
//    if (Status != XST_SUCCESS) {
//        xil_printf("Failed to communicate with MPU6050\n");
//    } else {
//        xil_printf("MPU6050 communication successful\n");
//    }
//
//    Status = read_accelerometer();
//	if (Status != XST_SUCCESS) {
//			xil_printf("Couldn't read accelerometer data.\n");
//		} else {
//			xil_printf("Successfully read accelerometer data.\n");
//		}
//
//    // Deinitialize IIC
//    XIic_Stop(&IicInstance);
//
//    return 0;
//}

/*****************************************************************************/
/**
*
* The function reads the temperature of the IIC temperature sensor on the
* IIC bus using the low-level driver.
*
* @param	IicBaseAddress is the base address of the device.
* @param	TempSensorAddress is the address of the Temperature Sensor device
*		on the IIC bus.
* @param	TemperaturePtr is the databyte read from the temperature sensor.
*
* @return	The number of bytes read from the temperature sensor, normally one
*		byte if successful.
*
* @note		None.
*
****************************************************************************/
int init_iic() {
    XIic_Config *Config;
    int Status;

    // Lookup configuration
    Config = XIic_LookupConfig(XPAR_IIC_0_DEVICE_ID);
    if (Config == NULL) {
        xil_printf("Failed to lookup IIC config\n");
        return XST_FAILURE;
    }

    // Initialize IIC instance
    Status = XIic_CfgInitialize(&IicInstance, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("Failed to initialize IIC\n");
        return XST_FAILURE;
    }

    // Start the IIC core
    Status = XIic_Start(&IicInstance);
    if (Status != XST_SUCCESS) {
        xil_printf("Failed to start IIC\n");
        return XST_FAILURE;
    }

    xil_printf("IIC initialized successfully\n");
    return XST_SUCCESS;
}

int read_who_am_i() {
    uint8_t reg = MPU6050_WHO_AM_I; // Register to read
    uint8_t data;
    int Status;

    // Send register address with repeated start
    Status = XIic_Send(IicInstance.BaseAddress, MPU6050_I2C_ADDR, &reg, 1, XIIC_REPEATED_START);
    if (Status != 1) {
        xil_printf("Failed to send register address: %d\n", Status);
        return XST_FAILURE;
    }

    // Read the register value
    Status = XIic_Recv(IicInstance.BaseAddress, MPU6050_I2C_ADDR, &data, 1, XIIC_STOP);
    if (Status != 1) {
        xil_printf("Failed to read register value: %d\n", Status);
        return XST_FAILURE;
    }

    xil_printf("WHO_AM_I register value: 0x%X\n", data);
    return XST_SUCCESS;
}

int read_accelerometer() {
    uint8_t buffer[2];
    uint8_t data[6];  // Buffer for X, Y, Z axes
    int status;

    // Step 1: Wake up the MPU6050 by writing to the PWR_MGMT_1 register
    buffer[0] = 0x6B;  // PWR_MGMT_1 register address
    buffer[1] = 0x00;  // Value to wake up the MPU6050
    status = XIic_Send(IicInstance.BaseAddress, MPU6050_I2C_ADDR, buffer, 2, XIIC_STOP);
    if (status != 2) {  // Check if 2 bytes were sent
        xil_printf("Failed to write wake-up value to MPU6050: %d\n", status);
        return XST_FAILURE;
    }
    xil_printf("MPU6050 woke up successfully.\n");

    // Step 2: Send the register address for accelerometer data
    uint8_t reg = 0x3B;  // ACCEL_XOUT_H
    status = XIic_Send(IicInstance.BaseAddress, MPU6050_I2C_ADDR, &reg, 1, XIIC_REPEATED_START);
    if (status != 1) {
        xil_printf("Failed to send register address for accelerometer: %d\n", status);
        return XST_FAILURE;
    }

    // Step 3: Read accelerometer data
    status = XIic_Recv(IicInstance.BaseAddress, MPU6050_I2C_ADDR, data, 6, XIIC_STOP);
    if (status != 6) {
        xil_printf("Failed to read accelerometer data: %d\n", status);
        return XST_FAILURE;
    }

    // Step 4: Process and print accelerometer data
    int16_t accel_x = (data[0] << 8) | data[1];
    int16_t accel_y = (data[2] << 8) | data[3];
    int16_t accel_z = (data[4] << 8) | data[5];

    xil_printf("Accel X: %d, Y: %d, Z: %d\n", accel_x, accel_y, accel_z);
    return XST_SUCCESS;
}

