/* include/app/event_queue.h */
#ifndef APP_EVENT_QUEUE_H
#define APP_EVENT_QUEUE_H

#include <stdbool.h>
#include <stdint.h>
#include "app/events.h"

void evq_init(void);
void evq_post(event_t e);
bool evq_pop (event_t * out);

extern uint32_t g_dropped_events;

#endif