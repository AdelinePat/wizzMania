#!/bin/bash

echo "========================================"
echo "  Running WizzMania Client"
echo "========================================"
echo ""

# Navigate to script's directory
cd "$(dirname "$0")"

# Clear Snap environment variables that can interfere with Qt
unset GTK_PATH
unset LD_LIBRARY_PATH

# Find and run the executable
if [ -f "build/wizzmania-client" ]; then
    echo "[OK] Starting client..."
    echo ""
    cd build
    ./wizzmania-client
elif [ -f "build/Release/wizzmania-client" ]; then
    echo "[OK] Starting client (Release)..."
    echo ""
    cd build/Release
    ./wizzmania-client
else
    echo "[ERROR] Client executable not found!"
    echo ""
    echo "Please build the client first using:"
    echo "  ./build-client.sh"
    echo ""
    exit 1
fi