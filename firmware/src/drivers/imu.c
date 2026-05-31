/* imu_bno055.c - BNO055 implementation of drivers/imu.h.
 *
 * Bridges Bosch's portable SensorAPI (third_party/bno055-sensorapi/) onto
 * our bsp_i2c bus layer.  All Bosch symbols (bno055_*) stay private to this
 * file; the rest of the firmware only sees the imu_* API.
 */
#include "drivers/imu.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app/assert.h"
#include "app/log.h"
#include "bsp/bsp_i2c.h"

/* Bosch vendor library - the only file that includes this. */
#include "bno055.h"

#define TAG "imu"

/* ---------- Static instance pool ----------------------------------- */
struct imu {
    struct bno055_t  bosch;       /* Bosch's API state, including fn pointers */
    bsp_i2c_t *      bus;
    uint8_t          addr;
    bool             in_use;
};

#ifndef CFG_MAX_IMU
#define CFG_MAX_IMU 1u
#endif
static struct imu g_pool[CFG_MAX_IMU];

/* Bosch's bus callbacks receive only dev_addr - this back-pointer lets the
 * shims find the bsp_i2c_t. Safe because we're single-threaded with one IMU. */
static struct imu * s_active = NULL;

/* ---------- Bus shims matching Bosch's function-pointer types ------ */
static s8 i2c_bus_write(u8 dev_addr, u8 reg_addr, u8 * reg_data, u8 wr_len) {
    ASSERT(s_active != NULL);
    uint8_t buf[32];
    ASSERT((size_t)wr_len + 1u <= sizeof(buf));
    buf[0] = reg_addr;
    memcpy(&buf[1], reg_data, wr_len);
    int r = bsp_i2c_write(s_active->bus, dev_addr, buf, (size_t)(wr_len + 1u));
    return (r == 0) ? 0 : -1;
}

static s8 i2c_bus_read(u8 dev_addr, u8 reg_addr, u8 * reg_data, u8 r_len) {
    ASSERT(s_active != NULL);
    int r = bsp_i2c_write_read(s_active->bus, dev_addr,
                               &reg_addr, 1, reg_data, (size_t)r_len);
    return (r == 0) ? 0 : -1;
}

static void delay_msec(u32 ms) {
    bsp_delay_ms(ms);
}

/* ---------- Public API --------------------------------------------- */
imu_t * imu_create(const imu_cfg_t * cfg) {
    ASSERT(cfg != NULL && cfg->bus != NULL);
    for (size_t i = 0; i < CFG_MAX_IMU; i++) {
        if (!g_pool[i].in_use) {
            g_pool[i].in_use = true;
            g_pool[i].bus    = cfg->bus;
            g_pool[i].addr   = cfg->addr_7bit;

            /* Wire Bosch's struct to our shims BEFORE calling any bno055_*. */
            g_pool[i].bosch.bus_write  = i2c_bus_write;
            g_pool[i].bosch.bus_read   = i2c_bus_read;
            g_pool[i].bosch.delay_msec = delay_msec;
            g_pool[i].bosch.dev_addr   = cfg->addr_7bit;

            s_active = &g_pool[i];
            LOG_INFO("[%s] created (addr 0x%02X)", TAG, cfg->addr_7bit);
            return &g_pool[i];
        }
    }
    LOG_ERROR("[%s] pool exhausted", TAG);
    return NULL;
}

smartclip_err_t imu_init(imu_t * dev) {
    ASSERT(dev != NULL);
    s_active = dev;

    /* Now bno055_init refers unambiguously to Bosch's library function. */
    s32 r = bno055_init(&dev->bosch);
    if (r != BNO055_SUCCESS) {
        LOG_ERROR("[%s] bosch init failed (r=%ld)", TAG, (long)r);
        return SMARTCLIP_ERR_BUS;
    }

    r = bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
    if (r != BNO055_SUCCESS) {
        LOG_ERROR("[%s] set NDOF failed (r=%ld)", TAG, (long)r);
        return SMARTCLIP_ERR_BUS;
    }

    LOG_INFO("[%s] init OK, mode=NDOF", TAG);
    return SMARTCLIP_OK;
}

smartclip_err_t imu_read_quat(imu_t * dev, float q[4]) {
    ASSERT(dev != NULL && q != NULL);
    s_active = dev;

    struct bno055_quaternion_t raw;
    s32 r = bno055_read_quaternion_wxyz(&raw);
    if (r != BNO055_SUCCESS) return SMARTCLIP_ERR_BUS;

    /* Bosch returns int16 LSB; quaternion scaling is 2^14 = 16384. */
    const float scale = 1.0f / 16384.0f;
    q[0] = (float)raw.w * scale;
    q[1] = (float)raw.x * scale;
    q[2] = (float)raw.y * scale;
    q[3] = (float)raw.z * scale;
    return SMARTCLIP_OK;
}

smartclip_err_t imu_get_cal(imu_t * dev, uint8_t * sys, uint8_t * gyro,
                            uint8_t * acc, uint8_t * mag) {
    ASSERT(dev != NULL);
    s_active = dev;

    u8 s = 0, g = 0, a = 0, m = 0;
    if (bno055_get_sys_calib_stat(&s)   != BNO055_SUCCESS) return SMARTCLIP_ERR_BUS;
    if (bno055_get_gyro_calib_stat(&g)  != BNO055_SUCCESS) return SMARTCLIP_ERR_BUS;
    if (bno055_get_accel_calib_stat(&a) != BNO055_SUCCESS) return SMARTCLIP_ERR_BUS;
    if (bno055_get_mag_calib_stat(&m)   != BNO055_SUCCESS) return SMARTCLIP_ERR_BUS;

    *sys = s; *gyro = g; *acc = a; *mag = m;
    return SMARTCLIP_OK;
}