/* bsp_i2c.h - the one I2C abstraction every driver above uses.
 *
 * Layering: drivers -> bsp_i2c -> HAL. No driver ever calls HAL_I2C_* directly.
 * Threading: NOT thread-safe. Single-threaded main loop only.
 */
#ifndef BSP_BSP_I2C_H
#define BSP_BSP_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32wbxx_hal.h"   /* for I2C_HandleTypeDef in create() */

typedef struct bsp_i2c bsp_i2c_t;   /* opaque - definition in bsp_i2c.c */

/* Lifecycle. Pass the CubeMX-generated handle (e.g. &hi2c1). */
bsp_i2c_t * bsp_i2c_create(I2C_HandleTypeDef * hi2c);

/* Returns 0 on success, -1 on bus error / NACK / timeout. */
int bsp_i2c_write     (bsp_i2c_t * bus, uint8_t addr_7bit,
                       const uint8_t * src, size_t n);
int bsp_i2c_read      (bsp_i2c_t * bus, uint8_t addr_7bit,
                       uint8_t * dst, size_t n);
int bsp_i2c_write_read(bsp_i2c_t * bus, uint8_t addr_7bit,
                       const uint8_t * tx, size_t tx_n,
                       uint8_t * rx, size_t rx_n);

/* Bring-up tool: walks 0x08..0x77, records addresses that ACK.
 * Returns number of devices found in *out_n. */
int bsp_i2c_scan      (bsp_i2c_t * bus,
                       uint8_t * out_addrs, size_t cap, size_t * out_n);

/* Used by drivers that need millisecond-scale waits (BNO055 boot, etc.). */
void bsp_delay_ms(uint32_t ms);

#endif /* BSP_BSP_I2C_H */