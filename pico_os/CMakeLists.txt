cmake_minimum_required(VERSION 3.13)

# Include Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_unified_system C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Specify board type for Pico 2 W
set(PICO_BOARD pico2_w)

# Initialize the SDK
pico_sdk_init()

# Create the executable
add_executable(pico_unified_system
    main.cpp
)

# Include directories
target_include_directories(pico_unified_system PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link libraries
target_link_libraries(pico_unified_system
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_printf
    pico_malloc
    hardware_adc
    hardware_watchdog
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(pico_unified_system 1)
pico_enable_stdio_uart(pico_unified_system 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(pico_unified_system)

# Compile options for optimization and Pico 2 W
target_compile_options(pico_unified_system PRIVATE
    -Wall
    -Wno-format
    -Wno-unused-function
    -Os                          # Optimize for size
    -ffunction-sections          # Place each function in its own section
    -fdata-sections             # Place each data item in its own section
)

# Linker options for size optimization
target_link_options(pico_unified_system PRIVATE
    -Wl,--gc-sections           # Remove unused sections
    -Wl,--print-memory-usage    # Print memory usage after linking
)

# Configure stack and heap sizes (optimized for Pico 2 W constraints)
# SCRATCH_Y is only 4KB, so stack must be smaller
target_compile_definitions(pico_unified_system PRIVATE
    PICO_STACK_SIZE=0x1000      # 4KB stack (fits in SCRATCH_Y)
    PICO_HEAP_SIZE=0x10000      # 64KB heap (plenty of RAM available)
    PICO_CORE1_STACK_SIZE=0x400 # 1KB for core 1
)

# WiFi configuration (optional - can be set in code)
# target_compile_definitions(pico_unified_system PRIVATE
#     WIFI_SSID=\"YourSSID\"
#     WIFI_PASSWORD=\"YourPassword\"
#     CYW43_HOST_NAME=\"pico-server\"
# )

# Enable Link Time Optimization for smaller binary size
# NOTE: Disabled due to conflicts with pico_printf wrapper functions
# set_target_properties(pico_unified_system PROPERTIES
#     INTERPROCEDURAL_OPTIMIZATION TRUE
# )

# Additional Pico 2 optimizations
if(PICO_PLATFORM STREQUAL "rp2350")
    message(STATUS "Building for RP2350 (Pico 2)")
    target_compile_options(pico_unified_system PRIVATE
        -mcpu=cortex-m33          # Pico 2 uses Cortex-M33
        -mthumb                   # Use Thumb instruction set
    )
endif()

# Print configuration summary
message(STATUS "==================================")
message(STATUS "Pico 2 W Unified System Build Info")
message(STATUS "==================================")
message(STATUS "Board: ${PICO_BOARD}")
message(STATUS "Platform: ${PICO_PLATFORM}")
message(STATUS "SDK Version: ${PICO_SDK_VERSION_STRING}")
message(STATUS "Optimization: -Os (size)")
message(STATUS "LTO: Disabled (conflicts with printf)")
message(STATUS "==================================")
