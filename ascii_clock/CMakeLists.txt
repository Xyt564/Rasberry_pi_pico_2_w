cmake_minimum_required(VERSION 3.13)

# Set board type for Pico 2 W before including SDK
set(PICO_BOARD pico2_w)

# Include the Pico SDK
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
else()
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

project(ascii_clock C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

option(USE_UART "Build with UART serial instead of USB" OFF)

add_executable(ascii_clock
    ascii_clock.cpp
)

# make it look in the active directory for cmake and other files in the folder
target_include_directories(ascii_clock PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(ascii_clock
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
)

if(USE_UART)
    message(STATUS "Building UART version (GPIO 0/1)")
    target_compile_definitions(ascii_clock PRIVATE USE_UART)
    target_link_libraries(ascii_clock hardware_uart)
    pico_enable_stdio_uart(ascii_clock 1)
    pico_enable_stdio_usb(ascii_clock 0)
else()
    message(STATUS "Building USB version")
    pico_enable_stdio_usb(ascii_clock 1)
    pico_enable_stdio_uart(ascii_clock 0)
endif()

pico_add_extra_outputs(ascii_clock)
