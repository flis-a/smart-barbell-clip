/*
 * timers.c - soft-timer pool. Expirations post events, not callbacks.
 *
 * Owner    : Andrew - smart-barbell-clip
 * Tick src : SysTick (1 ms). Move to TIM6 later w/o API change.
 * Threading: timer_arm/stop are main-thread only; timer_tick_1ms is ISR.
 */
#include "app/timers.h"

#include <stdbool.h>
#include <stdint.h>

#include "app/assert.h"
#include "app/event_queue.h"
#include "app/events.h"
#include "app/log.h"
#include "app/smartclip_err.h"

#define TAG "tim"
#define TIMER_POOL_SIZE 8u   /* Phase 6 uses ~5; leave headroom */

typedef struct {
    uint32_t deadline_ms;
    uint32_t period_ms;
    uint8_t  event_type;
    bool     repeating;
    bool     in_use;
} timer_t;

static timer_t           g_pool[TIMER_POOL_SIZE];
static volatile uint32_t g_now_ms;

/* ---------- Public API ---------------------------------------- */
void timers_init(void) {
    for (uint8_t i = 0; i < TIMER_POOL_SIZE; ++i) g_pool[i].in_use = false;
    g_now_ms = 0u;
}

smartclip_err_t timer_arm(uint8_t  slot,
                          uint32_t period_ms,
                          uint8_t  event_type,
                          bool     repeating) {
    ASSERT(slot < TIMER_POOL_SIZE);
    g_pool[slot] = (timer_t){
        .deadline_ms = g_now_ms + period_ms,
        .period_ms   = period_ms,
        .event_type  = event_type,
        .repeating   = repeating,
        .in_use      = true,
    };
    return SMARTCLIP_OK;
}

void timer_stop(uint8_t slot) {
    ASSERT(slot < TIMER_POOL_SIZE);
    g_pool[slot].in_use = false;
}

/* ---------- Called from SysTick_Handler ----------------------- */
void timer_tick_1ms(void) {
    g_now_ms++;
    for (uint8_t i = 0; i < TIMER_POOL_SIZE; ++i) {
        timer_t * t = &g_pool[i];
        if (!t->in_use) continue;
        /* signed compare = wraparound-safe */
        if ((int32_t)(g_now_ms - t->deadline_ms) < 0) continue;

        evq_post((event_t){
            .type  = t->event_type,
            .arg8  = i,    /* which slot fired - useful when timers share an event type */
        });

        if (t->repeating) t->deadline_ms += t->period_ms;
        else              t->in_use = false;
    }
}