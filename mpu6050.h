#ifndef MPU6050_H
#define MPU6050_H

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
#include <math.h>
#include "mpu_6050/driver_mpu6050_read_test.h"
#include "mpu_6050/driver_mpu6050.h"
#include "mpu_6050/driver_mpu6050_basic.h"

/************************** Constant Definitions ******************************/

#define IIC_DEVICE_ID XPAR_IIC_0_DEVICE_ID
#define RAD_TO_DEG 57.295779513082320876798154814105


/************************** Variable Definitions ******************************/
//Structure To Hold Raw Data from mpu-6050
typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
} mpu_data_t;

typedef struct
{
	double angle;
	double bias;
	double rate;
	double Q_angle;
	double Q_bias;
	double R_measure;
	double P[2][2];
} Kalman_t;

extern XIic Iic;

/************************** Function Prototypes *******************************/

int mpu_init();
int mpu_deinit();
mpu_data_t get_mpu_data();
int mpu_data_read_test();
float computeIncline(mpu_data_t mpu_data, float dt);
float computeUnfilteredIncline(mpu_data_t data);
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, float dt);


#endif /* MPU6050_H */
