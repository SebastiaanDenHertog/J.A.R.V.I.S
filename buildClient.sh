#!/bin/bash

# Name of the build directory
BUILD_DIR="build"
TFLITE_LIB_DIR="$BUILD_DIR/tflite_flex_lib"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "$BUILD_DIR"
fi

mkdir -p "$TFLITE_LIB_DIR/flatbuffers/include"

# Navigate to the build directory
cd "$BUILD_DIR" || { echo "Failed to change directory to $BUILD_DIR"; exit 1; }
cd ../

export TMPDIR=/var/fastnas/temp
export TF_NEED_MKL=0
export TF_ENABLE_ONEDNN_OPTS=0

# --- Build TFLite + Flex with GNU OpenMP (no MKL) ---
cd lib/tensorflow || { echo "TensorFlow directory not found!"; exit 1; }

if [ ! -f ".tf_configure.bazelrc" ]; then
  ./configure
fi

export TMPDIR=/var/fastnas/temp
export TF_NEED_MKL=0
export TF_ENABLE_ONEDNN_OPTS=0



if [ ! -f "./build/tflite_flex_lib/libtensorflowlite.so" ] || [ ! -f "./build/tflite_flex_lib/libtensorflowlite_flex.so" ]; then
    cd ./lib/tensorflow || { echo "TensorFlow directory not found!"; exit 1; }

    if [ ! -f ".tf_configure.bazelrc" ]; then
      ./configure
    fi
    echo "TensorFlow libraries not found in the build directory. Proceeding with the build."
    bazel --output_user_root=/var/fastnas/bazel_cache build \
       --config=monolithic --config=nogcp --config=nonccl \
       --copt=-fopenmp --linkopt=-fopenmp \
       //tensorflow/lite/delegates/flex:tensorflowlite_flex \
       //tensorflow/lite:tensorflowlite \
       --experimental_ui_max_stdouterr_bytes=10000000 \
       --define=tflite_convert_with_select_tf_ops=true \
       --define=SELECT_TF_OPS=true \
       --define=flex_with_tensorflow_ops=true \
       --define xnn_enable_avx512amx=false \
       --define xnn_enable_avxvnniint8=false \
       --define xnn_enable_avxvnni=false \
       --define xnn_enable_avx512fp16=false \
       --define verbose_failure=1 \
       || { echo "Bazel TF/TFLite build failed!"; exit 1; }
    cd ../
    cp -u bazel-bin/tensorflow/lite/libtensorflowlite.so                     "./build/tflite_flex_lib/"
    cp -u bazel-bin/tensorflow/lite/delegates/flex/libtensorflowlite_flex.so "./build/tflite_flex_lib/"
    cp -rf bazel-bin/external/flatbuffers/_virtual_includes/flatbuffers/flatbuffers "./build/tflite_flex_lib/flatbuffers/include" || true

else
    echo "TensorFlow libraries already exist in the build directory. Skipping the build."
fi

cd "$BUILD_DIR" || { echo "Failed to change directory to $BUILD_DIR"; exit 1; }

if [ ! -f "CMakeCache.txt" ] || ! grep -q "BUILD_CLIENT:BOOL=true" CMakeCache.txt; then
    echo "CMakeCache.txt does not exist or configuration changed. Running CMake..."
    cmake -DTARGET_ARCH=aarch64 -DTARGET_OS=linux -DBUILD_CLIENT:BOOL=ON -DBUILD_CLIENT=ON -DDEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF -DTFLITE_ENABLE_XNNPACK=ON -DTFLITE_ENABLE_SELECT_TF_OPS=ON -Wfatal-errors -ftemplate-backtrace-limit=0 ..
else
    echo "CMakeCache.txt found and configuration is valid. Skipping CMake reconfiguration."
fi

echo "Building the project..."
cmake --build . --target install -j"$(nproc)" || { echo "Install failed"; exit 1; }

# Navigate back to the root project directory
cd ..

if [ ! -d "/var/log/jarvis" ]; then
    echo "Build directory does not exist. Creating it..."
    mkdir "/var/log/jarvis"
fi

echo "Build process completed."
export AVAHI_COMPAT_NOWARN=y
