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

