#!/bin/bash

# Build script for Pico W webserver
# This properly sets the board type for CMake

set -e  # Exit on error

echo "================================"
echo "Pico W Webserver Build Script"
echo "================================"
echo ""

# Check for PICO_SDK_PATH
if [ -z "$PICO_SDK_PATH" ]; then
    echo "ERROR: PICO_SDK_PATH is not set!"
    echo "Please run: export PICO_SDK_PATH=~/pico-sdk"
    exit 1
fi

echo "Using SDK at: $PICO_SDK_PATH"
echo ""

# Create and enter build directory
if [ -d "build" ]; then
    echo "Cleaning old build directory..."
    rm -rf build/*
else
    mkdir build
fi

cd build

echo "Configuring CMake for Pico 2 W..."
# The key is passing PICO_BOARD as a CMake variable
# Use pico2_w for Pico 2 W (RP2350), or pico_w for original Pico W (RP2040)
cmake -DPICO_BOARD=pico2_w ..

echo ""
echo "Building project..."
make -j4

echo ""
echo "================================"
echo "Build complete!"
echo "================================"
echo ""

if [ -f "pico_webserver.uf2" ]; then
    echo "✓ Success! Your firmware is ready:"
    echo "  File: $(pwd)/pico_webserver.uf2"
    echo ""
    echo "To flash to your Pico W:"
    echo "  1. Hold BOOTSEL button on Pico"
    echo "  2. Connect USB cable"
    echo "  3. Release BOOTSEL"
    echo "  4. Copy pico_webserver.uf2 to the Pico drive"
else
    echo "✗ Build failed - no UF2 file generated"
    exit 1
fi
