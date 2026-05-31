/*
 * bno055.h - Bosch BNO055 9-axis IMU + fusion driver
 *
 * Bus       : I2C
 * Datasheet : BST-BNO055-DS000-15 (rev. 1.6)
 * Owner     : Andrew - smart-barbell-clip
 *
 * Layering  : driver -> bsp/bsp_i2c -> HAL. Do not include HAL here.
 * Threading : NOT thread-safe by design. Caller serializes access by
 *             only invoking from the main loop (Section 6.3 architecture).
 */
#ifndef DRIVERS_BNO055_H
#define DRIVERS_BNO055_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "bsp/bsp_i2c.h"
#include "app/smartclip_err.h"   /* shared error-code enum (6.2.5) */

/* ---------- Opaque handle (6.2.4) -------------------------------- */
typedef struct bno055 bno055_t;

/* ---------- Configuration struct --------------------------------- */
typedef struct {
    bsp_i2c_t * bus;
    uint8_t     addr_7bit;  /* 0x28 or 0x29 (ADR pin) */
    bool        use_external_xtal;
} bno055_cfg_t;

/* ---------- Lifecycle -------------------------------------------- */
bno055_t *      bno055_create(const bno055_cfg_t * cfg);
smartclip_err_t bno055_init  (bno055_t * dev);
smartclip_err_t bno055_deinit(bno055_t * dev);
void            bno055_destroy(bno055_t * dev);

/* ---------- Operations - the public API -------------------------- */
smartclip_err_t bno055_read_quat       (bno055_t * dev, float q[4]);
smartclip_err_t bno055_read_lin_accel  (bno055_t * dev, float a[3]);
smartclip_err_t bno055_read_cal_status (bno055_t * dev, uint8_t s[4]);
smartclip_err_t bno055_save_cal_profile(bno055_t * dev, uint8_t out[22]);
smartclip_err_t bno055_load_cal_profile(bno055_t * dev, const uint8_t in[22]);

#endif /* DRIVERS_BNO055_H */