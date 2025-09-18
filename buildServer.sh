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
cd ../

#bazel clean --expunge || { echo "Bazel clean failed!"; exit 1; }

export TMPDIR=/var/fastnas/temp
export TF_NEED_MKL=0
export TF_ENABLE_ONEDNN_OPTS=0

if [ ! -f "./build/tflite_flex_lib/libtensorflow_cc.so" ] || [ ! -f "./build/tflite_flex_lib/libtensorflow_framework.so" ]; then
    cd ./lib/tensorflow || { echo "TensorFlow directory not found!"; exit 1; }

    if [ ! -f ".tf_configure.bazelrc" ]; then
      ./configure
    fi
    echo "TensorFlow libraries not found in the build directory. Proceeding with the build."
    bazel --output_user_root=/var/fastnas/bazel_cache build \
          --config=nogcp --config=nonccl --config=dbg \
          --copt=-fopenmp --linkopt=-fopenmp \
          --cxxopt=-std=gnu++17 --host_cxxopt=-std=gnu++17 \
          //tensorflow:libtensorflow_cc.so \
          //tensorflow:libtensorflow_framework.so \
          --experimental_ui_max_stdouterr_bytes=10000000 \
          --define verbose_failure=1 \
          --define xnn_enable_avx512amx=false \
          --define xnn_enable_avxvnniint8=false \
          --define xnn_enable_avxvnni=false \
          --define xnn_enable_avx512fp16=false \
          || { echo "Bazel TF/TFLite build failed!"; exit 1; }
    cd ../
    cp -u ./lib/tensorflow/bazel-bin/tensorflow/libtensorflow_cc.so*              "./build/tflite_flex_lib/" || exit 1
    cp -u ./lib/tensorflow/bazel-bin/tensorflow/libtensorflow_framework.so*    "./build/tflite_flex_lib/" || exit 1
    # (optional) if your TF emits libtsl.so
    find bazel-bin -type f -name "libtsl.so*" -exec cp -u {} "1./$TFLITE_LIB_DIR/" \; 2>/dev/null || true
else
    echo "TensorFlow libraries already exist in the build directory. Skipping the build."
fi

cd "$BUILD_DIR" || { echo "Failed to change directory to $BUILD_DIR"; exit 1; }

# Check if the CMakeCache.txt exists and if BUILD_SERVER is correctly configured
if [ ! -f "CMakeCache.txt" ] || ! grep -q "BUILD_SERVER:BOOL=ON" CMakeCache.txt; then
    echo "CMakeCache.txt does not exist or configuration changed. Running CMake..."
    cmake -DTARGET_ARCH=x86_64 -DTARGET_OS=linux -DBUILD_SERVER:BOOL=ON -DBUILD_SERVER=ON -DDEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF -Wfatal-errors ..
else
    echo "CMakeCache.txt found and configuration is valid. Skipping CMake reconfiguration."
fi

echo "Building the project..."
cmake --build . --target install -j"$(nproc)" || { echo "Install failed"; exit 1; }

cd ..

if [ ! -d "/var/log/jarvis" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "/var/log/jarvis"
fi

echo "Build process completed."
export AVAHI_COMPAT_NOWARN=y