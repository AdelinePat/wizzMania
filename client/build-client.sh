#!/bin/bash

set -e  # Exit on error

echo "========================================"
echo "  WizzMania Client Build Script (Linux)"
echo "========================================"
echo ""

# Navigate to script's directory (client/)
cd "$(dirname "$0")"

# Check for required tools
echo "Checking dependencies..."
echo ""

if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake not found!"
    echo ""
    echo "Install with:"
    echo "  sudo apt install cmake"
    exit 1
fi

if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo "[ERROR] Qt6 not found!"
    echo ""
    echo "Install with:"
    echo "  sudo apt install qt6-base-dev qt6-tools-dev"
    exit 1
fi

echo "[OK] CMake found: $(cmake --version | head -n1)"

if command -v qmake6 &> /dev/null; then
    echo "[OK] Qt6 found: $(qmake6 --version | grep 'Qt version')"
elif command -v qmake &> /dev/null; then
    echo "[OK] Qt found: $(qmake --version | grep 'Qt version')"
fi

echo ""

# Verify CMakeLists.txt exists
if [ ! -f "CMakeLists.txt" ]; then
    echo "[ERROR] CMakeLists.txt not found!"
    echo "This script must be in the client/ directory."
    exit 1
fi

echo "[INFO] Building from: $(pwd)"
echo ""

# Clean build directory if it exists
if [ -d "build" ]; then
    echo "[INFO] Cleaning old build directory..."
    rm -rf build
fi

# Create fresh build directory
mkdir build
cd build

echo "========================================"
echo "Step 1: Configuring with CMake"
echo "========================================"
echo ""

# Configure with CMake
# Let CMake auto-detect everything on Linux
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo ""
    echo "[ERROR] CMake configuration failed!"
    echo ""
    echo "Troubleshooting:"
    echo "1. Make sure Qt6 is installed: sudo apt install qt6-base-dev"
    echo "2. Check that all dependencies are available"
    echo "3. Try: rm -rf build && ./build-client.sh"
    exit 1
fi

echo ""
echo "[OK] Configuration successful"
echo ""

echo "========================================"
echo "Step 2: Building Client"
echo "========================================"
echo ""

# Build with all CPU cores
cmake --build . --config Release -j$(nproc)

if [ $? -ne 0 ]; then
    echo ""
    echo "[FAILED] Build failed!"
    echo "Check the errors above"
    exit 1
fi

echo ""
echo "========================================"
echo "  Build Successful!"
echo "========================================"
echo ""

# Find the executable
if [ -f "wizzmania-client" ]; then
    echo "Executable: $(pwd)/wizzmania-client"
    echo ""
    echo "To run:"
    echo "  cd build"
    echo "  ./wizzmania-client"
elif [ -f "Release/wizzmania-client" ]; then
    echo "Executable: $(pwd)/Release/wizzmania-client"
    echo ""
    echo "To run:"
    echo "  cd build/Release"
    echo "  ./wizzmania-client"
else
    echo "Executable built - check build/ directory"
    ls -lh wizzmania-client* 2>/dev/null || true
fi

echo ""
echo "Or run with: ./run-client.sh"
echo ""