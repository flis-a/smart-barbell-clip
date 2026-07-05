#include "app/battery.h"
#include "app/log.h"
#define TAG "battery"

/* TODO(6.14): implement. See plan 6.14.1-6.14.7. Currently a compiling stub. */
smartclip_err_t battery_init(void) {
    LOG_WARN("[%s] stub - not implemented (6.14)", TAG);
    return SMARTCLIP_ERR_NOT_SUPPORTED;
}
smartclip_err_t battery_read_soc(uint8_t * soc) {
    if (soc) *soc = 100;            /* safe placeholder so callers don't div-by-zero */
    return SMARTCLIP_ERR_NOT_SUPPORTED;
}
charger_state_t battery_charger_state(void) { return CHG_NOT_PRESENT; }
void battery_tick(void) { /* TODO(6.14) */ }