#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

bool display_init   (void);                /* false if OLED not on the bus */
void display_splash (void);                /* draw the bitmap splash       */
void display_status (float qw, int32_t pressure_pa, float temp_c);

#endif