#!/bin/bash

echo "===================================="
echo "Pico 2 W Webserver - Pre-Build Check"
echo "===================================="
echo ""

ERROR=0

# Check for required files
echo "Checking for required files..."
FILES=("CMakeLists.txt" "pico_webserver.cpp" "lwipopts.h" "build.sh")

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file - MISSING!"
        ERROR=1
    fi
done

echo ""

# Check PICO_SDK_PATH
if [ -z "$PICO_SDK_PATH" ]; then
    echo "✗ PICO_SDK_PATH is not set!"
    echo "  Run: export PICO_SDK_PATH=~/pico-sdk"
    ERROR=1
else
    echo "✓ PICO_SDK_PATH: $PICO_SDK_PATH"
    
    # Check if SDK exists
    if [ -d "$PICO_SDK_PATH" ]; then
        echo "✓ Pico SDK directory exists"
    else
        echo "✗ Pico SDK directory not found at $PICO_SDK_PATH"
        ERROR=1
    fi
    
    # Check for CYW43 driver
    if [ -d "$PICO_SDK_PATH/lib/cyw43-driver" ]; then
        echo "✓ CYW43 driver found"
    else
        echo "✗ CYW43 driver missing - run: cd $PICO_SDK_PATH && git submodule update --init --recursive"
        ERROR=1
    fi
    
    # Check for lwIP
    if [ -d "$PICO_SDK_PATH/lib/lwip" ]; then
        echo "✓ lwIP found"
    else
        echo "✗ lwIP missing - run: cd $PICO_SDK_PATH && git submodule update --init --recursive"
        ERROR=1
    fi
fi

echo ""

if [ $ERROR -eq 0 ]; then
    echo "===================================="
    echo "✓ All checks passed!"
    echo "===================================="
    echo ""
    echo "You can now build with:"
    echo "  ./build.sh"
    echo ""
    echo "Or manually:"
    echo "  rm -rf build/*"
    echo "  cd build"
    echo "  cmake -DPICO_BOARD=pico2_w .."
    echo "  make -j4"
    exit 0
else
    echo "===================================="
    echo "✗ Some checks failed!"
    echo "===================================="
    echo "Please fix the issues above before building."
    exit 1
fi
