#!/bin/bash

TENSORFLOW_LITE_BRANCH="master" # Adjust this as needed

# Name of the build directory
BUILD_DIR="build"

# install boost
#sudo apt install libboost-all-dev libssl-dev libplist-dev libavahi-compat-libdnssd-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "$BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Run CMake to configure the project and generate the build system
echo "Running CMake..."
# Check if CMakeFile is updated in the last hour
if [[ $(find ../CMakeLists.txt -mmin -5) || ! -f "main" ]]; then
    echo "CMakeLists.txt has been updated in the last hour. Running CMake..."
    cmake -UBUILD_FULL -UBUILD_SERVER -UBUILD_CLIENT -DTARGET_ARCH=aarch64 -DTARGET_OS=linux -DBUILD_CLIENT=ON -DDEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ..
else
    echo "CMakeLists.txt has not been updated in the last hour. Skipping CMake..."
fi

# Build the project
echo "Building the project..."
make -j$(nproc) 

# Navigate back to the root project directory
cd ..

echo "Build process completed."
export AVAHI_COMPAT_NOWARN=y