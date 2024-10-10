#!/bin/bash

# check if bazel is installed
if ! [ -x "$(command -v bazel)" ]; then
  echo 'Error: bazel is not installed.' >&2
  exit 1
fi


cd lib 

# check if the tensorflow directory exists
if [ ! -d "tensorflow" ]; then
  echo "Tensorflow directory does not exist. Please run the git submodule command to clone the TensorFlow repository."
  exit 1
fi

#cd tensorflow
cd ..
# Build TensorFlow C++ Library (this can take a while)
# The build will create a bazel-bin directory where the compiled TensorFlow binaries are stored.
#bazel build -c opt tensorflow/lite/delegates/flex:tensorflowlite_flex

# Copy TensorFlow library and headers to root/lib
#mkdir -p ../root/lib/tensorflow
#cp bazel-bin/tensorflow/libtensorflow_cc.so ../root/lib/tensorflow/
#cp -r tensorflow ../../.temp/root/lib/tensorflow/include/

# Go back to root directory
#cd ../..

# Install TensorFlow dependencies if needed
# You might need to install any additional dependencies for TensorFlow
#sudo apt-get install -y build-essential cmake bazel

echo "TensorFlow C++ built and installed in root/lib/tensorflow"

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
if [ ! -f "CMakeCache.txt" ] || ! grep -q "BUILD_SERVER:BOOL=ON" CMakeCache.txt; then
    echo "CMakeCache.txt does not exist or configuration changed. Running CMake..."
    cmake -DTARGET_ARCH=x86_64 -DTARGET_OS=linux -DBUILD_SERVER:BOOL=ON -DBUILD_SERVER=ON -DDEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF ..
else
    echo "CMakeCache.txt found and configuration is valid. Skipping CMake reconfiguration."
fi

# Build the project
echo "Building the project..."
make -j$(nproc)

# Navigate back to the root project directory
cd ..

echo "Build process completed."
export AVAHI_COMPAT_NOWARN=y
