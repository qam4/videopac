#!/bin/bash
# Build O2EM on Amazon Linux (without package manager Allegro)

set -e  # Exit on error

echo "=== Building O2EM on Amazon Linux ==="
echo ""

# Check if we're in the o2em directory
if [ ! -f "main.c" ]; then
    echo "Error: Please run this script from the doc/o2em directory"
    echo "Usage: cd doc/o2em && bash ../../build_o2em_amazon_linux.sh"
    exit 1
fi

O2EM_DIR=$(pwd)
echo "O2EM directory: $O2EM_DIR"
echo ""

# Create build directory
mkdir -p /tmp/o2em_build
cd /tmp/o2em_build

echo "Step 1: Installing basic dependencies..."
sudo yum install -y gcc make cmake wget unzip libX11-devel libXext-devel libXcursor-devel libXpm-devel

echo ""
echo "Step 2: Downloading Allegro 4.4.3.1 (last stable 4.x version)..."
if [ ! -f "allegro-4.4.3.1.tar.gz" ]; then
    wget https://github.com/liballeg/allegro5/releases/download/4.4.3.1/allegro-4.4.3.1.tar.gz
fi

echo ""
echo "Step 3: Extracting Allegro..."
tar -xzf allegro-4.4.3.1.tar.gz
cd allegro-4.4.3.1

echo ""
echo "Step 4: Building Allegro (this may take a few minutes)..."
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)

echo ""
echo "Step 5: Installing Allegro..."
sudo make install
sudo ldconfig

echo ""
echo "Step 6: Building O2EM..."
cd "$O2EM_DIR"
make clean || true
make

echo ""
echo "=== Build Complete! ==="
echo ""
echo "To test Killer Bees:"
echo "  # Copy ROM and BIOS (run from o2em directory)"
echo "  cp ~/src/roms/Killer\ Bees\ \(1983\)\(Philips\)\(US\).bin ./"
echo "  cp ~/src/roms/BIOS/bios_O2rom.bin ./o2rom.bin"
echo "  ./o2em 'Killer Bees (1983)(Philips)(US).bin'"
echo ""
echo "Or with debugger:"
echo "  ./o2em -debug 'Killer Bees (1983)(Philips)(US).bin'"
echo ""
