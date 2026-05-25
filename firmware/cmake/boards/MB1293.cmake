# firmware/cmake/boards/MB1293.cmake
# STM32WB55 USB Dongle (MB1293-WB55CGU-C02)
#
# MCU: STM32WB55CGU6 - Cortex-M4F (with FPU) + Cortex-M0+ radio coprocessor
#      1 MB flash, 256 KB SRAM, 48-pin UFQFPN
# LEDs: PB0 (red, LED2), PB1 (green, LED3), PB5 (blue, LED1)
# Button: PA10 (SW1)
 
set(BOARD_NAME           "MB1293")
set(BOARD_MCU_FAMILY     "STM32WBxx")
set(BOARD_MCU_DEFINE     "STM32WB55xx")  # picks the right register map in stm32wbxx.h
 
# Cortex-M4 with single-precision FPU, hard-float ABI
set(BOARD_CPU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)
 
# Preprocessor defines visible to all source files
set(BOARD_DEFINES
    ${BOARD_MCU_DEFINE}
    BOARD_MB1293
    HSE_VALUE=32000000   # external 32 MHz crystal on the dongle
)
 
# Linker script + startup file for this MCU
set(BOARD_LINKER_SCRIPT  ${CMAKE_SOURCE_DIR}/linker/stm32wb55xx_flash_cm4.ld)
set(BOARD_STARTUP_SOURCE ${CMAKE_SOURCE_DIR}/startup/startup_stm32wb55xx_cm4.s)
 
# Blink target pin (used by main.c)
set(BOARD_LED_GPIO_PORT  B)  # GPIOB
set(BOARD_LED_PIN        1)  # PB1 = green LED3
