/* src/app/state_machine.c - sensor bring-up: init, calibrate, stream.
 * Onboard LEDs: blue blinks rapidly while calibrating, green solid while streaming. */
#include "app/state_machine.h"

#include <stdbool.h>
#include <stdint.h>

#include "app/assert.h"
#include "app/event_queue.h"
#include "app/events.h"
#include "app/log.h"
#include "app/timers.h"

#include "drivers/imu.h"
#include "drivers/baro.h"

#include "main.h"
#include "stm32wbxx_hal.h"

#define TAG "sm"

#define GREEN_PORT GPIOB
#define GREEN_PIN  GPIO_PIN_0
#define BLUE_PORT  GPIOA
#define BLUE_PIN   GPIO_PIN_4

#define CAL_MIN_HOLD_MS  3000u   /* blink at least this long           */
#define CAL_BLINK_MS     100u    /* blue toggle period -> ~5 Hz blink  */

typedef enum {
    S_BOOT, S_INIT, S_CALIBRATING, S_STREAMING, S_ERROR, S_COUNT
} state_t;
static const char * STATE_NAMES[] = {
    "BOOT", "INIT", "CALIBRATING", "STREAMING", "ERROR"
};

static state_t  g_state;
static imu_t  * g_imu;
static baro_t * g_baro;
static uint32_t g_cal_entry_ms;

/* ---------- Helpers ------------------------------------------------ */
static void leds_off(void) {
    HAL_GPIO_WritePin(GREEN_PORT, GREEN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BLUE_PORT,  BLUE_PIN,  GPIO_PIN_RESET);
}

static void transition(state_t next);

static void on_entry(state_t s) {
    switch (s) {
    case S_CALIBRATING:
        g_cal_entry_ms = HAL_GetTick();
        HAL_GPIO_WritePin(GREEN_PORT, GREEN_PIN, GPIO_PIN_RESET);
        LOG_INFO("[%s] calibrating - tumble the board slowly", TAG);
        timer_arm(TIMER_SLOT_CAL_POLL,  500,          E_IMU_CAL_DONE, true);
        timer_arm(TIMER_SLOT_CAL_BLINK, CAL_BLINK_MS, E_CAL_BLINK,    true);
        break;
    case S_STREAMING:
        HAL_GPIO_WritePin(GREEN_PORT, GREEN_PIN, GPIO_PIN_SET);   /* solid green */
        HAL_GPIO_WritePin(BLUE_PORT,  BLUE_PIN,  GPIO_PIN_RESET); /* blue off    */
        LOG_INFO("[%s] streaming: IMU 100Hz, baro 25Hz", TAG);
        timer_arm(TIMER_SLOT_IMU,  10, E_IMU_SAMPLE,  true);
        timer_arm(TIMER_SLOT_BARO, 40, E_BARO_SAMPLE, true);
        break;
    case S_ERROR:
        leds_off();
        break;
    default: break;
    }
}

static void on_exit_s(state_t s) {
    switch (s) {
    case S_CALIBRATING:
        timer_stop(TIMER_SLOT_CAL_POLL);
        timer_stop(TIMER_SLOT_CAL_BLINK);
        HAL_GPIO_WritePin(BLUE_PORT, BLUE_PIN, GPIO_PIN_RESET);
        break;
    case S_STREAMING:
        timer_stop(TIMER_SLOT_IMU);
        timer_stop(TIMER_SLOT_BARO);
        break;
    default: break;
    }
}

static void transition(state_t next) {
    LOG_INFO("[%s] %s -> %s", TAG, STATE_NAMES[g_state], STATE_NAMES[next]);
    on_exit_s(g_state);
    g_state = next;
    on_entry(next);
}

/* ---------- Lifecycle ---------------------------------------------- */
void state_machine_init(imu_t * imu, baro_t * baro) {
    ASSERT(imu != NULL && baro != NULL);
    g_imu   = imu;
    g_baro  = baro;
    g_state = S_BOOT;
    leds_off();
    LOG_INFO("[%s] init, state=%s", TAG, STATE_NAMES[g_state]);
    evq_post((event_t){ .type = E_INIT_DONE });
}

void state_machine_dispatch(event_t e) {
    switch (g_state) {

    case S_BOOT:
        if (e.type == E_INIT_DONE) {
            transition(S_INIT);
            smartclip_err_t ei = imu_init(g_imu);
            smartclip_err_t eb = baro_init(g_baro);
            if (ei != SMARTCLIP_OK || eb != SMARTCLIP_OK) {
                LOG_ERROR("[%s] sensor init failed (imu=%d baro=%d)",
                          TAG, (int)ei, (int)eb);
                transition(S_ERROR);
            } else {
                transition(S_CALIBRATING);
            }
        }
        break;

    case S_CALIBRATING:
        switch (e.type) {
        case E_CAL_BLINK:
            HAL_GPIO_TogglePin(BLUE_PORT, BLUE_PIN);
            break;
        case E_IMU_CAL_DONE: {
            uint8_t sys, gyro, acc, mag;
            bool cal_ok = false;
            if (imu_get_cal(g_imu, &sys, &gyro, &acc, &mag) == SMARTCLIP_OK) {
                LOG_INFO("[%s] cal S=%u G=%u A=%u M=%u", TAG, sys, gyro, acc, mag);
                cal_ok = (gyro == 3 && acc == 3);   /* was: sys == 3 && gyro == 3 && acc == 3 */  /* mag optional near iron */
            }
            bool min_hold_met = (HAL_GetTick() - g_cal_entry_ms) >= CAL_MIN_HOLD_MS;
            if (cal_ok && min_hold_met) {
                transition(S_STREAMING);
            }
            break;
        }
        default: break;
        }
        break;

    case S_STREAMING:
        switch (e.type) {
        case E_IMU_SAMPLE: {
            float q[4];
            if (imu_read_quat(g_imu, q) == SMARTCLIP_OK) {
                static uint16_t n = 0;
                if (++n >= 50) {          /* 100 Hz -> log ~2 Hz */
                    n = 0;
                    LOG_INFO("[%s] q = % .3f % .3f % .3f % .3f",
                             TAG, q[0], q[1], q[2], q[3]);
                }
            }
            break;
        }
        case E_BARO_SAMPLE: {
            int32_t pa; float tc;
            if (baro_read(g_baro, &pa, &tc) == SMARTCLIP_OK) {
                static uint8_t n = 0;
                if (++n >= 25) {          /* 25 Hz -> log ~1 Hz */
                    n = 0;
                    LOG_INFO("[%s] baro %ld Pa, %.1f C", TAG, (long)pa, (double)tc);
                }
            }
            break;
        }
        default: break;
        }
        break;

    case S_ERROR:
        break;   /* parked; add recovery later */

    default:
        ASSERT_UNREACHABLE("bad state");
    }
}