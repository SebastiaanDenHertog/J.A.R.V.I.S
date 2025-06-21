#!/bin/bash

# Name of the build directory
BUILD_DIR="build"
TFLITE_LIB_DIR="$BUILD_DIR/tflite_flex_lib"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "$BUILD_DIR"
fi

if [ ! -d "$TFLITE_LIB_DIR" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "$TFLITE_LIB_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR" || { echo "Failed to change directory to $BUILD_DIR"; exit 1; }

# Build Flex delegate with Bazel and copy to build dir

cd ../lib/tensorflow || { echo "TensorFlow directory not found!"; exit 1; }

#./configure

export TMPDIR=/var/fastnas/temp

bazel --output_user_root=/var/fastnas/bazel_cache build //tensorflow/lite:tensorflowlite --experimental_ui_max_stdouterr_bytes=10000000  --define xnn_enable_avx512fp16=false --define xnn_enable_avx512bf16=false --define xnn_enable_avxvnni=false  --define xnn_enable_avxvnniint8=false --define verbose_failure=1 || { echo "Bazel tensorflowlite build failed!"; exit 1; }

bazel --output_user_root=/var/fastnas/bazel_cache build //tensorflow/lite/delegates/flex:tensorflowlite_flex --config=monolithic --experimental_ui_max_stdouterr_bytes=10000000 --define xnn_enable_avx512amx=false --define xnn_enable_avxvnniint8=false --define xnn_enable_avxvnni=false --define xnn_enable_avx512fp16=false --define verbose_failure=1 || { echo "Bazel tensorflowlite_flex build failed!"; exit 1; }

# Ensure output dir exists
mkdir -p "../../$TFLITE_LIB_DIR/flatbuffers/include" || { echo "Failed to create flatbuffers include directory"; exit 1; }

cp -u bazel-bin/tensorflow/lite/libtensorflowlite.so "../../$TFLITE_LIB_DIR/" || exit 1
echo "✅ libtensorflowlite.so built and copied successfully."
cp -u bazel-bin/tensorflow/lite/delegates/flex/libtensorflowlite_flex.so "../../$TFLITE_LIB_DIR/" || exit 1
echo "✅ libtensorflowlite_flex.so built and copied successfully."
cp -r bazel-bin/external/flatbuffers/_virtual_includes/flatbuffers "../../$TFLITE_LIB_DIR/flatbuffers/include" || exit 1
echo "✅ flatbuffers built and copied successfully."

# Go back to the build directory

cd "../../build" || { echo "Failed to change directory to root"; exit 1; }


# Check if the CMakeCache.txt exists and if BUILD_SERVER is correctly configured
if [ ! -f "CMakeCache.txt" ] || ! grep -q "BUILD_SERVER:BOOL=ON" CMakeCache.txt; then
    echo "CMakeCache.txt does not exist or configuration changed. Running CMake..."
    cmake -DTARGET_ARCH=x86_64 -DTARGET_OS=linux -DBUILD_SERVER:BOOL=ON -DBUILD_SERVER=ON -DDEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF -DTFLITE_ENABLE_XNNPACK=ON -DTFLITE_ENABLE_SELECT_TF_OPS=ON ..
else
    echo "CMakeCache.txt found and configuration is valid. Skipping CMake reconfiguration."
fi

echo "Building the project..."
make install -j$(nproc) || { echo "Install failed"; exit 1; }


if [ ! -d "/var/log/jarvis" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "/var/log/jarvis"
fi

echo "Build process completed."
export AVAHI_COMPAT_NOWARN=y