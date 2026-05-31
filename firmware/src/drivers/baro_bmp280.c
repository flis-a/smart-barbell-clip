/* baro_bmp280.c - BMP280 implementation of drivers/baro.h.
 * Wraps the ebrezadev dependency-injection BMP280 driver onto bsp_i2c. */
#include "drivers/baro.h"

#include <stdint.h>
#include <stdbool.h>

#include "app/assert.h"
#include "app/log.h"
#include "bsp/bsp_i2c.h"

#include "bmp280.h"   /* ebrezadev driver (third_party/bmp280-ebreza/inc) */

#define TAG "baro"

struct baro {
    bmp280_handle_t h;
    bsp_i2c_t *     bus;
    uint8_t         addr;
    bool            in_use;
};

#ifndef CFG_MAX_BARO
#define CFG_MAX_BARO 1u
#endif
static struct baro g_pool[CFG_MAX_BARO];
static struct baro * s_active = NULL;   /* shims find the bus through this */

/* ---------- Interface shims --------------------------------------------
 * CONFIRM (1): these signatures must match the typedefs the driver expects.
 * Open third_party/bmp280-ebreza/example/ and copy the exact prototypes;
 * adjust return type (int vs int8_t) and the length type if they differ.
 * All return 0 on success per the README. */
static int if_init(uint8_t address) {
    (void)address;        /* bus already up; nothing to do */
    return 0;
}

static int if_deinit(uint8_t address) {
    (void)address;
    return 0;
}

static int if_write(uint8_t address, uint8_t reg_addr,
                    uint8_t * data, uint8_t length) {
    ASSERT(s_active != NULL);
    uint8_t buf[32];
    ASSERT((size_t)length + 1u <= sizeof(buf));
    buf[0] = reg_addr;
    for (uint8_t i = 0; i < length; i++) buf[i + 1] = data[i];
    return (bsp_i2c_write(s_active->bus, address, buf,
                          (size_t)(length + 1u)) == 0) ? 0 : 1;
}

static int if_read(uint8_t address, uint8_t reg_addr,
                   uint8_t * data, uint8_t length) {
    ASSERT(s_active != NULL);
    return (bsp_i2c_write_read(s_active->bus, address,
                               &reg_addr, 1, data, (size_t)length) == 0) ? 0 : 1;
}

static int if_delay(uint32_t delay_ms) {
    bsp_delay_ms(delay_ms);
    return 0;
}

/* ---------- Public API -------------------------------------------------- */
baro_t * baro_create(const baro_cfg_t * cfg) {
    ASSERT(cfg != NULL && cfg->bus != NULL);
    for (size_t i = 0; i < CFG_MAX_BARO; i++) {
        if (!g_pool[i].in_use) {
            g_pool[i].in_use = true;
            g_pool[i].bus    = cfg->bus;
            g_pool[i].addr   = cfg->addr_7bit;

            /* Wire the dependency interface (README field names). */
            g_pool[i].h.dependency_interface.bmp280_interface_init   = if_init;
            g_pool[i].h.dependency_interface.bmp280_interface_deinit = if_deinit;
            g_pool[i].h.dependency_interface.bmp280_write_array      = if_write;
            g_pool[i].h.dependency_interface.bmp280_read_array       = if_read;
            g_pool[i].h.dependency_interface.bmp280_delay_function   = if_delay;
            /* bmp280_power_function omitted: BMP280_INCLUDE_ALTITUDE == 0 */

            s_active = &g_pool[i];
            LOG_INFO("[%s] created (addr 0x%02X)", TAG, cfg->addr_7bit);
            return &g_pool[i];
        }
    }
    LOG_ERROR("[%s] pool exhausted", TAG);
    return NULL;
}

smartclip_err_t baro_init(baro_t * dev) {
    ASSERT(dev != NULL);
    s_active = dev;

    /* CONFIRM (2): the init enum + address constant names. The README shows
     * bmp280_init(&handle, BMP280_I2C, BMP280_I2C_ADDRESS_1). Your 0x76 board
     * is the "primary"/"address 1" variant. Match these names to the header. */
    bmp280_error_code_t e = bmp280_init(&dev->h, BMP280_I2C, dev->addr);
    if (e != BMP280_ERROR_OK) {
        LOG_ERROR("[%s] bmp280_init failed (e=%d)", TAG, (int)e);
        return SMARTCLIP_ERR_BUS;
    }
    LOG_INFO("[%s] init OK", TAG);
    return SMARTCLIP_OK;
}

smartclip_err_t baro_read(baro_t * dev, int32_t * pressure_pa, float * temp_c) {
    ASSERT(dev != NULL && pressure_pa != NULL && temp_c != NULL);
    s_active = dev;

    bmp280_sensors_data_t data;
    bmp280_error_code_t e = bmp280_get_all(&dev->h, &data);  /* swap name if grep differs */
    if (e != BMP280_ERROR_OK) return SMARTCLIP_ERR_BUS;

    *temp_c      = (float)data.temperature;
    *pressure_pa = (int32_t)(data.pressure + 0.5f);
    return SMARTCLIP_OK;
}