cmake_minimum_required(VERSION 3.13)

# Include Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_os C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add current directory to include path for lwipopts.h
include_directories(${CMAKE_CURRENT_LIST_DIR})

# LittleFS library
add_library(littlefs INTERFACE)
target_sources(littlefs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs.c
    ${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs_util.c
)
target_include_directories(littlefs INTERFACE ${CMAKE_CURRENT_LIST_DIR}/littlefs)

# Main executable
add_executable(pico_os
    pico_os.cpp
)

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(pico_os
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_sntp
    pico_multicore
    hardware_flash
    hardware_watchdog
    littlefs
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(pico_os 1)
pico_enable_stdio_uart(pico_os 0)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico_os)

# Compiler options
target_compile_options(pico_os PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# Set stack and heap sizes (reduced to fit in SCRATCH memory)
target_compile_definitions(pico_os PRIVATE
    PICO_STACK_SIZE=0x1000
    PICO_HEAP_SIZE=0x4000
    PICO_CORE1_STACK_SIZE=0x0
)
