/* bno055.c - see bno055.h for contract */
#include "drivers/bno055.h"

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "bsp/bsp_i2c.h"
#include "app/smartclip_err.h"
#include "app/log.h"
#include "app/assert.h"

/* ---------- Module log tag --------------------------------------- */
#define TAG "bno055"

/* ---------- Register map ----------------------------------------- */
/* All registers in PAGE 0 unless commented. */
enum {
    REG_CHIP_ID      = 0x00,   /* expects 0xA0 */
    REG_PAGE_ID      = 0x07,
    REG_OPR_MODE     = 0x3D,
    REG_PWR_MODE     = 0x3E,
    REG_SYS_TRIGGER  = 0x3F,
    REG_QUA_DATA_W   = 0x20,   /* W,X,Y,Z each 2B LE, total 8B */
    REG_LIA_DATA_X   = 0x28,   /* linear accel, 6B LE */
    REG_CALIB_STAT   = 0x35,
    REG_ACC_OFFSET_X = 0x55,   /* cal profile starts here, 22B */
};

enum { OP_CONFIGMODE = 0x00, OP_NDOF = 0x0C };

#define CHIP_ID_EXPECTED 0xA0

/* ---------- Static instance pool --------------------------------- */
struct bno055 {
    bsp_i2c_t * bus;
    uint8_t     addr;
    uint8_t     cur_page;
    uint8_t     mode;
    bool        in_use;
};

#ifndef CFG_MAX_BNO055
#define CFG_MAX_BNO055 1u
#endif
static struct bno055 g_pool[CFG_MAX_BNO055];

/* ---------- Low-level helpers (static, file-private) ------------- */
static smartclip_err_t reg_write(bno055_t * dev, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { reg, val };
    int r = bsp_i2c_write(dev->bus, dev->addr, buf, sizeof(buf));
    return (r == 0) ? SMARTCLIP_OK : SMARTCLIP_ERR_BUS;
}

static smartclip_err_t reg_read(bno055_t * dev, uint8_t reg,
                                uint8_t * dst, size_t n) {
    int r = bsp_i2c_write_read(dev->bus, dev->addr, &reg, 1, dst, n);
    return (r == 0) ? SMARTCLIP_OK : SMARTCLIP_ERR_BUS;
}

/* ---------- Public API ------------------------------------------- */
bno055_t * bno055_create(const bno055_cfg_t * cfg) {
    ASSERT(cfg != NULL && cfg->bus != NULL);
    for (size_t i = 0; i < CFG_MAX_BNO055; i++) {
        if (!g_pool[i].in_use) {
            g_pool[i].in_use   = true;
            g_pool[i].bus      = cfg->bus;
            g_pool[i].addr     = cfg->addr_7bit;
            g_pool[i].cur_page = 0xFFu;
            g_pool[i].mode     = OP_CONFIGMODE;
            return &g_pool[i];
        }
    }
    LOG_ERROR(TAG, "pool exhausted (size=%u)", (unsigned)CFG_MAX_BNO055);
    return NULL;
}

smartclip_err_t bno055_init(bno055_t * dev) {
    ASSERT(dev != NULL);
    bsp_delay_ms(700);              /* DS table 0-2 worst-case boot */

    uint8_t id = 0;
    smartclip_err_t e = reg_read(dev, REG_CHIP_ID, &id, 1);
    if (e != SMARTCLIP_OK) return e;
    if (id != CHIP_ID_EXPECTED) {
        LOG_ERROR(TAG, "bad CHIP_ID 0x%02X (want 0xA0)", id);
        return SMARTCLIP_ERR_BAD_ID;
    }
    /* ... configure clock, switch to NDOF, etc. ... */
    return SMARTCLIP_OK;
}

/* destroy/deinit and reads omitted for brevity */