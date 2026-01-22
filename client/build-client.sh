#!/bin/bash

set -e

echo "========================================"
echo "  WizzMania Client Build"
echo "========================================"

echo ""
echo "========================================"
echo "Step 1: Running Tests"
echo "========================================"

# Build and run tests (Linux native for speed)
mkdir -p build-test
cd build-test

echo "Configuring tests..."
cmake ../client -DBUILD_TESTS=ON

echo "Building tests..."
cmake --build . --target tests

echo ""
echo "Running tests..."
./tests

TEST_RESULT=$?

if [ $TEST_RESULT -ne 0 ]; then
    echo ""
    echo "❌ Tests FAILED! Aborting build."
    exit 1
fi

echo ""
echo "✅ All tests PASSED!"
echo ""

cd ..
rm -rf build-test

echo "========================================"
echo "Step 2: Cross-Compiling for Windows"
echo "========================================"

mkdir -p build-client
cd build-client

echo "Configuring client for Windows..."
cmake ../client \
    -DCMAKE_TOOLCHAIN_FILE=/toolchain-windows.cmake \
    -DCMAKE_BUILD_TYPE=Release

echo "Building client..."
cmake --build . --target wizzmania-client

echo ""
echo "========================================"
echo "Step 3: Copying Files to dist-windows/"
echo "========================================"

mkdir -p /app/dist-windows
cp wizzmania-client.exe /app/dist-windows/

# Copy necessary Qt DLLs for Windows
QT_WINDOWS_PATH="/opt/qt-windows/6.8.1/mingw_64"
cp ${QT_WINDOWS_PATH}/bin/Qt6Core.dll /app/dist-windows/
cp ${QT_WINDOWS_PATH}/bin/Qt6Gui.dll /app/dist-windows/
cp ${QT_WINDOWS_PATH}/bin/Qt6Widgets.dll /app/dist-windows/
cp ${QT_WINDOWS_PATH}/bin/Qt6Network.dll /app/dist-windows/

# Copy MinGW runtime DLLs
cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll /app/dist-windows/ 2>/dev/null || true
cp /usr/lib/gcc/x86_64-w64-mingw32/10-posix/libgcc_s_seh-1.dll /app/dist-windows/ 2>/dev/null || true
cp /usr/lib/gcc/x86_64-w64-mingw32/10-posix/libstdc++-6.dll /app/dist-windows/ 2>/dev/null || true

# Copy Qt plugins
mkdir -p /app/dist-windows/platforms
cp ${QT_WINDOWS_PATH}/plugins/platforms/qwindows.dll /app/dist-windows/platforms/

echo ""
echo "✅ Windows client build complete!"
echo ""
echo "========================================"
echo "Distribution files in: dist-windows/"
echo "========================================"
ls -lh /app/dist-windows/