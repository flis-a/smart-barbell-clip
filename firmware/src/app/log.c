/* src/app/log.c - blocking UART logger for bring-up. */
#include "app/log.h"

#include <stdarg.h>
#include <stdio.h>

#include "main.h"
#include "stm32wbxx_hal.h"

/* Adjust this if CubeMX named your VCOM UART differently. */
extern UART_HandleTypeDef huart1;       /* was hlpuart1 */
#define LOG_UART (&huart1)              /* was &hlpuart1 */

void log_init(void) {
    /* CubeMX already initialized the UART in MX_LPUART1_UART_Init(). */
}

void log_write(const char * level, const char * fmt, ...) {
    char buf[128];
    uint32_t now = HAL_GetTick();

    int n = snprintf(buf, sizeof(buf), "[%s][%lu] ", level, (unsigned long)now);

    va_list ap;
    va_start(ap, fmt);
    n += vsnprintf(buf + n, sizeof(buf) - (size_t)n - 2, fmt, ap);
    va_end(ap);

    if (n < 0) return;
    if (n > (int)sizeof(buf) - 2) n = sizeof(buf) - 2;
    buf[n++] = '\r';
    buf[n++] = '\n';

    HAL_UART_Transmit(LOG_UART, (uint8_t *)buf, (uint16_t)n, 100);
}