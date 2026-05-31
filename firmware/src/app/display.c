/* display.c - thin wrapper over the SH1106 driver. Prototype-only; the
 * product has no display, so the rest of the firmware only touches display_*. */
#include "app/display.h"
#include "app/log.h"
#include "app/bitmap.h"

#include "SH1106.h"
#include "fonts.h"

#include <stdbool.h>
#include <stdio.h>

#define TAG "oled"

static bool g_present = false;

bool display_init(void) {
    g_present = (SH1106_Init() != 0);
    if (g_present) LOG_INFO("[%s] init OK", TAG);
    else           LOG_WARN("[%s] not detected", TAG);
    return g_present;
}

void display_splash(void) {
    if (!g_present) return;
    SH1106_Fill(SH1106_COLOR_BLACK);
    SH1106_DrawBitmap(0, 0, PETERpg, 128, 64, 1);
    SH1106_UpdateScreen();
}

void display_status(const float q[4], int32_t pressure_pa, float temp_c,
                    ble_state_t ble, int8_t rssi_dbm) {
    if (!g_present) return;
    char line[26];

    SH1106_Fill(SH1106_COLOR_BLACK);

    /* All four quaternion components, two per line. */
    snprintf(line, sizeof(line), "w%+.2f x%+.2f", (double)q[0], (double)q[1]);
    SH1106_GotoXY(0, 0);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    snprintf(line, sizeof(line), "y%+.2f z%+.2f", (double)q[2], (double)q[3]);
    SH1106_GotoXY(0, 12);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    /* Barometer on one line: pressure + temperature. */
    snprintf(line, sizeof(line), "%ldPa %.1fC", (long)pressure_pa, (double)temp_c);
    SH1106_GotoXY(0, 28);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    /* BLE status + strength. */
    if (ble == BLE_CONNECTED) {
        snprintf(line, sizeof(line), "BLE conn %ddBm", (int)rssi_dbm);
    } else {
        snprintf(line, sizeof(line), "BLE disconnected");
    }
    SH1106_GotoXY(0, 44);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_UpdateScreen();
}