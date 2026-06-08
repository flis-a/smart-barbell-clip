/*
 * ws2812b.h - WS2812B LED driver
 *
 * Bus       : TIM (PWM)
 * Datasheet : WS2812B (rev. 1.0)
 * Owner     : Andrew - smart-barbell-clip
 *
 * Layering  : driver -> bsp/bsp_tim -> HAL. Do not include HAL here.
 * Threading : called from main context; ws2812_show() kicks off a DMA transfer,
 *             so callbacks may be from ISR context.
 */
#ifndef DRIVERS_WS2812B_H
#define DRIVERS_WS2812B_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "tim.h"
#include "app/smartclip_err.h"

/* ---------- Opaque handle (6.2.4) -------------------------------- */
typedef struct ws2812 ws2812_t;

/* ---------- Configuration struct --------------------------------- */
typedef struct {
    TIM_HandleTypeDef * htim;
    uint32_t            channel;     /* TIM_CHANNEL_x */
    uint16_t            num_leds;
} ws2812_cfg_t;

/* ---------- Lifecycle -------------------------------------------- */
ws2812_t *      ws2812_create  (const ws2812_cfg_t * cfg);
void            ws2812_set_pixel(ws2812_t * dev, uint16_t i, uint8_t r, uint8_t g, uint8_t b);
void            ws2812_clear   (ws2812_t * dev);
smartclip_err_t ws2812_show    (ws2812_t * dev);   /* kicks DMA */

#endif /* DRIVERS_WS2812B_H */