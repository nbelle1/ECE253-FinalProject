/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_mpu6050_interface_template.c
 * @brief     driver mpu6050 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2022-06-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2022/06/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_mpu6050_interface.h"
#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
#include <unistd.h> // For usleep()

#define IIC_DEVICE_ID	   XPAR_IIC_0_DEVICE_ID

static XIic IicInstance;

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t mpu6050_interface_iic_init(void)
{
    int Status;
    XIic_Config *ConfigPtr;

    // Look up the configuration for the IIC device
    ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr == NULL)
    {
        xil_printf("Error: IIC configuration not found.\n");
        return 1;  // Configuration not found
    }

    // Initialize the IIC instance with the configuration
    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr, ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS)
    {
        xil_printf("Error: IIC initialization failed.\n");
        return 2;  // Initialization failed
    }

    // Start the IIC device
    Status = XIic_Start(&IicInstance);
    if (Status != XST_SUCCESS)
    {
        xil_printf("Error: IIC start failed.\n");
        return 3;  // Failed to start IIC
    }

    // Verify that the IIC bus is ready
    if (XIic_IsIicBusy(&IicInstance))
    {
        xil_printf("Error: IIC bus is busy.\n");
        return 4;  // IIC bus is busy
    }

    // Enable repeated start option
    //XIic_SetOptions(&IicInstance, XII_REPEATED_START_OPTION);

    // Set the 7-bit MPU6050 address (0x68 if AD0=0, or 0x69 i f AD0=1)
    XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE, 0x68);

    //xil_printf("IIC interface initialized successfully.\n");
    return 0;  // Success
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t mpu6050_interface_iic_deinit(void)
{
    int status;

    // Stop the IIC driver
    status = XIic_Stop(&IicInstance);
    if (status != XST_SUCCESS)
    {
        xil_printf("IIC deinit failed with status: %d\n", status);
        return 1; // Deinit failed
    }

    return 0; // Success
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr is the iic device write address
 * @param[in]  reg is the iic register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t mpu6050_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    int status;

    uint8_t iic_addr_7bit = addr >> 1;


    //xil_printf("IIC Read: Addr=0x%X, Reg=0x%X, Len=%d\n", iic_addr_7bit, reg, len);

    // Send register address
    //status = XIic_MasterSend(&IicInstance, &tx_buf, 1);
    //XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE, iic_addr_7bit);
    status = XIic_Send(IicInstance.BaseAddress, iic_addr_7bit, &reg, 1, XIIC_REPEATED_START);
//    if (status == 0)
//    {
//        xil_printf("IIC Write (Register Address) failed: %d\n", status);
//        return 1;
//    }

    // Receive data
    //status = XIic_MasterRecv(&IicInstance, buf, len);
    status = XIic_Recv(IicInstance.BaseAddress, iic_addr_7bit, buf, len, XIIC_STOP);
//    if (status == 0)
//    {
//        xil_printf("IIC Read (Data) failed: %d\n", status);
//        return 1;
//    }

    // Print out received data
//	xil_printf("Received Data: ");
//	for (uint16_t i = 0; i < len; i++)
//	{
//	   xil_printf("0x%X ", buf[i]);
//	}
//	xil_printf("\n");

    return 0; // Success
}

/**
 * @brief     interface iic bus write
 * @param[in] addr is the iic device write address
 * @param[in] reg is the iic register address
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t mpu6050_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    int status;
    uint8_t iic_addr_7bit = addr >> 1;
    uint8_t data[len + 1];

    // Prepare the data buffer (register address + data)
    data[0] = reg; // First byte is the register address
//    for (uint16_t i = 0; i < len; i++) {
//        data[i + 1] = buf[i]; // Remaining bytes are the data to write
//    }
    memcpy(&data[1], buf, len);

    // Perform the I2C write operation
    //status = XIic_MasterSend(&IicInstance, data, len + 1);
    status = XIic_Send(IicInstance.BaseAddress, iic_addr_7bit, data, len + 1, XIIC_STOP);
//    if (status == 0) {
//        xil_printf("IIC write failed with status: %d\n", status);
//        return 1; // Write failed
//    }

    return 0; // Success
}

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void mpu6050_interface_delay_ms(uint32_t ms)
{
    usleep(ms * 20); // Convert milliseconds to microseconds
}

/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @note      none
 */
#define DEBUG_BUFFER_SIZE 256  // Define a buffer size for formatted messages
#include "xil_printf.h"
#include <stdarg.h>
#include <stdio.h>
void mpu6050_interface_debug_print(const char *const fmt, ...)
{
	char buffer[DEBUG_BUFFER_SIZE];
	va_list args;

	// Initialize the variable argument list
	va_start(args, fmt);

	// Format the string into the buffer
	vsnprintf(buffer, DEBUG_BUFFER_SIZE, fmt, args);

	// Clean up the variable argument list
	va_end(args);

	// Print the formatted string using xil_printf
	xil_printf("%s", buffer);
}

/**
 * @brief     interface receive callback
 * @param[in] type is the irq type
 * @note      none
 */
void mpu6050_interface_receive_callback(uint8_t type)
{
    switch (type)
    {
        case MPU6050_INTERRUPT_MOTION :
        {
            mpu6050_interface_debug_print("mpu6050: irq motion.\n");
            
            break;
        }
        case MPU6050_INTERRUPT_FIFO_OVERFLOW :
        {
            mpu6050_interface_debug_print("mpu6050: irq fifo overflow.\n");
            
            break;
        }
        case MPU6050_INTERRUPT_I2C_MAST :
        {
            mpu6050_interface_debug_print("mpu6050: irq i2c master.\n");
            
            break;
        }
        case MPU6050_INTERRUPT_DMP :
        {
            mpu6050_interface_debug_print("mpu6050: irq dmp\n");
            
            break;
        }
        case MPU6050_INTERRUPT_DATA_READY :
        {
            mpu6050_interface_debug_print("mpu6050: irq data ready\n");
            
            break;
        }
        default :
        {
            mpu6050_interface_debug_print("mpu6050: irq unknown code.\n");
            
            break;
        }
    }
}

/**
 * @brief     interface dmp tap callback
 * @param[in] count is the tap count
 * @param[in] direction is the tap direction
 * @note      none
 */
void mpu6050_interface_dmp_tap_callback(uint8_t count, uint8_t direction)
{
    switch (direction)
    {
        case MPU6050_DMP_TAP_X_UP :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq x up with %d.\n", count);
            
            break;
        }
        case MPU6050_DMP_TAP_X_DOWN :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq x down with %d.\n", count);
            
            break;
        }
        case MPU6050_DMP_TAP_Y_UP :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq y up with %d.\n", count);
            
            break;
        }
        case MPU6050_DMP_TAP_Y_DOWN :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq y down with %d.\n", count);
            
            break;
        }
        case MPU6050_DMP_TAP_Z_UP :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq z up with %d.\n", count);
            
            break;
        }
        case MPU6050_DMP_TAP_Z_DOWN :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq z down with %d.\n", count);
            
            break;
        }
        default :
        {
            mpu6050_interface_debug_print("mpu6050: tap irq unknown code.\n");
            
            break;
        }
    }
}

/**
 * @brief     interface dmp orient callback
 * @param[in] orientation is the dmp orientation
 * @note      none
 */
void mpu6050_interface_dmp_orient_callback(uint8_t orientation)
{
    switch (orientation)
    {
        case MPU6050_DMP_ORIENT_PORTRAIT :
        {
            mpu6050_interface_debug_print("mpu6050: orient irq portrait.\n");
            
            break;
        }
        case MPU6050_DMP_ORIENT_LANDSCAPE :
        {
            mpu6050_interface_debug_print("mpu6050: orient irq landscape.\n");
            
            break;
        }
        case MPU6050_DMP_ORIENT_REVERSE_PORTRAIT :
        {
            mpu6050_interface_debug_print("mpu6050: orient irq reverse portrait.\n");
            
            break;
        }
        case MPU6050_DMP_ORIENT_REVERSE_LANDSCAPE :
        {
            mpu6050_interface_debug_print("mpu6050: orient irq reverse landscape.\n");
            
            break;
        }
        default :
        {
            mpu6050_interface_debug_print("mpu6050: orient irq unknown code.\n");
            
            break;
        }
    }
}
