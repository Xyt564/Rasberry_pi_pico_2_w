#!/bin/bash

# Build script for Pico 2 W Scanner

# Set SDK path if not already set
if [ -z "$PICO_SDK_PATH" ]; then
    export PICO_SDK_PATH="$HOME/pico-sdk"
    echo "Setting PICO_SDK_PATH to $PICO_SDK_PATH"
fi

# Verify SDK exists
if [ ! -d "$PICO_SDK_PATH" ]; then
    echo "ERROR: Pico SDK not found at $PICO_SDK_PATH"
    echo "Please set PICO_SDK_PATH or install SDK to ~/pico-sdk"
    exit 1
fi

echo "Using Pico SDK: $PICO_SDK_PATH"

# Check if SDK is initialized
if [ ! -f "$PICO_SDK_PATH/lib/cyw43-driver/src/cyw43.h" ]; then
    echo "WARNING: CYW43 driver not found. Initializing submodules..."
    cd "$PICO_SDK_PATH"
    git submodule update --init --recursive
    cd -
fi

# Clean and rebuild
echo "Cleaning build directory..."
rm -rf build
mkdir -p build
cd build

echo "Running CMake..."
cmake -DPICO_BOARD=pico2_w ..

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo "Building..."
make -j4

if [ $? -eq 0 ]; then
    echo ""
    echo "==================================="
    echo "Build successful!"
    echo "Flash file: build/pico_scanner.uf2"
    echo "==================================="
else
    echo "ERROR: Build failed"
    exit 1
fi
