#ifndef MPU6050_H
#define MPU6050_H

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
#include "mpu_6050/driver_mpu6050_read_test.h"
#include "mpu_6050/driver_mpu6050.h"
#include "mpu_6050/driver_mpu6050_basic.h"
#include "incline_display.h"

/************************** Constant Definitions ******************************/

#define IIC_DEVICE_ID XPAR_IIC_0_DEVICE_ID

/************************** Function Prototypes *******************************/

int getMpuData_Test(u16 DeviceId);
void getMpuData(MpuData *mpu_data);
float getMpuAccel();
float getMpuGyro();
float computeIncline(MpuData mpu_data);


/************************** Variable Definitions ******************************/

extern XIic Iic;

#endif /* MPU6050_H */