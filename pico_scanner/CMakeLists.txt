cmake_minimum_required(VERSION 3.13)

# Set PICO_SDK_PATH if not already set
if (NOT DEFINED ENV{PICO_SDK_PATH})
    set(ENV{PICO_SDK_PATH} "$ENV{HOME}/pico-sdk")
    message("PICO_SDK_PATH not set, using default: $ENV{PICO_SDK_PATH}")
endif()

# Set board type for Pico 2 W (MUST be before pico_sdk_import!)
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_scanner CXX C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(pico_scanner
    scanner.cpp
)

# Add lwIP include path
target_include_directories(pico_scanner PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Pull in common dependencies
target_link_libraries(pico_scanner
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_gpio
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(pico_scanner 1)
pico_enable_stdio_uart(pico_scanner 0)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico_scanner)
