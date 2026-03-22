#!/bin/bash

set -e

# ==================== COLORS ====================
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m'

title() {
  echo -e "${BLUE}========================================${RESET}"
  echo -e "${BLUE}  $1${RESET}"
  echo -e "${BLUE}========================================${RESET}"
}

success() {
  echo -e "${GREEN}$1${RESET}"
}

failure() {
  echo -e "${RED}$1${RESET}"
}

# ==================== BUILD ====================

title "WizzMania Server Build & Run"

# Wait for database to be ready
# echo "Waiting for database..."
# until mysql -h"${DB_HOST}" -u"${DB_USER}" -p"${DB_PASSWORD}" -e "SELECT 1" > /dev/null 2>&1; do
#     echo "Database is unavailable - sleeping"
#     sleep 2
# done
# success "✅ Database is ready!"

echo ""
title "Step 1: Running Tests"

mkdir -p build-test
cd build-test

echo "Configuring tests..."
cmake ../server -DBUILD_TESTS=ON

echo "Building tests..."
cmake --build . --target tests

echo ""
echo "Running tests..."
./tests

TEST_RESULT=$?

if [ $TEST_RESULT -ne 0 ]; then
    echo ""
    failure "❌ Tests FAILED! Aborting build."
    exit 1
fi

echo ""
success "✅ All tests PASSED!"
echo ""

cd ..
rm -rf build-test

title "Step 2: Building Server"

mkdir -p build-server
cd build-server

echo "Configuring server..."
cmake ../server

echo "Building server..."
cmake --build . --target wizzmania-server -j$(nproc)

echo ""
success "✅ Server build complete!"

echo ""
title "Step 3: Running Server"

./wizzmania-server