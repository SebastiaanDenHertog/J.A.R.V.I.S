cmake_minimum_required(VERSION 3.16)
project(J.A.R.V.I.S)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEBUG_MODE "Enable debug prints" ON)

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0042 NEW)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to Release, for debug builds use '-DCMAKE_BUILD_TYPE=Debug'.")
    set(CMAKE_BUILD_TYPE Release)
else()
    message(STATUS "Build type is ${CMAKE_BUILD_TYPE}.")
    add_definitions(-DDEBUG_MODE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")

if((UNIX AND NOT APPLE) OR USE_X11)
    if(NOT NO_X11_DEPS)
        find_package(X11)

        if(X11_FOUND)
            message(STATUS "Will compile using X11 Libraries (use cmake option -DNO_X11_DEPS=ON if X11 dependence is not wanted)")
            set(LINK_LIBS ${LINK_LIBS} ${X11_LIBRARIES})
            include_directories(${X11_INCLUDE_DIR})
        else()
            message(STATUS "X11 libraries not found, will compile without X11 dependence")
        endif()
    else()
        message(STATUS "Will compile without X11 dependence")
    endif()
endif()

if(UNIX AND NOT APPLE)
    add_definitions(-DSUPPRESS_AVAHI_COMPAT_WARNING)
else()
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Define target architecture and OS
set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86_64, x86, aarch64)")
set(TARGET_OS "linux" CACHE STRING "Target operating system (linux, windows)")

# Options for different components
option(BUILD_SERVER "Build only server components" OFF)
option(BUILD_CLIENT "Build only client components" OFF)

# Default to BUILD_SERVER if none are selected
if(NOT BUILD_SERVER AND NOT BUILD_CLIENT)
    message(STATUS "No specific build selected, defaulting to server build.")
    set(BUILD_SERVER ON)
endif()

# Set preprocessor definitions based on the build type
if(BUILD_SERVER)
    add_definitions(-DBUILD_SERVER)
    message("Building server code")
elseif(BUILD_CLIENT)
    add_definitions(-DBUILD_CLIENT)
    message("Building client code")
endif()

# Include the toolchain file based on the architecture and OS
if("${TARGET_ARCH}" STREQUAL "aarch64" AND "${TARGET_OS}" STREQUAL "linux")
    set(CMAKE_TOOLCHAIN_FILE ${PROJECT_SOURCE_DIR}/cmake/aarch64_linux_toolchain.cmake)
    message(STATUS "Using aarch64 Linux toolchain")
elseif("${TARGET_ARCH}" STREQUAL "x86_64" AND "${TARGET_OS}" STREQUAL "linux")
    set(CMAKE_TOOLCHAIN_FILE ${PROJECT_SOURCE_DIR}/cmake/x86_64_linux_toolchain.cmake)
    message(STATUS "Using x86_64 Linux toolchain")
elseif("${TARGET_ARCH}" STREQUAL "x86" AND "${TARGET_OS}" STREQUAL "windows")
    set(CMAKE_TOOLCHAIN_FILE ${PROJECT_SOURCE_DIR}/cmake/x86_windows_toolchain.cmake)
    message(STATUS "Using x86 Windows toolchain")
else()
    message(FATAL_ERROR "Unsupported combination of ${TARGET_ARCH} and ${TARGET_OS}")
endif()

# Find Boost 1.74+
find_package(Boost 1.74 REQUIRED COMPONENTS system thread filesystem log log_setup regex date_time chrono atomic)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_compile_definitions(BOOST_LOG_DYN_LINK) # Necessary if using dynamic linking for Boost.Log
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
    include_directories(${OpenSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found: ${OPENSSL_VERSION}")
endif()

# Check if nlohmann_json exists and handle accordingly
find_package(nlohmann_json)

if(NOT nlohmann_json_FOUND)
    message(WARNING "nlohmann_json not found, will build from source.")

    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/json)
        message(FATAL_ERROR "Json source directory not found. Please ensure that submodules are downloaded. Run:\n git submodule update --init --recursive")
    else()
        set(JSON_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/json)
        add_subdirectory(${JSON_SOURCE_DIR} ${CMAKE_BINARY_DIR}/json)
        include_directories(${JSON_SOURCE_DIR}/include)
    endif()
else()
    message(STATUS "nlohmann_json found")
    include_directories(${JSON_INCLUDE_DIR})
endif()

if(BUILD_SERVER)
    find_package(OpenCV)

    if(NOT OpenCV_FOUND)
        message(WARNING "OpenCV not found, will build from source.")

        if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/opencv)
            message(FATAL_ERROR "OpenCV source directory not found. Please ensure that submodules are downloaded. Run:\n git submodule update --init --recursive")
        else()
            set(OPENCV_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/opencv)
            add_subdirectory(${OPENCV_SOURCE_DIR} ${CMAKE_BINARY_DIR}/opencv)
            include_directories(${OPENCV_SOURCE_DIR}/include)
            set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui) # Add other required OpenCV modules here
        endif()
    else()
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        include_directories(${OpenCV_INCLUDE_DIRS})
    endif()
endif()

if(BUILD_SERVER)
    find_package(Whisper)

    if(NOT Whisper_FOUND)
        message(WARNING "Whisper.cpp not found, will build from source.")

        if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/whisper.cpp)
            message(FATAL_ERROR "Whisper.cpp source directory not found. Please ensure that submodules are downloaded. Run:\n git submodule update --init --recursive")
        else()
            # Set Whisper source directory
            set(WHISPER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/whisper.cpp)

            # Add Whisper.cpp directory to the project
            add_subdirectory(${WHISPER_SOURCE_DIR} ${CMAKE_BINARY_DIR}/whisper EXCLUDE_FROM_ALL)

            # Include Whisper headers
            include_directories(${WHISPER_SOURCE_DIR})

            # Link the Whisper library
            set(WhisperCPP_LIBRARIES whisper)
        endif()
    else()
        message(STATUS "Whisper.cpp found")
        include_directories(${WhisperCPP_INCLUDE_DIR})
    endif()

    find_package(LlamaCPP)

    if(NOT LlamaCPP_FOUND)
        message(WARNING "llama.cpp not found, will build from source.")

        if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/llama.cpp)
            message(FATAL_ERROR "llama.cpp source directory not found. Please ensure that submodules are downloaded. Run:\n git submodule update --init --recursive")
        else()
            # Set llama.cpp source directory
            set(LLAMA_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/llama.cpp)
            include_directories(${LLAMA_SOURCE_DIR})

            # Add llama.cpp directory to the project
            add_subdirectory(${LLAMA_SOURCE_DIR} ${CMAKE_BINARY_DIR}/llama EXCLUDE_FROM_ALL)
        endif()
    else()
        message(STATUS "llama.cpp found")
        include_directories(${LlamaCPP_INCLUDE_DIR})
    endif()

    find_package(tensorflow-lite)

    if(NOT tensorflow-lite_FOUND)
        message(WARNING "TensorFlow not found, will build from source.")

        if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/tensorflow)
            message(FATAL_ERROR "TensorFlow source directory not found. Please ensure that submodules are downloaded. Run:\n git submodule update --init --recursive")
        else()
            # Configure TensorFlow Lite build. We assume TensorFlow Lite's source is in a third_party directory
            set(TFLITE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow/lite)
            include_directories(${TFLITE_SOURCE_DIR})
            include_directories(${TFLITE_SOURCE_DIR}/tools/make/downloads/flatbuffers/include)

            # Add TensorFlow Lite directory to the project
            add_subdirectory(${TFLITE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/tensorflow-lite EXCLUDE_FROM_ALL)
        endif()
    else()
        message(STATUS "TensorFlow found")
        include_directories(${TENSORFLOW_INCLUDE_DIR})
    endif()
endif()

if(BUILD_CLIENT)
    # Find GLIB using pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB2 REQUIRED glib-2.0)

    include_directories(${GLIB2_INCLUDE_DIRS})
    link_directories(${GLIB2_LIBRARY_DIRS})
    add_definitions(${GLIB2_CFLAGS_OTHER})

    if(UNIX AND NOT APPLE)
        add_definitions(-DSUPPRESS_AVAHI_COMPAT_WARNING)
    else()
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    endif()

    add_subdirectory(include/AirPlayServer/lib)
    add_subdirectory(include/AirPlayServer/lib/llhttp)
    add_subdirectory(include/AirPlayServer/lib/playfair)
    add_subdirectory(include/AirPlayServer/renderers)

    if(GST_MACOS)
        add_definitions(-DGST_MACOS)
        message(STATUS "define GST_MACOS")

        if(GST_124)
            add_definitions(-DGST_124)
            message(STATUS "define GST_124")
        endif()

        set(AIRPLAYSERVER_PATH -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -lglib-2.0 -lgobject-2.0)
    endif()
endif()

# Find the httpserver package
find_package(httpserver)

if(NOT httpserver_FOUND)
    message(WARNING "LibHTTPServer not found, will build from source.")

    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/libhttpserver)
        message(FATAL_ERROR "LibHTTPServer source directory not found. Please ensure that submodules are downloaded. Run:\n git submodule update --init --recursive")
    else()
        set(LibHttpServer_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/libhttpserver)
        include_directories(${LibHttpServer_SOURCE_DIR}/src) # Ensure the src folder is included
        include_directories(${CMAKE_BINARY_DIR}/httpserver/include) # This might change depending on your structure
        add_subdirectory(${LibHttpServer_SOURCE_DIR} ${CMAKE_BINARY_DIR}/httpserver EXCLUDE_FROM_ALL)
    endif()
else()
    message(STATUS "LibHTTPServer found")
    include_directories(${LibHttpServer_INCLUDE_DIR}) # Make sure this points to the correct path
endif()

message(STATUS "buildstatus ${BUILD_SERVER} ${BUILD_CLIENT}")

# Include directories for the project
include_directories(
    ${PROJECT_SOURCE_DIR}/include/additions
    ${PROJECT_SOURCE_DIR}/include/ac10x
    ${PROJECT_SOURCE_DIR}/include/bluethoothcomm
    ${PROJECT_SOURCE_DIR}/include/ml
    ${PROJECT_SOURCE_DIR}/include/networkmanager
    ${PROJECT_SOURCE_DIR}/include/mediaplayer
    ${PROJECT_SOURCE_DIR}/include/AirPlayServer
    ${PROJECT_SOURCE_DIR}/include/hardwareinterface
    ${PROJECT_SOURCE_DIR}/include/task
    ${PROJECT_SOURCE_DIR}/include/taskprocessor
    ${PROJECT_SOURCE_DIR}/include/inputhandler
    ${PROJECT_SOURCE_DIR}/include/HomeAssistantAPI
    ${PROJECT_SOURCE_DIR}/include/Tokenizer
    ${PROJECT_SOURCE_DIR}/include/ClientInfo
    ${PROJECT_SOURCE_DIR}/include/prometheus
    ${PROJECT_SOURCE_DIR}/include/webServer
    ${PROJECT_SOURCE_DIR}/include/whisperTranscriber
    ${PROJECT_SOURCE_DIR}/include/dataStructures
    ${PROJECT_SOURCE_DIR}/include/WatchDog
    ${PROJECT_SOURCE_DIR}/include/Configuration
)

# Source files
if(BUILD_SERVER)
    message("Building server code")

    # Common (default) sources
    file(GLOB DEFAULT_SOURCES ${PROJECT_SOURCE_DIR}/src/default/*/*.cpp)
    file(GLOB DEFAULT_SOURCESS ${PROJECT_SOURCE_DIR}/src/default/*/*/*.cpp)

    # Server-specific sources (no client sources)
    file(GLOB SERVER_SOURCES ${PROJECT_SOURCE_DIR}/src/server/*/*.cpp)

    # Ensure client sources are not included
    add_executable(server ${PROJECT_SOURCE_DIR}/src/main.cpp ${SERVER_SOURCES} ${DEFAULT_SOURCES} ${DEFAULT_SOURCESS})
    add_compile_definitions(SERVER_BUILD)
elseif(BUILD_CLIENT)
    message("Building client code")

    # Common (default) sources
    file(GLOB DEFAULT_SOURCES ${PROJECT_SOURCE_DIR}/src/default/*/*.cpp)
    file(GLOB DEFAULT_SOURCESS ${PROJECT_SOURCE_DIR}/src/default/*/*/*.cpp)

    # Client-specific sources (no server sources)
    file(GLOB CLIENT_SOURCES ${PROJECT_SOURCE_DIR}/src/client/*/*.cpp)

    add_executable(client ${PROJECT_SOURCE_DIR}/src/main.cpp ${CLIENT_SOURCES} ${DEFAULT_SOURCES} ${DEFAULT_SOURCESS})
    add_compile_definitions(CLIENT_BUILD)
endif()

message(STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")

# Linking libraries
if(${TARGET_OS} STREQUAL linux)
    if(BUILD_SERVER)
        target_link_libraries(server PRIVATE ${Boost_LIBRARIES} ${WhisperCPP_LIBRARIES} ${LlamaCPP_LIBRARIES} tensorflow-lite OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -lm -pthread i2c ${GLIB2_LIBRARIES} -lX11 ${OpenCV_LIBS} nlohmann_json::nlohmann_json libhttpserver)
    elseif(BUILD_CLIENT)
        target_link_libraries(client PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -lm -pthread -lpigpio -lrt -lspidev-lib++ i2c ${GLIB2_LIBRARIES} renderers airplay -lX11 libhttpserver)
    endif()
elseif(${TARGET_OS} STREQUAL windows)
    if(BUILD_SERVER)
        target_link_libraries(server PRIVATE ${Boost_LIBRARIES} ${WhisperCPP_LIBRARIES} ${LlamaCPP_LIBRARIES} tensorflow-lite OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -pthread ${GLIB2_LIBRARIES} -lX11 ${OpenCV_LIBS} nlohmann_json::nlohmann_json libhttpserver)
    elseif(BUILD_CLIENT)
        target_link_libraries(client PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -pthread -lpigpio -lrt -lspidev-lib++ i2c ${GLIB2_LIBRARIES} renderers airplay -lX11 libhttpserver)
    endif()
endif()

# Compile options
if(${TARGET_OS} STREQUAL linux)
    if(BUILD_SERVER)
        target_compile_options(server PRIVATE -Wall -g -pthread -DSERVER_MODE)
    elseif(BUILD_CLIENT)
        target_compile_options(client PRIVATE -Wall -g -pthread)
    endif()
elseif(${TARGET_OS} STREQUAL windows)
    if(BUILD_SERVER)
        target_compile_options(server PRIVATE -Wall -g -pthread -DSERVER_MODE)
    elseif(BUILD_CLIENT)
        target_compile_options(client PRIVATE -Wall -g -pthread)
    endif()
endif()
