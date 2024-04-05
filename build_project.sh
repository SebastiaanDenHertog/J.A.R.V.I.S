#!/bin/bash

# Directory where TensorFlow Lite should be cloned
TENSORFLOW_DIR="lib/tensorflow"
TENSORFLOW_LITE_REPO="https://github.com/tensorflow/tensorflow.git"
TENSORFLOW_LITE_BRANCH="master" # Adjust this as needed

# Name of the build directory
BUILD_DIR="build"

# Check if TensorFlow Lite directory exists, if not, clone it
if [ ! -d "$TENSORFLOW_DIR" ]; then
    echo "TensorFlow Lite directory does not exist. Cloning it..."
    git clone --branch $TENSORFLOW_LITE_BRANCH --depth 1 $TENSORFLOW_LITE_REPO $TENSORFLOW_DIR
else
    echo "TensorFlow Lite directory exists. Skipping cloning..."
fi

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
cmake ..

# Build the project
echo "Building the project..."
make

# Navigate back to the root project directory
cd ..

echo "Build process completed."
