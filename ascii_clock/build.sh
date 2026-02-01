#!/bin/bash

# Build script for Pico ASCII Clock
# Make sure PICO_SDK_PATH is set before running this script

set -e

echo "==================================="
echo "Pico ASCII Clock Build Script"
echo "==================================="
echo ""

# Check if PICO_SDK_PATH is set
if [ -z "$PICO_SDK_PATH" ]; then
    echo "ERROR: PICO_SDK_PATH is not set!"
    echo "Please set it with: export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

echo "Using Pico SDK: $PICO_SDK_PATH"
echo ""

# Ask which version to build
echo "Which version do you want to build?"
echo "1) USB Serial version (default)"
echo "2) UART Serial version (GPIO 0/1)"
read -p "Enter choice [1-2]: " choice

# Create and enter build directory
# rm -rf build (u can remove the hastage if u want)
mkdir -p build
cd build

if [ "$choice" == "2" ]; then
    echo ""
    echo "Building UART version..."
    echo "Running CMake with UART option..."
    cmake .. -DUSE_UART=ON
    VERSION="UART"
else
    echo ""
    echo "Building USB version..."
    echo "Running CMake..."
    cmake ..
    VERSION="USB"
fi

echo ""
echo "Compiling..."
make -j4

echo ""
echo "==================================="
echo "Build Complete!"
echo "==================================="
echo ""

if [ -f "ascii_clock.uf2" ]; then
    echo "Output file: build/ascii_clock.uf2"
    echo "Version: $VERSION"
    echo ""
    echo "To flash:"
    echo "1. Hold BOOTSEL button on Pico"
    echo "2. Connect to USB"
    echo "3. Copy ascii_clock.uf2 to Pico"
    echo ""
    
    if [ "$VERSION" == "UART" ]; then
        echo "Hardware connections:"
        echo "- GPIO 0 (TX) → RX on serial adapter"
        echo "- GPIO 1 (RX) → TX on serial adapter"
        echo "- GND → GND"
        echo "Baud rate: 115200"
    else
        echo "View output:"
        echo "- Linux: screen /dev/ttyACM0 115200"
        echo "- macOS: screen /dev/tty.usbmodem* 115200"
        echo "- Windows: PuTTY on COM port"
    fi
fi

echo ""
