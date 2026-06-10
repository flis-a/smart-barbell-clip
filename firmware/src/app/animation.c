/* src/app/animations.c - WS2812 animation engine, ticked at ~50 Hz. */
#include "app/animations.h"
#include "app/assert.h"
#include "drivers/ws2812b.h"

#define ANIM_TICK_HZ       50u
#define BREATHE_PERIOD_MS  2000u
#define BLINK_PERIOD_MS    400u
#define FLASH_PERIOD_MS    200u
#define CHASE_STEP_MS      80u

static ws2812_t *  s_strip = NULL;
static uint16_t    s_n     = 0;

static anim_kind_t s_kind  = ANIM_OFF;
static uint8_t     s_r, s_g, s_b;
static uint32_t    s_phase = 0;          /* +1 each tick */

/* flash-then-settle bookkeeping */
static uint8_t     s_flash_left = 0;
static anim_kind_t s_then_kind;
static uint8_t     s_then_r, s_then_g, s_then_b;

void anim_init(ws2812_t * strip, uint16_t num_leds) {
    ASSERT(strip != NULL);
    s_strip = strip;
    s_n     = num_leds;
    s_kind  = ANIM_OFF;
    s_phase = 0;
}

void anim_set(anim_kind_t kind, uint8_t r, uint8_t g, uint8_t b) {
    s_kind = kind;
    s_r = r; s_g = g; s_b = b;
    s_flash_left = 0;
    s_phase = 0;                          /* crisp restart on every change */
}

void anim_flash(uint8_t count, uint8_t r, uint8_t g, uint8_t b,
                anim_kind_t then_kind, uint8_t tr, uint8_t tg, uint8_t tb) {
    s_kind = ANIM_FLASH;
    s_r = r; s_g = g; s_b = b;
    s_flash_left = count;
    s_then_kind = then_kind; s_then_r = tr; s_then_g = tg; s_then_b = tb;
    s_phase = 0;
}

/* scale channel by level 0..255 with squared gamma for perceptual smoothness */
static inline uint8_t scale(uint8_t c, uint8_t level) {
    uint16_t gl = ((uint16_t)level * level) / 255u;
    return (uint8_t)(((uint16_t)c * gl) / 255u);
}

static void fill(uint8_t r, uint8_t g, uint8_t b) {
    for (uint16_t i = 0; i < s_n; i++) ws2812_set_pixel(s_strip, i, r, g, b);
}

void anim_tick(void) {
    if (s_strip == NULL) return;

    switch (s_kind) {
    case ANIM_OFF:
        ws2812_clear(s_strip);
        break;

    case ANIM_SOLID:
        fill(s_r, s_g, s_b);
        break;

    case ANIM_BREATHE: {
        uint32_t period = (BREATHE_PERIOD_MS * ANIM_TICK_HZ) / 1000u;
        uint32_t half   = period / 2u;
        uint32_t p      = s_phase % period;
        uint8_t  level  = (p < half)
                        ? (uint8_t)((p * 255u) / half)
                        : (uint8_t)(((period - p) * 255u) / half);
        fill(scale(s_r, level), scale(s_g, level), scale(s_b, level));
        break;
    }

    case ANIM_BLINK: {
        uint32_t period = (BLINK_PERIOD_MS * ANIM_TICK_HZ) / 1000u;
        if ((s_phase % period) < (period / 2u)) fill(s_r, s_g, s_b);
        else                                    ws2812_clear(s_strip);
        break;
    }

    case ANIM_CHASE: {
        uint32_t step = (CHASE_STEP_MS * ANIM_TICK_HZ) / 1000u;
        if (step == 0) step = 1;
        uint16_t head = (uint16_t)((s_phase / step) % s_n);
        for (uint16_t i = 0; i < s_n; i++) {
            uint16_t d = (uint16_t)((s_n + head - i) % s_n);   /* dist behind head */
            uint8_t level = (d == 0) ? 255 : (d == 1) ? 80 : (d == 2) ? 20 : 0;
            ws2812_set_pixel(s_strip, i,
                             scale(s_r, level), scale(s_g, level), scale(s_b, level));
        }
        break;
    }

    case ANIM_FLASH: {
        uint32_t period = (FLASH_PERIOD_MS * ANIM_TICK_HZ) / 1000u;
        uint32_t p      = s_phase % period;
        if (p < (period / 2u)) fill(s_r, s_g, s_b);
        else                   ws2812_clear(s_strip);
        if (p == period - 1u) {                 /* a full flash just completed */
            if (s_flash_left > 0) s_flash_left--;
            if (s_flash_left == 0) {
                anim_set(s_then_kind, s_then_r, s_then_g, s_then_b);
            }
        }
        break;
    }
    }

    ws2812_show(s_strip);
    s_phase++;
}