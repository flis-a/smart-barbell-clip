#ifndef APP_BATTERY_H
#define APP_BATTERY_H
#include <stdint.h>
#include "app/smartclip_err.h"

/* TODO(6.14): ADC battery telemetry. Deferred - hardware not testable yet.
 * Driver split per plan 6.14.5:
 *   drivers/adc_battery.c : ADC ch + VREFINT + oversampling -> mV
 *   drivers/charger.c     : STAT-pin debounce -> charger_state_t
 *   app/battery.c (this)  : mV -> SOC%, hysteresis, posts E_BATTERY_LOW / E_CHARGER_CONNECTED
 */
typedef enum { CHG_NOT_PRESENT = 0, CHG_CHARGING, CHG_FULL, CHG_FAULT } charger_state_t;

smartclip_err_t battery_init(void);
smartclip_err_t battery_read_soc(uint8_t * soc_pct_out);   /* 0..100 */
charger_state_t battery_charger_state(void);
void            battery_tick(void);   /* TODO: arm on TIMER_SLOT_BATTERY later */

#endif