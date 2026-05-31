# firmware/CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
 
# ---- 1. Toolchain MUST be set before project() ----
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/arm-none-eabi-toolchain.cmake"
    CACHE FILEPATH "")
 
# ---- 2. Pick a board (override with -DBOARD=...) ----
if(NOT DEFINED BOARD)
    set(BOARD MB1293 CACHE STRING "Target board")
endif()
include("${CMAKE_SOURCE_DIR}/cmake/boards/${BOARD}.cmake")
 
project(smart-barbell-clip-fw LANGUAGES C ASM)
 
set(CMAKE_C_STANDARD          11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd
 
# ---- 3. Common flags from the board file ----
add_compile_options(
    ${BOARD_CPU_FLAGS}
    -Wall -Wextra -Wpedantic
    -ffunction-sections -fdata-sections
    $<$<CONFIG:Debug>:-O0>    # Updated from -Og
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-g>
)
add_compile_definitions(${BOARD_DEFINES})
 
add_link_options(
    ${BOARD_CPU_FLAGS}
    -T${BOARD_LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map,--cref
    --specs=nano.specs
    --specs=nosys.specs
    -u _printf_float          # <-- ADD: pull in nano's float printf
)
 
# ---- 4. The executable ----
# ---- Sources ----
# HAL driver sources live in cube-wb; we filter out the *_template files
# that CubeMX-style trees include but which would cause multiple-definition
# errors if linked (they ship as weak overrides of MspInit, etc.)
file(GLOB HAL_SOURCES_ALL
    third_party/cube-wb/Drivers/STM32WBxx_HAL_Driver/Src/*.c
)
set(HAL_SOURCES "")
foreach(src ${HAL_SOURCES_ALL})
    if(NOT src MATCHES ".*_template.c$")
        list(APPEND HAL_SOURCES ${src})
    endif()
endforeach()
 
file(GLOB_RECURSE CORE_SOURCES
    Core/Src/*.c
)
 
file(GLOB_RECURSE WPAN_SOURCES
    STM32_WPAN/App/*.c
    STM32_WPAN/Target/*.c
)
 
# Your hand-written modules (these directories may be empty at first)
file(GLOB_RECURSE APP_SOURCES
    src/app/*.c
    src/drivers/*.c
    src/ble/*.c
    src/bsp/*.c
)

#SH1106 OLED display driver sources
file(GLOB SH1106_SOURCES third_party/sh1106/*.c)

# Transport layer + SHCI - CPU1<->CPU2 communication over IPCC.
# NOTE: the tl/ folder also contains Zigbee/Thread/802.15.4 variants.
# List ONLY the BLE-relevant files; do not glob.
set(WPAN_TL_SOURCES
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/hci_tl.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/hci_tl_if.c     # NEW
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.c    # NEW
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci/shci.c
)

# Small WPAN utilities. NOTE: utilities/ also contains heap_4.c (FreeRTOS-only),
# fmm.c (file mgmt), memory_manager.c (older API). Don't glob; list explicitly.
set(WPAN_UTIL_SOURCES
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/utilities/otp.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/utilities/stm_list.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/utilities/stm_queue.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/utilities/dbg_trace.c
)

# Sequencer + low-power manager from cube-wb Utilities/. These directories are
# small and BLE-safe to glob - but explicit is still clearer.
set(STM32_UTIL_SOURCES
    third_party/cube-wb/Utilities/lpm/tiny_lpm/stm32_lpm.c
    third_party/cube-wb/Utilities/sequencer/stm32_seq.c
)

# WPAN BLE host stack: GAP/GATT/HAL/HCI/L2CAP command bindings (auto-generated
# from ST's IDL), OSAL glue, and the service controller framework.
set(WPAN_BLE_CORE_SOURCES
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_events.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_gap_aci.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_gatt_aci.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_gen_aci.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_hal_aci.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_hci_le.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto/ble_l2cap_aci.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/template/osal.c
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/svc/Src/svc_ctl.c
)
 
set(BNO055_SENSORAPI_SOURCES
    third_party/bno055-sensorapi/bno055.c
)

set(BMP280_EBREZA_SOURCES 
    third_party/bmp280-ebreza/src/bmp280.c
)

add_executable(${PROJECT_NAME}.elf
    ${HAL_SOURCES}
    ${CORE_SOURCES}
    ${WPAN_SOURCES}
    ${WPAN_BLE_CORE_SOURCES}
    ${WPAN_TL_SOURCES}
    ${WPAN_UTIL_SOURCES}
    ${STM32_UTIL_SOURCES}
    ${BNO055_SENSORAPI_SOURCES} 
    ${BMP280_EBREZA_SOURCES} 
    ${SH1106_SOURCES}
    ${APP_SOURCES}
    ${BOARD_STARTUP_SOURCE}
    include/app/display.h
)
 
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    Core/Inc
    STM32_WPAN/App
    STM32_WPAN/Target
    include
    third_party/cube-wb/Drivers/CMSIS/Include
    third_party/cube-wb/Drivers/CMSIS/Device/ST/STM32WBxx/Include
    third_party/cube-wb/Drivers/STM32WBxx_HAL_Driver/Inc
    third_party/cube-wb/Drivers/STM32WBxx_HAL_Driver/Inc/Legacy
    third_party/cube-wb/Middlewares/ST/STM32_WPAN
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/auto
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/core/template
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/utilities
    third_party/cube-wb/Utilities/lpm/tiny_lpm
    third_party/cube-wb/Utilities/sequencer

    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/svc/Inc
    third_party/cube-wb/Middlewares/ST/STM32_WPAN/ble/svc/Src

    third_party/bno055-sensorapi
    third_party/bmp280-ebreza/inc
    third_party/sh1106  
)
 
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    USE_HAL_DRIVER
    ${BOARD_DEFINES}
)
 
# ---- 5. Post-build: print size, generate .bin and .hex ----
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE}    $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Producing .hex and .bin and printing image size"
)
