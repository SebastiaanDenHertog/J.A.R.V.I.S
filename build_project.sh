#!/bin/bash

TENSORFLOW_LITE_BRANCH="master" # Adjust this as needed

# Name of the build directory
BUILD_DIR="build"


# Check if the build directory exists
if [ -d "$BUILD_DIR" ]; then
    echo "Build directory exists. Clearing it..."
    rm -rf "$BUILD_DIR"/*
else
    echo "Build directory does not exist. Creating it..."
    mkdir "$BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake to configure the project and generate the build system
echo "Running CMake..."
cmake -DTARGET_ARCH=aarch64 -DTARGET_OS=linux -DBUILD_COMPONENT=full -DDEBUG_MODE=ON ..

# Build the project
echo "Building the project..."
make -j$(nproc)

# Navigate back to the root project directory
cd ..

echo "Build process completed."
