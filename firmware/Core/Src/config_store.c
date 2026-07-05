#include "app/config_store.h"
#include <string.h>
/* TODO(6.15): back with internal flash config partition. RAM stub for now -
 * profile does NOT survive power cycle yet. */
static uint8_t s_profile[CAL_PROFILE_LEN];
static bool    s_valid = false;

bool config_store_has_cal(void) { return s_valid; }

smartclip_err_t config_store_load_cal(uint8_t out[CAL_PROFILE_LEN]) {
    if (!s_valid) return SMARTCLIP_ERR_NOT_SUPPORTED;
    memcpy(out, s_profile, CAL_PROFILE_LEN);
    return SMARTCLIP_OK;
}
smartclip_err_t config_store_save_cal(const uint8_t in[CAL_PROFILE_LEN]) {
    memcpy(s_profile, in, CAL_PROFILE_LEN);
    s_valid = true;
    return SMARTCLIP_OK;
}