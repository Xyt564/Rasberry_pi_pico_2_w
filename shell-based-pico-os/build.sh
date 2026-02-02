#!/bin/bash

# Raspberry Pi Pico 2 W OS Build Script
# This script downloads dependencies and builds the OS

set -e

echo "=================================="
echo "Pico OS Build Script"
echo "=================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if PICO_SDK_PATH is set
if [ -z "$PICO_SDK_PATH" ]; then
    echo -e "${YELLOW}PICO_SDK_PATH not set. Attempting to find or clone SDK...${NC}"
    
    if [ -d "pico-sdk" ]; then
        export PICO_SDK_PATH=$(pwd)/pico-sdk
        echo -e "${GREEN}Found pico-sdk in current directory${NC}"
    elif [ -d "$HOME/pico/pico-sdk" ]; then
        export PICO_SDK_PATH=$HOME/pico/pico-sdk
        echo -e "${GREEN}Found pico-sdk in $HOME/pico/pico-sdk${NC}"
    else
        echo -e "${YELLOW}Cloning Pico SDK...${NC}"
        mkdir -p pico-sdk
        git clone https://github.com/raspberrypi/pico-sdk.git pico-sdk
        cd pico-sdk
        git submodule update --init
        cd ..
        export PICO_SDK_PATH=$(pwd)/pico-sdk
    fi
fi

echo -e "${GREEN}Using Pico SDK at: $PICO_SDK_PATH${NC}"

# Clone LittleFS if not present
if [ ! -d "littlefs" ]; then
    echo -e "${YELLOW}Cloning LittleFS library...${NC}"
    git clone https://github.com/littlefs-project/littlefs.git
    echo -e "${GREEN}LittleFS cloned successfully${NC}"
else
    echo -e "${GREEN}LittleFS already present${NC}"
fi

# Removing old build folder
echo -e "${YELLOW}Removing old build folder..${NC}"
rm -rf build # you can # this out if its ur first time running the build.sh

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Run CMake
echo -e "${YELLOW}Running CMake...${NC}"
cmake .. -DPICO_BOARD=pico2_w -DCMAKE_BUILD_TYPE=Release

# Build the project
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

# Check if build was successful
if [ -f "pico_os.uf2" ]; then
    echo -e "${GREEN}=================================="
    echo -e "Build successful!"
    echo -e "==================================${NC}"
    echo -e "${GREEN}Output file: build/pico_os.uf2${NC}"
    echo ""
    echo "Before Anything do sudo apt install screen or sudo apt install minicom which ever you wish to use"
    echo "To flash your Pico 2 W:"
    echo "1. Hold down BOOTSEL button while connecting USB"
    echo "2. Copy build/pico_os.uf2 to the RP2350 drive"
    echo "3. Connect via serial: screen /dev/ttyACM0 115200"
    echo "   or: minicom -D /dev/ttyACM0 -b 115200"
else
    echo -e "${RED}Build failed! Check errors above.${NC}"
    exit 1
fi

cd ..

# Auto creates a flash helper script
cat > flash.sh << 'EOF'
#!/bin/bash
# Quick flash script
if [ -f "build/pico_os.uf2" ]; then
    # Try to find the Pico in bootloader mode
    if [ -d "/media/$USER/RP2350" ]; then
        echo "Copying to Pico..."
        cp build/pico_os.uf2 /media/$USER/RP2350/
        echo "Flashed! Pico will reboot automatically."
    else
        echo "Pico not found in bootloader mode."
        echo "Hold BOOTSEL button and connect USB, then run this script again."
    fi
else
    echo "Build first! Run: ./build.sh"
fi
EOF

chmod +x flash.sh

echo -e "${GREEN}Helper script created: ./flash.sh${NC}"
echo -e "${YELLOW}Run ./flash.sh to quickly flash your Pico 2 W${NC}"
