#include "app/calibration.h"
#include "app/config_store.h"
#include "app/log.h"
#define TAG "cal"

void calibration_load_or_default(imu_t * imu) {
    (void)imu;
    if (config_store_has_cal()) {
        /* TODO(6.15): uint8_t p[CAL_PROFILE_LEN]; config_store_load_cal(p);
         *             imu_write_profile(imu, p) BEFORE NDOF. */
        LOG_INFO("[%s] saved profile present (load TODO)", TAG);
    } else {
        LOG_INFO("[%s] no saved profile - chip self-calibrates", TAG);
    }
}

void calibration_save(imu_t * imu) {
    (void)imu;
    /* TODO(6.15): imu_read_profile(imu, p) from regs 0x55-0x6A, then save. */
    uint8_t p[CAL_PROFILE_LEN] = {0};   /* placeholder so flow works */
    config_store_save_cal(p);
    LOG_INFO("[%s] profile saved (bytes are placeholder until imu_read_profile)", TAG);
}