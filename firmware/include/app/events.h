/*
 * events.h - master event-type enum and event record format.
 *
 * Owner    : Andrew - smart-barbell-clip
 * Scope    : every event producer in the firmware includes this.
 * Threading: event_t is POD; safe to copy across ISR/main boundary.
 */
#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdint.h>

/* ---------- Master event-type list (X-macro) ------------------- */
#define EVENT_LIST(X)         \
    X(NONE)                   \
    X(INIT_DONE)              \
    X(IMU_CAL_DONE)           \
    X(CAL_BLINK)              \
    X(IMU_SAMPLE)             \
    X(BARO_SAMPLE)            \
    X(DISPLAY_TICK)           \
    X(ANIM_TICK)              \
    X(ANIM_INTRO_DONE)        \
    X(ALL_OK)                 \
    X(HW_FAULT)               \
    X(BLE_START)              \
    X(BLE_CONNECTED)          \
    X(BLE_DISCONNECTED)       \
    X(CCCD_WRITE)             \
    X(BUTTON_SHORT)           \
    X(BUTTON_LONG)            \
    X(NFC_FIELD_ON)           \
    X(TIMER_TICK)             \
    X(BATTERY_LOW)            \
    X(CHARGER_CONNECTED)      \
    X(ZERO_ON_BAR)            \
    X(BEGIN_CAL)              \
    X(SAVE_CAL)

#define AS_EVT_ENUM(n) E_##n,
typedef enum { EVENT_LIST(AS_EVT_ENUM) E_COUNT } event_type_t;

/* ---------- Event record (8 bytes, 4-byte aligned) ------------- */
typedef struct {
    uint8_t  type;
    uint8_t  arg8;
    uint16_t arg16;
    uint32_t arg32;
} event_t;

#endif /* APP_EVENTS_H */