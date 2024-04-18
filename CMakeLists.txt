cmake_minimum_required(VERSION 3.10)
project(J.A.R.V.I.S)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to select the architecture, default is x86_64
set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture")

# Include the toolchain file based on the architecture
if(TARGET_ARCH STREQUAL "aarch64")
    include("arm64_toolchain.cmake")
elseif(TARGET_ARCH STREQUAL "x86_64")
    include("x86_64_toolchain.cmake")
endif()

# Setting option to download dependencies for mlpack
set(DOWNLOAD_DEPENDENCIES ON CACHE BOOL "Download dependencies for mlpack")

# Include the mlpack library directory
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/mlpack/")

# Source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*/*.cpp" "${PROJECT_SOURCE_DIR}/src/*/*.c")

# Executable
add_executable(main "${PROJECT_SOURCE_DIR}/src/main.cpp" ${SOURCES})

# Include directories
target_include_directories(main PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/lib/mlpack/src")

# Link against mlpack and other libraries
target_link_libraries(main PRIVATE mlpack -lbluetooth -lstdc++ -lm)

# Compiler options
target_compile_options(main PRIVATE -Wall -g -pthread)
