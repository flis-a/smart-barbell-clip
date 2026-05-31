#ifndef DRIVERS_IMU_H
#define DRIVERS_IMU_H

#include <stdint.h>

#include "app/smartclip_err.h"
#include "bsp/bsp_i2c.h"

typedef struct imu imu_t;

typedef struct {
    bsp_i2c_t * bus;
    uint8_t     addr_7bit;    /* BNO055: 0x28 default, 0x29 if ADR pin high */
} imu_cfg_t;

imu_t *         imu_create   (const imu_cfg_t * cfg);
smartclip_err_t imu_init     (imu_t * dev);
smartclip_err_t imu_read_quat(imu_t * dev, float q[4]);
smartclip_err_t imu_get_cal  (imu_t * dev, uint8_t * sys, uint8_t * gyro, uint8_t * acc, uint8_t * mag);

#endif