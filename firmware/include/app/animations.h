#ifndef APP_ANIMATIONS_H
#define APP_ANIMATIONS_H

#include <stdint.h>
#include "drivers/ws2812b.h"

typedef enum {
    ANIM_OFF = 0,
    ANIM_SOLID,
    ANIM_BREATHE,
    ANIM_BLINK,
    ANIM_CHASE,
    ANIM_FLASH,    /* flash N times, then settle into a target animation */
} anim_kind_t;

void anim_init(ws2812_t * strip, uint16_t num_leds);
void anim_set (anim_kind_t kind, uint8_t r, uint8_t g, uint8_t b);

/* Flash (r,g,b) `count` times, then switch to (then_kind, tr,tg,tb). */
void anim_flash(uint8_t count, uint8_t r, uint8_t g, uint8_t b,
                anim_kind_t then_kind, uint8_t tr, uint8_t tg, uint8_t tb);

void anim_tick(void);   /* call ~50 Hz from E_ANIM_TICK */

#endif