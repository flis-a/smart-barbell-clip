/* ws2812b.c - see ws2812b.h for contract */
#include "drivers/ws2812b.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "tim.h"
#include "app/assert.h"
#include "app/log.h"
#include "app/smartclip_err.h"

#define TAG "ws2812b"

/* Scope-verified timing for TIM2 @ 32 MHz, ARR=39 (1.25 us bit). */
#define WS_CCR_0        13u    /* 0.40 us high */
#define WS_CCR_1        26u    /* 0.80 us high */
#define WS_BITS_PER_LED 24u
#define WS_RESET_SLOTS  48u    /* ~60 us low >= 50 us latch */

#ifndef CFG_MAX_WS2812
#define CFG_MAX_WS2812  1u
#endif
#ifndef CFG_MAX_LEDS
#define CFG_MAX_LEDS    16u    /* size for your worst case */
#endif

struct ws2812 {
    TIM_HandleTypeDef * htim;
    uint32_t            channel;
    uint16_t            num_leds;
    uint16_t            len;        /* num_leds*24 + reset slots */
    uint16_t            buf[CFG_MAX_LEDS * WS_BITS_PER_LED + WS_RESET_SLOTS];
    volatile bool       busy;
    bool                in_use;
};

static struct ws2812   g_pool[CFG_MAX_WS2812];
static struct ws2812 * s_active = NULL;   /* for the DMA-complete callback */

/* One color byte -> 8 CCR values, MSB first. */
static void encode_byte(uint16_t * dst, uint8_t byte) {
    for (int bit = 7; bit >= 0; --bit)
        *dst++ = (byte & (1u << bit)) ? WS_CCR_1 : WS_CCR_0;
}

ws2812_t * ws2812_create(const ws2812_cfg_t * cfg) {
    ASSERT(cfg != NULL && cfg->htim != NULL);
    ASSERT(cfg->num_leds <= CFG_MAX_LEDS);

    for (size_t i = 0; i < CFG_MAX_WS2812; i++) {
        if (!g_pool[i].in_use) {
            g_pool[i].in_use   = true;
            g_pool[i].htim     = cfg->htim;
            g_pool[i].channel  = cfg->channel;
            g_pool[i].num_leds = cfg->num_leds;
            g_pool[i].len      = cfg->num_leds * WS_BITS_PER_LED + WS_RESET_SLOTS;
            g_pool[i].busy     = false;

            for (uint16_t k = 0; k < g_pool[i].len; k++) g_pool[i].buf[k] = 0;

            s_active = &g_pool[i];
            LOG_INFO("[%s] created (%u leds, len=%u)", TAG,
                     cfg->num_leds, g_pool[i].len);
            return &g_pool[i];
        }
    }
    LOG_ERROR("[%s] pool exhausted", TAG);
    return NULL;
}

void ws2812_set_pixel(ws2812_t * d, uint16_t i, uint8_t r, uint8_t g, uint8_t b) {
    ASSERT(d != NULL && i < d->num_leds);
    uint16_t * base = &d->buf[i * WS_BITS_PER_LED];
    encode_byte(base + 0,  g);    /* GRB order! */
    encode_byte(base + 8,  r);
    encode_byte(base + 16, b);
}

void ws2812_clear(ws2812_t * d) {
    ASSERT(d != NULL);
    for (uint16_t i = 0; i < d->num_leds; i++) ws2812_set_pixel(d, i, 0, 0, 0);
}

smartclip_err_t ws2812_show(ws2812_t * d) {
    ASSERT(d != NULL);
    if (d->busy) return SMARTCLIP_ERR_BUSY;     /* last frame still going */
    d->busy = true;
    if (HAL_TIM_PWM_Start_DMA(d->htim, d->channel,
                              (uint32_t *)d->buf, d->len) != HAL_OK) {
        d->busy = false;
        return SMARTCLIP_ERR_BUS;
    }
    return SMARTCLIP_OK;
}

/* DMA-complete: stop PWM so the line idles low, clear busy. */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef * htim) {
    if (s_active != NULL && htim == s_active->htim) {
        HAL_TIM_PWM_Stop_DMA(s_active->htim, s_active->channel);
        s_active->busy = false;
    }
}