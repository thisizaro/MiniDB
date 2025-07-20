#!/bin/bash
# MiniDB Build Script

set -e  # Exit on any error

echo "Building MiniDB..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# Build the project
echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build completed successfully!"
echo "Executable: ./minidb"
echo "Tests: ./tests/minidb_tests"

# Run tests
echo ""
echo "Running tests..."
if ./tests/minidb_tests; then
    echo "All tests passed!"
else
    echo "Some tests failed."
    exit 1
fi

echo ""
echo "MiniDB is ready to use!"
echo "Run './minidb' to start the interactive CLI"
