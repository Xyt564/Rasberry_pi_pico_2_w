cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# MUST set board before project()
if(NOT PICO_BOARD)
    set(PICO_BOARD pico2_w)
    message("Defaulting to board type: ${PICO_BOARD}")
    message("Note: Use pico_w for original Pico W (RP2040)")
endif()

project(pico_webserver C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Add project directory to include path for lwipopts.h
include_directories(${CMAKE_CURRENT_LIST_DIR})

# Initialize the Pico SDK
pico_sdk_init()

# Check if we have wireless support
if (NOT TARGET pico_cyw43_arch_lwip_poll)
    message(FATAL_ERROR "
    ========================================
    ERROR: Wireless libraries not found!
    
    Your Pico SDK is missing the CYW43 driver.
    Please run these commands:
    
    cd $ENV{PICO_SDK_PATH}
    git submodule update --init --recursive
    
    Then rebuild this project.
    ========================================
    ")
endif()

# Add executable
add_executable(pico_webserver
    pico_webserver.cpp
)

# Add the standard include files to the build
# THIS IS CRITICAL - tells the compiler to look in the project directory for lwipopts.h
target_include_directories(pico_webserver PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Pull in pico libraries that we need
target_link_libraries(pico_webserver
    pico_stdlib
    pico_cyw43_arch_lwip_poll
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(pico_webserver 1)
pico_enable_stdio_uart(pico_webserver 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico_webserver)
