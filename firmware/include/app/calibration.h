#ifndef APP_CALIBRATION_H
#define APP_CALIBRATION_H
#include "drivers/imu.h"
/* On boot: load saved profile into BNO055 if present (skips re-converge). */
void calibration_load_or_default(imu_t * imu);
/* When all cal targets met: read 22-byte profile, persist to config_store. */
void calibration_save(imu_t * imu);
#endif