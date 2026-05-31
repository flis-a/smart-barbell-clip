/* bsp_i2c.c - the one place HAL_I2C_* is called. */
#include "bsp/bsp_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app/assert.h"
#include "app/log.h"
#include "stm32wbxx_hal.h"

#define TAG "i2c"
#define BSP_I2C_TIMEOUT_MS  100u

/* ---------- Static instance pool ---------------------------------- */
#ifndef CFG_MAX_I2C_BUSES
#define CFG_MAX_I2C_BUSES 1u
#endif

struct bsp_i2c {
    I2C_HandleTypeDef * hi2c;
    bool                in_use;
};

static struct bsp_i2c g_pool[CFG_MAX_I2C_BUSES];

/* ---------- Forward decls of file-private helpers ----------------- */
static void handle_err(struct bsp_i2c * bus, HAL_StatusTypeDef st);

/* ---------- Lifecycle --------------------------------------------- */
bsp_i2c_t * bsp_i2c_create(I2C_HandleTypeDef * hi2c) {
    ASSERT(hi2c != NULL);
    for (size_t i = 0; i < CFG_MAX_I2C_BUSES; i++) {
        if (!g_pool[i].in_use) {
            g_pool[i].in_use = true;
            g_pool[i].hi2c   = hi2c;
            LOG_INFO("[%s] bus %u created (Instance=%p)",
                     TAG, (unsigned)i, (void *)hi2c->Instance);
            return &g_pool[i];
        }
    }
    LOG_ERROR("[%s] pool exhausted (cap=%u)", TAG, (unsigned)CFG_MAX_I2C_BUSES);
    return NULL;
}

/* ---------- Public API -------------------------------------------- */
int bsp_i2c_write(bsp_i2c_t * bus, uint8_t addr_7bit,
                  const uint8_t * src, size_t n) {
    ASSERT(bus != NULL && src != NULL);
    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(
        bus->hi2c,
        (uint16_t)(addr_7bit << 1u),
        (uint8_t *)src,
        (uint16_t)n,
        BSP_I2C_TIMEOUT_MS);
    if (st != HAL_OK) {
        LOG_WARN("[%s] write 0x%02X failed (st=%d)", TAG, addr_7bit, (int)st);
        handle_err(bus, st);
        return -1;
    }
    return 0;
}

int bsp_i2c_read(bsp_i2c_t * bus, uint8_t addr_7bit,
                 uint8_t * dst, size_t n) {
    ASSERT(bus != NULL && dst != NULL);
    HAL_StatusTypeDef st = HAL_I2C_Master_Receive(
        bus->hi2c,
        (uint16_t)(addr_7bit << 1u),
        dst,
        (uint16_t)n,
        BSP_I2C_TIMEOUT_MS);
    if (st != HAL_OK) {
        LOG_WARN("[%s] read 0x%02X failed (st=%d)", TAG, addr_7bit, (int)st);
        handle_err(bus, st);
        return -1;
    }
    return 0;
}

int bsp_i2c_write_read(bsp_i2c_t * bus, uint8_t addr_7bit,
                       const uint8_t * tx, size_t tx_n,
                       uint8_t * rx, size_t rx_n) {
    ASSERT(bus != NULL && tx != NULL && rx != NULL);

    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(
        bus->hi2c, (uint16_t)(addr_7bit << 1u),
        (uint8_t *)tx, (uint16_t)tx_n, BSP_I2C_TIMEOUT_MS);
    if (st != HAL_OK) { handle_err(bus, st); return -1; }

    st = HAL_I2C_Master_Receive(
        bus->hi2c, (uint16_t)(addr_7bit << 1u),
        rx, (uint16_t)rx_n, BSP_I2C_TIMEOUT_MS);
    if (st != HAL_OK) { handle_err(bus, st); return -1; }

    return 0;
}

int bsp_i2c_scan(bsp_i2c_t * bus,
                 uint8_t * out_addrs, size_t cap, size_t * out_n) {
    ASSERT(bus != NULL && out_addrs != NULL && out_n != NULL);

    size_t found = 0;
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        /* HAL_I2C_IsDeviceReady is built-in and uses a single START/STOP
         * with no data phase - the cleanest "is there an ACK at this addr?" probe. */
        HAL_StatusTypeDef st = HAL_I2C_IsDeviceReady(
            bus->hi2c, (uint16_t)(a << 1u), 1, 10);
        if (st == HAL_OK && found < cap) {
            out_addrs[found++] = a;
            LOG_INFO("[%s] device at 0x%02X", TAG, a);
        }
    }
    *out_n = found;
    LOG_INFO("[%s] scan complete, %u device(s)", TAG, (unsigned)found);
    return 0;
}

void bsp_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

/* ---------- Bus recovery ------------------------------------------ */
/* Slave got stuck holding SDA low (master reset mid-transaction).
 * Manually clock SCL up to 9 cycles until SDA releases, then issue a STOP. */
static void handle_err(struct bsp_i2c * bus, HAL_StatusTypeDef st) {
    if (st == HAL_BUSY || st == HAL_ERROR) {
        LOG_WARN("[%s] attempting bus recovery", TAG);
        HAL_I2C_DeInit(bus->hi2c);
        /* Pin recovery would clock SCL by hand here; for v1 a DeInit/Init
         * pair is usually enough since CubeMX-generated init reconfigures
         * the pins as alternate-function open-drain from scratch. */
        HAL_I2C_Init(bus->hi2c);
    }
}