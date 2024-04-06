cmake_minimum_required(VERSION 3.10)
project(J.A.R.V.I.S)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/main.cpp)
add_compile_options(-Wall -g)


set(TENSORFLOW_SOURCE_DIR "" CACHE PATH
  "Directory that contains the TensorFlow project"
)
message(TENSORFLOW_SOURCE_DIR="${TENSORFLOW_SOURCE_DIR}")
message(CMAKE_CURRENT_LIST_DIR="${CMAKE_CURRENT_LIST_DIR}")
if(NOT TENSORFLOW_SOURCE_DIR)
  
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/lib/tensorflow"
    ABSOLUTE
  )
endif()

add_subdirectory(
  "./lib/tensorflow/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
  EXCLUDE_FROM_ALL
  )

include_directories(include)
file(GLOB SOURCppES "${PROJECT_SOURCE_DIR}/*/*.cpp")
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*/*.c")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SOURCES} ${SOURCppES} )

# Link TensorFlow libraries
target_link_libraries(${PROJECT_NAME} tensorflow-lite)
