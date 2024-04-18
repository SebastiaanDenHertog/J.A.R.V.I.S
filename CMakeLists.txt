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

# Specify the path to the Torch library
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/lib/libtorch")
find_package(Torch REQUIRED)

# Source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*/*.cpp" "${PROJECT_SOURCE_DIR}/src/*/*.c")

# Executable
add_executable(main "${PROJECT_SOURCE_DIR}/src/main.cpp" ${SOURCES})

# Include directories
target_include_directories(main PRIVATE "${PROJECT_SOURCE_DIR}/include")

# Compiler and linker options
target_compile_options(main PRIVATE -Wall -g -pthread ${TORCH_CXX_FLAGS})
target_link_libraries(main PRIVATE "${TORCH_LIBRARIES}" -lbluetooth -lstdc++ -lm)

# Windows DLLs copying for PyTorch
if (MSVC)
    file(GLOB TORCH_DLLs "${PROJECT_SOURCE_DIR}/lib/libtorch/lib/*.dll")
    add_custom_command(TARGET main POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       ${TORCH_DLLs}
                       $<TARGET_FILE_DIR:main>)
endif()
