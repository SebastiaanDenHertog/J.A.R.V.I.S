# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(TENSORFLOW_INCLUDE_PATH "/path/to/tensorflow/include")
set(TENSORFLOW_LIB_PATH "/path/to/tensorflow/lib")


include_directories(${TENSORFLOW_INCLUDE_PATH})

# Add executable target
add_executable(${PROJECT_NAME} main.cpp)

# Link TensorFlow libraries
target_link_libraries(${PROJECT_NAME} ${TENSORFLOW_LIB_PATH}/libtensorflow_cc.so ${TENSORFLOW_LIB_PATH}/libtensorflow_framework.so)