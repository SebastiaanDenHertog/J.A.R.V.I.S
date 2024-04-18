cmake_minimum_required(VERSION 3.10)
project(J.A.R.V.I.S)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to select the architecture, default is x86_64
set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture")

# Include the toolchain file based on the architecture
if(TARGET_ARCH STREQUAL "aarch64")
    set(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/cmake/aarch64_toolchain.cmake")
    message(STATUS "Using aarch64 toolchain")
elseif(TARGET_ARCH STREQUAL "x86_64")
    set(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/cmake/x86_64_toolchain.cmake")
    message(STATUS "Using x86_64 toolchain")
endif()

# Configure TensorFlow Lite build. We assume TensorFlow Lite's source is in a third_party directory
set(TFLITE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow/lite")

# Include directories for the project
include_directories("${PROJECT_SOURCE_DIR}/include")

# Include directories for TensorFlow Lite
include_directories("${TFLITE_SOURCE_DIR}")
include_directories("${TFLITE_SOURCE_DIR}/tools/make/downloads/flatbuffers/include")

# Add TensorFlow Lite directory to the project
add_subdirectory(${TFLITE_SOURCE_DIR} "${CMAKE_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

# Source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*/*.cpp" "${PROJECT_SOURCE_DIR}/src/*/*.c")

# Executable
add_executable(main "${PROJECT_SOURCE_DIR}/src/main.cpp" ${SOURCES})

# Link against TensorFlow Lite and other libraries
target_link_libraries(main PRIVATE "${PROJECT_SOURCE_DIR}/include"  tensorflow-lite -lbluetooth -lstdc++ -lm)

# Compiler options
target_compile_options(main PRIVATE -Wall -g -pthread)
