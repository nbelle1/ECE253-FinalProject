#include "driver_mpu6050_read_test.h"

static mpu6050_handle_t gs_handle;        /**< mpu6050 handle */

/**
 * @brief     read test
 * @param[in] addr is the iic device address
 * @param[in] times is the test times
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t mpu6050_display_test(mpu6050_address_t addr, uint32_t times)
{
    uint8_t res;
    uint32_t i;
    mpu6050_info_t info;
    
    int16_t accel_raw[1][3];
    float accel_g[1][3];
    int16_t gyro_raw[1][3];
    float gyro_dps[1][3];
    uint16_t len = 1;
    
    /* link interface function */
    DRIVER_MPU6050_LINK_INIT(&gs_handle, mpu6050_handle_t);
    DRIVER_MPU6050_LINK_IIC_INIT(&gs_handle, mpu6050_interface_iic_init);
    DRIVER_MPU6050_LINK_IIC_DEINIT(&gs_handle, mpu6050_interface_iic_deinit);
    DRIVER_MPU6050_LINK_IIC_READ(&gs_handle, mpu6050_interface_iic_read);
    DRIVER_MPU6050_LINK_IIC_WRITE(&gs_handle, mpu6050_interface_iic_write);
    DRIVER_MPU6050_LINK_DELAY_MS(&gs_handle, mpu6050_interface_delay_ms);
    DRIVER_MPU6050_LINK_DEBUG_PRINT(&gs_handle, mpu6050_interface_debug_print);
    DRIVER_MPU6050_LINK_RECEIVE_CALLBACK(&gs_handle, mpu6050_interface_receive_callback);
    
    /* get information */
    res = mpu6050_info(&info);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: get info failed.\n");
       
        return 1;
    }
    else
    {
        /* print chip info */
        mpu6050_interface_debug_print("mpu6050: chip is %s.\n", info.chip_name);
        mpu6050_interface_debug_print("mpu6050: manufacturer is %s.\n", info.manufacturer_name);
        mpu6050_interface_debug_print("mpu6050: interface is %s.\n", info.interface);
        mpu6050_interface_debug_print("mpu6050: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        mpu6050_interface_debug_print("mpu6050: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        mpu6050_interface_debug_print("mpu6050: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        mpu6050_interface_debug_print("mpu6050: max current is %0.2fmA.\n", info.max_current_ma);
        mpu6050_interface_debug_print("mpu6050: max temperature is %0.1fC.\n", info.temperature_max);
        mpu6050_interface_debug_print("mpu6050: min temperature is %0.1fC.\n", info.temperature_min);
    }
    
    /* start read test */
    mpu6050_interface_debug_print("mpu6050: start display test.\n");
    
    /* set the addr pin */
    res = mpu6050_set_addr_pin(&gs_handle, addr);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set addr pin failed.\n");
       
        return 1;
    }
    
    /* init */
    res = mpu6050_init(&gs_handle);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: init failed.\n");
       
        return 1;
    }
    
    /* delay 100 ms */
    mpu6050_interface_delay_ms(100);
    
    /* disable sleep */
    res = mpu6050_set_sleep(&gs_handle, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set sleep failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* set pll x */
    res = mpu6050_set_clock_source(&gs_handle, MPU6050_CLOCK_SOURCE_PLL_X_GYRO);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set clock source failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* set 50Hz */
    res = mpu6050_set_sample_rate_divider(&gs_handle, (1000 / 50) - 1);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set sample rate divider failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* set low pass filter 3 */
    res = mpu6050_set_low_pass_filter(&gs_handle, MPU6050_LOW_PASS_FILTER_3);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set low pass filter failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable temperature sensor */
    res = mpu6050_set_temperature_sensor(&gs_handle, MPU6050_BOOL_TRUE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set temperature sensor failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* disable cycle wake up */
    res = mpu6050_set_cycle_wake_up(&gs_handle, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set cycle wake up failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* set wake up frequency 1.25Hz */
    res = mpu6050_set_wake_up_frequency(&gs_handle, MPU6050_WAKE_UP_FREQUENCY_1P25_HZ);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set wake up frequency failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable acc x */
    res = mpu6050_set_standby_mode(&gs_handle, MPU6050_SOURCE_ACC_X, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set standby mode failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable acc y */
    res = mpu6050_set_standby_mode(&gs_handle, MPU6050_SOURCE_ACC_Y, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set standby mode failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable acc z */
    res = mpu6050_set_standby_mode(&gs_handle, MPU6050_SOURCE_ACC_Z, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set standby mode failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable gyro x */
    res = mpu6050_set_standby_mode(&gs_handle, MPU6050_SOURCE_GYRO_X, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set standby mode failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable gyro y */
    res = mpu6050_set_standby_mode(&gs_handle, MPU6050_SOURCE_GYRO_Y, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set standby mode failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    /* enable gyro z */
    res = mpu6050_set_standby_mode(&gs_handle, MPU6050_SOURCE_GYRO_Z, MPU6050_BOOL_FALSE);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set standby mode failed.\n");
        (void)mpu6050_deinit(&gs_handle);
       
        return 1;
    }
    
    

    
    
    
        
    /* set gyroscope range */
    res = mpu6050_set_gyroscope_range(&gs_handle, MPU6050_GYROSCOPE_RANGE_500DPS);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set gyroscope range failed.\n");
        (void)mpu6050_deinit(&gs_handle);
        return 1;
    }

    /* set accelerometer range */
    res = mpu6050_set_accelerometer_range(&gs_handle, MPU6050_ACCELEROMETER_RANGE_4G);
    if (res != 0)
    {
        mpu6050_interface_debug_print("mpu6050: set accelerometer range failed.\n");
        (void)mpu6050_deinit(&gs_handle);
        return 1;
    }

    /* display data every second */
    mpu6050_interface_debug_print("mpu6050: start displaying data.\n");
    while (1) /* Infinite loop for continuous display */
    {
        res = mpu6050_read(&gs_handle, accel_raw, accel_g, gyro_raw, gyro_dps, &len);
        if (res != 0)
        {
            mpu6050_interface_debug_print("mpu6050: read failed.\n");
            (void)mpu6050_deinit(&gs_handle);
            return 1;
        }

        /* display accelerometer data */
        mpu6050_interface_debug_print("mpu6050: Acceleration (g) - X: %0.2f, Y: %0.2f, Z: %0.2f\n",
                                       accel_g[0][0], accel_g[0][1], accel_g[0][2]);

        /* display gyroscope data */
        mpu6050_interface_debug_print("mpu6050: Gyroscope (dps) - X: %0.2f, Y: %0.2f, Z: %0.2f\n",
                                       gyro_dps[0][0], gyro_dps[0][1], gyro_dps[0][2]);

        /* delay 1 second */
        mpu6050_interface_delay_ms(1000);
    }
    

    
    /* finish read test */
    mpu6050_interface_debug_print("mpu6050: finish read test.\n");
    (void)mpu6050_deinit(&gs_handle);
    
    return 0;
}
