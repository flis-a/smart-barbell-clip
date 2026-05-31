/* src/app/assert.c - bring-up version. */
#include "app/assert.h"
#include "app/log.h"

#include "stm32wbxx.h"

void assert_failed(const char * file, int line, const char * expr) {
    LOG_ERROR("ASSERT %s:%d: %s", file, line, expr);

    __disable_irq();

    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
        __BKPT(0);
    }

    for (;;) { }
}