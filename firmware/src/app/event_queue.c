/*
 * event_queue.c - single ring buffer feeding the main loop.
 *
 * Producers: ISRs, BLE callbacks, soft-timer expiry.
 * Consumer : main loop only.
 * Threading: ISR-safe via PRIMASK save/restore in evq_post().
 */
#include "app/event_queue.h"

#include <stdbool.h>
#include <stdint.h>

#include "app/assert.h"
#include "app/events.h"
#include "app/log.h"

#include "stm32wbxx_hal.h"   /* __disable_irq, __get_PRIMASK */

#define TAG "evq"

/* ---------- Sizing -------------------------------------------- */
#define EVQ_SIZE 32u
_Static_assert((EVQ_SIZE & (EVQ_SIZE - 1u)) == 0u,
               "EVQ_SIZE must be power of 2");

/* ---------- File state ---------------------------------------- */
static event_t          q_buf[EVQ_SIZE];
static volatile uint8_t q_head;
static volatile uint8_t q_tail;
uint32_t                g_dropped_events;  /* extern in header, debugger sees it */

/* ---------- Public API ---------------------------------------- */
void evq_init(void) {
    q_head = q_tail = 0u;
    g_dropped_events = 0u;
}

void evq_post(event_t e) {
    uint32_t pri = __get_PRIMASK();
    __disable_irq();
    uint8_t next = (q_head + 1u) & (EVQ_SIZE - 1u);
    if (next == q_tail) {
        g_dropped_events++;
    } else {
        q_buf[q_head] = e;
        q_head = next;
    }
    __set_PRIMASK(pri);
}

bool evq_pop(event_t * out) {
    ASSERT(out != NULL);
    if (q_head == q_tail) return false;
    *out = q_buf[q_tail];
    q_tail = (q_tail + 1u) & (EVQ_SIZE - 1u);
    return true;
}