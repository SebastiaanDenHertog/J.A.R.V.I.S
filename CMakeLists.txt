cmake_minimum_required(VERSION 3.16)
project(J.A.R.V.I.S)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to select the architecture, default is x86_64
set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86_64, x86, aarch64)")

# Option to select the build component, default is full build
set(BUILD_COMPONENT "full" CACHE STRING "Build component (full, server, or client)")

# Option to select the operating system, default is linux
set(TARGET_OS "linux" CACHE STRING "Target operating system (linux, windows)")

# Include the toolchain file based on the architecture and OS
if(TARGET_ARCH STREQUAL "aarch64" AND TARGET_OS STREQUAL "linux")
    set(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/cmake/aarch64_linux_toolchain.cmake")
    message(STATUS "Using aarch64 Linux toolchain")
elseif(TARGET_ARCH STREQUAL "x86_64" AND TARGET_OS STREQUAL "linux")
    set(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/cmake/x86_64_linux_toolchain.cmake")
    message(STATUS "Using x86_64 Linux toolchain")
elseif(TARGET_ARCH STREQUAL "x86" AND TARGET_OS STREQUAL "windows")
    set(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/cmake/x86_windows_toolchain.cmake")
    message(STATUS "Using x86 Windows toolchain")
else()
    message(FATAL_ERROR "Unsupported combination of TARGET_ARCH and TARGET_OS")
endif()

# Include directories for the project
include_directories("${PROJECT_SOURCE_DIR}/include")

if(BUILD_COMPONENT STREQUAL "full" OR BUILD_COMPONENT STREQUAL "server")
    # Configure TensorFlow Lite build. We assume TensorFlow Lite's source is in a third_party directory
    set(TFLITE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow/lite")
    include_directories("${TFLITE_SOURCE_DIR}")
    include_directories("${TFLITE_SOURCE_DIR}/tools/make/downloads/flatbuffers/include")
    # Add TensorFlow Lite directory to the project
    add_subdirectory(${TFLITE_SOURCE_DIR} "${CMAKE_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)
endif()

# Source files
if(BUILD_COMPONENT STREQUAL "full")
    file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*/*.cpp")
elseif(BUILD_COMPONENT STREQUAL "server")
    file(GLOB SERVER_SOURCES "${PROJECT_SOURCE_DIR}/src/server/*.cpp")
    set(SOURCES ${SERVER_SOURCES})
    add_definitions(-DSERVER_BUILD)
elseif(BUILD_COMPONENT STREQUAL "client")
    file(GLOB CLIENT_SOURCES "${PROJECT_SOURCE_DIR}/src/client/*.cpp")
    set(SOURCES ${CLIENT_SOURCES} )
    add_definitions(-DCLIENT_BUILD)
endif()

# Main executable
add_executable(main "${PROJECT_SOURCE_DIR}/src/main.cpp" ${SOURCES})

# Link against TensorFlow Lite and other libraries
if(TARGET_OS STREQUAL "linux")
    target_link_libraries(main PRIVATE tensorflow-lite -lbluetooth -lstdc++ -lm)
elseif(TARGET_OS STREQUAL "windows")
    target_link_libraries(main PRIVATE tensorflow-lite -lbluetooth -lstdc++)
endif()

# Compiler options
target_compile_options(main PRIVATE -Wall -g -pthread)
