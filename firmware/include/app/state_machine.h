#ifndef APP_STATE_MACHINE_H
#define APP_STATE_MACHINE_H

#include "app/events.h"
#include "drivers/imu.h"
#include "drivers/baro.h"

void state_machine_init    (imu_t * imu, baro_t * baro);
void state_machine_dispatch(event_t e);

#endif