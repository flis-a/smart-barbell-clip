/* include/app/timers.h */
#ifndef APP_TIMERS_H
#define APP_TIMERS_H

#include <stdbool.h>
#include <stdint.h>
#include "app/smartclip_err.h"

/* Named timer slots. Onboard LEDs are driven via these + the state machine. */
enum {
    /* --- Active in Phase 6 sensor bring-up --- */
    TIMER_SLOT_IMU       = 0,   /* 100 Hz quaternion sampling (6.8)   */
    TIMER_SLOT_BARO      = 1,   /*  25 Hz pressure sampling (6.9)     */
    TIMER_SLOT_CAL_POLL  = 2,   /* cal-status poll while calibrating  */
    TIMER_SLOT_CAL_BLINK = 3,   /* blue LED blink while calibrating   */

    /* --- Skeleton: reserved for upcoming sections --- */
    TIMER_SLOT_DISPLAY   = 4,   /* OLED dirty-flush refresh (6.11)         */
    TIMER_SLOT_ANIM      = 5,   /* WS2812 animation step (6.12)            */
    TIMER_SLOT_BATTERY   = 6,   /* battery sample + hysteresis (6.14)      */
    TIMER_SLOT_BLE_ADV   = 7,   /* advertising timeout (6.18)              */
    TIMER_SLOT_BUTTON    = 8,   /* button poll/debounce + long-press (6.15)*/

    TIMER_POOL_SIZE      = 12,  /* array size; slots 9-11 spare headroom   */
};

void            timers_init    (void);
smartclip_err_t timer_arm      (uint8_t slot, uint32_t period_ms,
                                uint8_t event_type, bool repeating);
void            timer_stop     (uint8_t slot);
void            timer_tick_1ms (void);   /* call from SysTick_Handler */

#endif