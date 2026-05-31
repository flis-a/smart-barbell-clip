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

void display_status(float qw, int32_t pressure_pa, float temp_c) {
    if (!g_present) return;
    char line[24];

    SH1106_Fill(SH1106_COLOR_BLACK);

    SH1106_GotoXY(0, 0);
    SH1106_Puts((char *)"SmartClip", &Font_7x10, SH1106_COLOR_WHITE);

    snprintf(line, sizeof(line), "qw %.3f", (double)qw);
    SH1106_GotoXY(0, 16);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    snprintf(line, sizeof(line), "P %ld Pa", (long)pressure_pa);
    SH1106_GotoXY(0, 30);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    snprintf(line, sizeof(line), "T %.1f C", (double)temp_c);
    SH1106_GotoXY(0, 44);
    SH1106_Puts(line, &Font_7x10, SH1106_COLOR_WHITE);

    SH1106_UpdateScreen();
}