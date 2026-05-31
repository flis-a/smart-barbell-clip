/* include/drivers/baro.h - abstract barometer interface.
 *
 * v1 implementation: baro_bmp280.c (wraps Bosch BMP280_driver).
 * Swappable for baro_lps28dfw.c later without touching callers.
 */
#ifndef DRIVERS_BARO_H
#define DRIVERS_BARO_H

#include <stdint.h>

#include "app/smartclip_err.h"
#include "bsp/bsp_i2c.h"

typedef struct baro baro_t;

typedef struct {
    bsp_i2c_t * bus;
    uint8_t     addr_7bit;    /* BMP280: 0x76 (SDO low) or 0x77 (SDO high) */
} baro_cfg_t;

baro_t *        baro_create(const baro_cfg_t * cfg);
smartclip_err_t baro_init  (baro_t * dev);
smartclip_err_t baro_read  (baro_t * dev, int32_t * pressure_pa, float * temp_c);

#endif