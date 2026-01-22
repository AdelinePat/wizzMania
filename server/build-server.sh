#!/bin/bash

set -e

echo "========================================"
echo "  WizzMania Server Build & Run"
echo "========================================"

# Wait for database to be ready
# echo "Waiting for database..."
# until mysql -h"${DB_HOST}" -u"${DB_USER}" -p"${DB_PASSWORD}" -e "SELECT 1" > /dev/null 2>&1; do
#     echo "Database is unavailable - sleeping"
#     sleep 2
# done
# echo "✅ Database is ready!"

# echo ""
# echo "========================================"
# echo "Step 1: Running Tests"
# echo "========================================"

# # Build and run tests
# mkdir -p build-test
# cd build-test

# echo "Configuring tests..."
# cmake ../server -DBUILD_TESTS=ON

# echo "Building tests..."
# cmake --build . --target tests

# echo ""
# echo "Running tests..."
# ./tests

# TEST_RESULT=$?

# if [ $TEST_RESULT -ne 0 ]; then
#     echo ""
#     echo "❌ Tests FAILED! Aborting build."
#     exit 1
# fi

# echo ""
# echo "✅ All tests PASSED!"
# echo ""

# cd ..
# rm -rf build-test

echo "========================================"
echo "Step 2: Building Server"
echo "========================================"

mkdir -p build-server
cd build-server

echo "Configuring server..."
cmake ../server

echo "Building server..."
cmake --build . --target wizzmania-server

echo ""
echo "✅ Server build complete!"

echo ""
echo "========================================"
echo "Step 3: Running Server"
echo "========================================"

# Run the server
./wizzmania-server