#!/bin/bash

# Name of the build directory
BUILD_DIR="build"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "$BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Check if the CMakeCache.txt exists and if BUILD_SERVER is correctly configured
if [ ! -f "CMakeCache.txt" ] || ! grep -q "BUILD_CLIENT:BOOL=ON" CMakeCache.txt; then
    echo "CMakeCache.txt does not exist or configuration changed. Running CMake..."
    cmake -DTARGET_ARCH=aarch64 -DTARGET_OS=linux -DBUILD_CLIENT:BOOL=ON -DBUILD_CLIENT=ON -DDEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF -DTFLITE_ENABLE_XNNPACK=ON ..
else
    echo "CMakeCache.txt found and configuration is valid. Skipping CMake reconfiguration."
fi

# Build the project
echo "Building the project..."
make install -j$(nproc)

# Navigate back to the root project directory
cd ..

if [ ! -d "/var/log/jarvis" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "/var/log/jarvis"
fi

echo "Build process completed."
export AVAHI_COMPAT_NOWARN=y
