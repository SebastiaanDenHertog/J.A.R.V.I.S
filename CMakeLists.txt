cmake_minimum_required(VERSION 3.10)
project(J.A.R.V.I.S)
set(CMAKE_CXX_STANDARD 17)

# Option to select the architecture, default is x86_64
set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture")

# Include the toolchain file based on the architecture
if(TARGET_ARCH STREQUAL "aarch64")
    include("arm64_toolchain.cmake")
elseif(TARGET_ARCH STREQUAL "x86_64")
    include("x86_64_toolchain.cmake")
endif()

# Compile options
add_compile_options(-Wall -g)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lstdc++ -lm")

# Include directories
include_directories("${PROJECT_SOURCE_DIR}/include")

# TensorFlow setup
set(TENSORFLOW_SOURCE_DIR "" CACHE PATH "Directory that contains the TensorFlow project")
if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/tensorflow" ABSOLUTE)
endif()

add_subdirectory(
  "./lib/tensorflow/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
  EXCLUDE_FROM_ALL
)

# Source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*/*.cpp" "${PROJECT_SOURCE_DIR}/src/*/*.c")

# Executable
add_executable(main "${PROJECT_SOURCE_DIR}/src/main.cpp" ${SOURCES})

# Linking libraries
target_link_libraries(main PRIVATE tensorflow-lite -lbluetooth)
