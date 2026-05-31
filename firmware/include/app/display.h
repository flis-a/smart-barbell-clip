#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BLE_DISCONNECTED = 0,
    BLE_CONNECTED,
} ble_state_t;

bool display_init   (void);
void display_splash (void);
void display_status (const float q[4], int32_t pressure_pa, float temp_c,
                     ble_state_t ble, int8_t rssi_dbm);

#endif