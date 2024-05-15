# Just a Rather Very Intelligent System

# How to run
Linux: chmod +x build_project.sh
./build_project.sh

do this in the lib folder:
wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip
unzip libtorch-shared-with-deps-latest.zip
rm libtorch-shared-with-deps-latest.zip

in the mlpackchange cmakefile.txt to 
file(READ ${CMAKE_SOURCE_DIR}/src/mlpack/config.hpp CONFIG_CONTENTS) -> file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/mlpack/config.hpp CONFIG_CONTENTS)

for debug mode add -DDEBUG_MODE=ON to the build_project cmake

Building the Project change this in the build_project file you use

You can specify the target architecture, build component, and operating system when running CMake:

For AArch64 Linux:
``cmake -DTARGET_ARCH=aarch64 -DTARGET_OS=linux -DBUILD_COMPONENT=full ..
make -j$(nproc)``

For x86_64 Linux:
``cmake -DTARGET_ARCH=x86_64 -DTARGET_OS=linux -DBUILD_COMPONENT=full ..
make -j$(nproc)``

For Windows x86:
``cmake -DTARGET_ARCH=x86 -DTARGET_OS=windows -DBUILD_COMPONENT=full ..
cmake --build . -- -j %NUMBER_OF_PROCESSORS%``

-bash: ./build_project.sh: /bin/bash^M: bad interpreter: No such file or directory: FIX -> sed -i -e 's/\r$//' build_project.sh
