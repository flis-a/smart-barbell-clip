/* include/app/log.h - lightweight UART logger.
 *
 * Usage: LOG_INFO("BNO055 id 0x%02X", id);
 * Output: [I][12345] BNO055 id 0xA0
 *         (level)(ms-since-boot) message
 */
#ifndef APP_LOG_H
#define APP_LOG_H

#include <stdarg.h>

void log_init (void);
void log_write(const char * level, const char * fmt, ...);

#define LOG_INFO(fmt, ...)  log_write("I", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_write("W", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_write("E", fmt, ##__VA_ARGS__)

#endif