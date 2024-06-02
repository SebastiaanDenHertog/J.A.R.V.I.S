cmake_minimum_required(VERSION 3.16)
project(J.A.R.V.I.S)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEBUG_MODE "Enable debug prints" OFF)

if(DEBUG_MODE)
    add_definitions(-DDEBUG_MODE)
endif()

if ((UNIX AND NOT APPLE) OR USE_X11)
  if (NOT NO_X11_DEPS)
    find_package(X11)
    if (X11_FOUND)
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

# Find Boost
find_package(Boost REQUIRED COMPONENTS system thread filesystem log log_setup regex date_time chrono atomic)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_compile_definitions(BOOST_LOG_DYN_LINK) # Necessary if using dynamic linking for Boost.Log
else()
    message(FATAL_ERROR "Boost not found!")
endif()

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found: ${OPENSSL_VERSION}")
endif()


if(BUILD_COMPONENT STREQUAL "full" OR BUILD_COMPONENT STREQUAL "server")
    # Configure TensorFlow Lite build. We assume TensorFlow Lite's source is in a third_party directory
    set(TFLITE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow/lite")
    include_directories("${TFLITE_SOURCE_DIR}")
    include_directories("${TFLITE_SOURCE_DIR}/tools/make/downloads/flatbuffers/include")
    # Add TensorFlow Lite directory to the project
    add_subdirectory(${TFLITE_SOURCE_DIR} "${CMAKE_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)
endif()


# Find GLIB using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})
if( UNIX AND NOT APPLE )
add_definitions( -DSUPPRESS_AVAHI_COMPAT_WARNING )
else()
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
endif()
add_subdirectory( include/AirPlayServer/lib/llhttp )
add_subdirectory( include/AirPlayServer/lib/playfair )
add_subdirectory( include/AirPlayServer/lib )
add_subdirectory( include/AirPlayServer/renderers )
if  ( GST_MACOS )
    add_definitions( -DGST_MACOS )
    message ( STATUS "define GST_MACOS" )


if  ( GST_124 )
     add_definitions( -DGST_124 )
     message ( STATUS "define GST_124" )
endif()
    # create variable for the path to the AirPlayServer
    set(AIRPLAYSERVER_PATH "-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -lglib-2.0 -lgobject-2.0")
endif()

# Include directories for the project
include_directories(
    "${PROJECT_SOURCE_DIR}/include/additions"
    "${PROJECT_SOURCE_DIR}/include/audio_input"
    "${PROJECT_SOURCE_DIR}/include/bluethoothcomm"
    "${PROJECT_SOURCE_DIR}/include/ml"
    "${PROJECT_SOURCE_DIR}/include/web_server"
    "${PROJECT_SOURCE_DIR}/include/networkmanager"
    "${PROJECT_SOURCE_DIR}/include/mediaplayer"
    "${PROJECT_SOURCE_DIR}/include/AirPlayServer"
    "${PROJECT_SOURCE_DIR}/include/hardwareinterface"
)

file(GLOB DEFEALT_SOURCES "${PROJECT_SOURCE_DIR}/src/defealt/*/*.cpp")

# Source files
if(BUILD_COMPONENT STREQUAL "full")
    file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*/*/*.cpp")
    add_executable(main "${PROJECT_SOURCE_DIR}/src/full.cpp" ${SOURCES} ${DEFEALT_SOURCES})
elseif(BUILD_COMPONENT STREQUAL "server")
    file(GLOB SERVER_SOURCES "${PROJECT_SOURCE_DIR}/src/server/*/*.cpp")
    set(SOURCES ${SERVER_SOURCES})
    add_executable(main "${PROJECT_SOURCE_DIR}/src/server.cpp" ${SOURCES} ${DEFEALT_SOURCES})
    add_compile_definitions(SERVER_BUILD)
elseif(BUILD_COMPONENT STREQUAL "client")
    file(GLOB CLIENT_SOURCES "${PROJECT_SOURCE_DIR}/src/client/*/*.cpp")
    set(SOURCES ${CLIENT_SOURCES})
    add_executable(main "${PROJECT_SOURCE_DIR}/src/client.cpp" ${SOURCES} ${DEFEALT_SOURCES})
    add_compile_definitions(CLIENT_BUILD)
endif()

# Link against TensorFlow Lite and other libraries
if(TARGET_OS STREQUAL "linux")
    if(BUILD_COMPONENT STREQUAL "full" OR BUILD_COMPONENT STREQUAL "server")
        target_link_libraries(main PRIVATE ${Boost_LIBRARIES} ${AIRPLAYSERVER_PATH} tensorflow-lite OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -lm -pthread i2c ${GLIB2_LIBRARIES} renderers airplay -lX11)
    else()
        target_link_libraries(main PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -lm -pthread -lpigpio -lrt -lspidev-lib++ i2c ${GLIB2_LIBRARIES} renderers airplay -lX11)
    endif()
elseif(TARGET_OS STREQUAL "windows")
        if(BUILD_COMPONENT STREQUAL "full" OR BUILD_COMPONENT STREQUAL "server")
            target_link_libraries(main PRIVATE ${Boost_LIBRARIES} tensorflow-lite OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -pthread  ${GLIB2_LIBRARIES} renderers airplay -lX11)
        else()
            target_link_libraries(main PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto -lbluetooth -lstdc++ -pthread -lpigpio -lrt -lspidev-lib++ i2c ${GLIB2_LIBRARIES} renderers airplay -lX11) 
        endif()
endif()

# Compiler options
target_compile_options(main PRIVATE -Wall -g -pthread)
