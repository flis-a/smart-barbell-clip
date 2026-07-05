#ifndef APP_CONFIG_STORE_H
#define APP_CONFIG_STORE_H
#include <stdbool.h>
#include <stdint.h>
#include "app/smartclip_err.h"
#define CAL_PROFILE_LEN 22u   /* BNO055 regs 0x55-0x6A */

bool            config_store_has_cal(void);
smartclip_err_t config_store_load_cal(uint8_t out[CAL_PROFILE_LEN]);
smartclip_err_t config_store_save_cal(const uint8_t in[CAL_PROFILE_LEN]);
#endif