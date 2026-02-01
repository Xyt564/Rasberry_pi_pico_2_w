#!/bin/bash

# Pico 2 W Unified System Build Script
# This script automates the build process for the Pico project

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔══════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║        Pico 2 W Unified System Build Script             ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if PICO_SDK_PATH is set
if [ -z "$PICO_SDK_PATH" ]; then
    echo -e "${RED}Error: PICO_SDK_PATH environment variable is not set!${NC}"
    echo "Please set it to your Pico SDK directory:"
    echo "  export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

echo -e "${GREEN}✓ PICO_SDK_PATH is set to: $PICO_SDK_PATH${NC}"

# Check if SDK exists
if [ ! -d "$PICO_SDK_PATH" ]; then
    echo -e "${RED}Error: Pico SDK directory not found at $PICO_SDK_PATH${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Pico SDK directory found${NC}"

# Create build directory
echo ""
echo -e "${YELLOW}Creating build directory...${NC}"
rm -rf build
mkdir -p build
cd build

# Run CMake
echo ""
echo -e "${YELLOW}Running CMake configuration...${NC}"
cmake .. -DPICO_BOARD=pico2_w

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ CMake configuration successful${NC}"

# Build the project
echo ""
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful!${NC}"

# Display build artifacts
echo ""
echo -e "${BLUE}╔══════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║                  BUILD COMPLETE                          ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════╝${NC}"
echo ""
echo "Build artifacts:"
echo "  UF2 file:  build/pico_unified_system.uf2"
echo "  ELF file:  build/pico_unified_system.elf"
echo "  BIN file:  build/pico_unified_system.bin"
echo ""
echo -e "${YELLOW}To flash to your Pico 2 W:${NC}"
echo "  1. Hold BOOTSEL button while connecting USB"
echo "  2. Copy build/pico_unified_system.uf2 to the Pico drive"
echo "  3. Wait for Pico to reboot"
echo ""
echo -e "${YELLOW}After flashing:${NC}"
echo "  1. Open serial monitor (115200 baud) to see boot messages"
echo "  2. Wait for WiFi connection"
echo "  3. Note the IP address displayed"
echo "  4. Visit http://<ip_address>/ for portfolio"
echo "  5. Visit http://<ip_address>/terminal for web terminal"
echo ""
echo -e "${GREEN}Build completed successfully!${NC}"
