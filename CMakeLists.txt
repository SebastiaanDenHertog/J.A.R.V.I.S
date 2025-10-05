cmake_minimum_required(VERSION 3.16)
project(J.A.R.V.I.S)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

option(DEBUG_MODE "Enable debug prints" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_compile_options(-fno-omit-frame-pointer)
add_link_options(-fno-omit-frame-pointer)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g3 -O0)
endif()

add_link_options(-rdynamic)

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
            message(STATUS "Will compile using X11 Libraries (use -DNO_X11_DEPS=ON to disable)")
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
        message(FATAL_ERROR "Json source directory not found. Run:\n git submodule update --init --recursive")
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
            message(FATAL_ERROR "OpenCV source directory not found. Run:\n git submodule update --init --recursive")
        else()
            set(OPENCV_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/opencv)
            add_subdirectory(${OPENCV_SOURCE_DIR} ${CMAKE_BINARY_DIR}/opencv)
            include_directories(${OPENCV_SOURCE_DIR}/include)
            set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui) # adjust as needed
        endif()
    else()
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        include_directories(${OpenCV_INCLUDE_DIRS})
    endif()

    find_package(Whisper)
    if(NOT Whisper_FOUND)
        message(WARNING "Whisper.cpp not found, will build from source.")
        if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/whisper.cpp)
            message(FATAL_ERROR "Whisper.cpp source directory not found. Run:\n git submodule update --init --recursive")
        else()
            set(WHISPER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/whisper.cpp)
            add_subdirectory(${WHISPER_SOURCE_DIR} ${CMAKE_BINARY_DIR}/whisper EXCLUDE_FROM_ALL)
            include_directories(${WHISPER_SOURCE_DIR})
            set(WhisperCPP_LIBRARIES whisper)
        endif()
    else()
        message(STATUS "Whisper.cpp found")
        include_directories(${WhisperCPP_INCLUDE_DIR})
    endif()

    set(BUILD_SHARED_LIBS_DEFAULT OFF)
    find_package(LlamaCPP)
    if(NOT LlamaCPP_FOUND)
        message(WARNING "llama.cpp not found, will build from source.")
        if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/llama.cpp)
            message(FATAL_ERROR "llama.cpp source directory not found. Run:\n git submodule update --init --recursive")
        else()
            set(LLAMA_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/llama.cpp)
            include_directories(${LLAMA_SOURCE_DIR})
            add_subdirectory(${LLAMA_SOURCE_DIR} ${CMAKE_BINARY_DIR}/llama EXCLUDE_FROM_ALL)
        endif()
    else()
        message(STATUS "llama.cpp found")
        include_directories(${LlamaCPP_INCLUDE_DIR})
    endif()

    add_definitions(-DFLEX_ALLOWLIST)
endif()

if(BUILD_CLIENT)
    # GLib via pkg-config
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

# LibHTTPServer
find_package(httpserver)
if(NOT httpserver_FOUND)
    message(WARNING "LibHTTPServer not found, will build from source.")
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/libhttpserver)
        message(FATAL_ERROR "LibHTTPServer source directory not found. Run:\n git submodule update --init --recursive")
    else()
        set(LibHttpServer_SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/libhttpserver)
        include_directories(${LibHttpServer_SOURCE_DIR}/src)
        include_directories(${CMAKE_BINARY_DIR}/httpserver)
        include_directories(${CMAKE_BINARY_DIR}/httpserver/include/details)
        include_directories(${CMAKE_BINARY_DIR}/httpserver/include/utils)
        add_subdirectory(${LibHttpServer_SOURCE_DIR} ${CMAKE_BINARY_DIR}/httpserver EXCLUDE_FROM_ALL)
    endif()
else()
    message(STATUS "LibHTTPServer found")
    include_directories(${LibHttpServer_INCLUDE_DIR})
endif()

find_package(OpenMP REQUIRED)

message(STATUS "buildstatus ${BUILD_SERVER} ${BUILD_CLIENT}")

# --- TFLite/Flex from Bazel build (REQUIRED: .so; OPTIONAL: allowlisted .a) ---
set(TF_LIB_DIR  "${CMAKE_SOURCE_DIR}/build/tflite_flex_lib")

# Probe libs (both cases)
file(GLOB TF_CC_CANDS  "${TF_LIB_DIR}/libtensorflow_cc.so*")
file(GLOB TF_FW_CANDS  "${TF_LIB_DIR}/libtensorflow_framework.so*")
file(GLOB TSL_CANDS    "${TF_LIB_DIR}/libtsl.so*")
file(GLOB TFLITE_CANDS "${TF_LIB_DIR}/libtensorflowlite.so*")
file(GLOB FLEX_CANDS   "${TF_LIB_DIR}/libtensorflowlite_flex.so*")
set(TF_SRC_DIR "${CMAKE_SOURCE_DIR}/lib/tensorflow")

# Project include dirs
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
if(BUILD_SERVER)
    message("Building server code")
    file(GLOB DEFAULT_SOURCES ${PROJECT_SOURCE_DIR}/src/default/*/*.cpp)
    file(GLOB DEFAULT_SOURCESS ${PROJECT_SOURCE_DIR}/src/default/*/*/*.cpp)
    file(GLOB SERVER_SOURCES ${PROJECT_SOURCE_DIR}/src/server/*/*.cpp)
    add_executable(server ${PROJECT_SOURCE_DIR}/src/main.cpp ${SERVER_SOURCES} ${DEFAULT_SOURCES} ${DEFAULT_SOURCESS})

    if(NOT TF_CC_CANDS OR NOT TF_FW_CANDS)
        message(FATAL_ERROR "Server build expects full TF: libtensorflow_cc.so and libtensorflow_framework.so not found in ${TF_LIB_DIR}")
    endif()
    list(GET TF_CC_CANDS 0 TF_CC_LIB)
    list(GET TF_FW_CANDS 0 TF_FW_LIB)
    if(TSL_CANDS)
        list(GET TSL_CANDS 0 TSL_LIB)
    endif()

    file(GLOB TF_FW_CANDS "${FLEX_LIB_DIR}/libtensorflow_framework.so*")
    file(GLOB TSL_CANDS   "${FLEX_LIB_DIR}/libtsl.so*")
    if(TF_FW_CANDS)
        list(GET TF_FW_CANDS 0 TF_FRAMEWORK_LIB)
    endif()
    if(TSL_CANDS)
        list(GET TSL_CANDS 0 TSL_LIB)
    endif()

    # Make sure the linker and runtime loader look here first
    link_directories(${TF_LIB_DIR})

    include_directories(
            ${TF_SRC_DIR}
            ${TF_SRC_DIR}/bazel-bin
            ${TF_SRC_DIR}/bazel-bin/tensorflow
            ${TF_SRC_DIR}/bazel-out
    )

    set_target_properties(server PROPERTIES
            BUILD_RPATH "${TF_LIB_DIR}"
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${TF_LIB_DIR}"
            SKIP_BUILD_RPATH OFF
            SKIP_INSTALL_RPATH OFF
    )

    target_include_directories(server PRIVATE
            "${TF_LIB_DIR}"
            "${TF_LIB_DIR}/flatbuffers/include"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow/lite"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/tensorflow/lite/schema"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-bin"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/com_google_absl"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/com_google_protobuf/src"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/local_xla"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-bin/external/local_xla"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/local_tsl"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-bin/external/local_tsl"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/eigen_archive"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/ml_dtypes_py"
            "${PROJECT_SOURCE_DIR}/lib/tensorflow/bazel-tensorflow/external/farmhash_archive/src"
    )

    # Ensure the linker keeps dependent libs
    target_link_options(server PRIVATE -Wl,--no-as-needed)

    add_compile_definitions(SERVER_BUILD)

elseif(BUILD_CLIENT)
    message("Building client code")
    file(GLOB DEFAULT_SOURCES ${PROJECT_SOURCE_DIR}/src/default/*/*.cpp)
    file(GLOB DEFAULT_SOURCESS ${PROJECT_SOURCE_DIR}/src/default/*/*/*.cpp)
    file(GLOB CLIENT_SOURCES ${PROJECT_SOURCE_DIR}/src/client/*/*.cpp)
    add_executable(client ${PROJECT_SOURCE_DIR}/src/main.cpp ${CLIENT_SOURCES} ${DEFAULT_SOURCES} ${DEFAULT_SOURCESS})
    add_compile_definitions(CLIENT_BUILD)

    file(GLOB TFLITE_CANDS "${TF_LIB_DIR}/libtensorflowlite.so*")
    file(GLOB FLEX_CANDS   "${TF_LIB_DIR}/libtensorflowlite_flex.so*")
    if(NOT TFLITE_CANDS OR NOT FLEX_CANDS)
        message(FATAL_ERROR "Client build expects TFLite libs in ${TF_LIB_DIR}")
    endif()
    list(GET TFLITE_CANDS 0 TFLITE_LIB)
    list(GET FLEX_CANDS   0 FLEX_LIB)
    set(ALLOWLIST_A "${TF_LIB_DIR}/liballowlisted_flex_ops_lib.a")
    if(EXISTS "${ALLOWLIST_A}")
        set(HAVE_TFLITE_FLEX_ALLOWLIST 1)
    else()
        set(HAVE_TFLITE_FLEX_ALLOWLIST 0)
    endif()
    link_directories(${TF_LIB_DIR})
    set_target_properties(client PROPERTIES
        BUILD_RPATH "${TF_LIB_DIR}"
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${TF_LIB_DIR}"
    )

endif()

# Linking
if(${TARGET_OS} STREQUAL linux)
    if(BUILD_SERVER)
        target_link_libraries(server PRIVATE
                ${Boost_LIBRARIES}
                ${WhisperCPP_LIBRARIES}
                ${LlamaCPP_LIBRARIES}
                OpenSSL::SSL
                OpenSSL::Crypto
                -lbluetooth -lstdc++ -lm -pthread
                i2c ${GLIB2_LIBRARIES}
                -lX11
                ${OpenCV_LIBS}
                nlohmann_json::nlohmann_json
                libhttpserver
                OpenMP::OpenMP_CXX
                ${TF_CC_LIB}
                ${TF_FW_LIB}
                dl
        )
        if(HAVE_TFLITE_FLEX_ALLOWLIST)
            target_link_libraries(server PRIVATE "${ALLOWLIST_A}")
        endif()

        # Explicitly avoid _GLIBCXX_DEBUG for this target
        target_compile_definitions(server PRIVATE $<$<CONFIG:Release>:NDEBUG>)

    elseif(BUILD_CLIENT)
        target_link_libraries(client PRIVATE
                ${Boost_LIBRARIES}
                OpenSSL::SSL OpenSSL::Crypto
                -lbluetooth -lstdc++ -lm -pthread
                -lpigpio -lrt -lspidev-lib++
                i2c ${GLIB2_LIBRARIES}
                renderers airplay
                -lX11
                libhttpserver
                OpenMP::OpenMP_CXX
                ${TFLITE_LIB}
                ${FLEX_LIB}
                dl
        )
        if(HAVE_TFLITE_FLEX_ALLOWLIST)
            target_link_libraries(client PRIVATE "${ALLOWLIST_A}")
        endif()
    endif()

elseif(${TARGET_OS} STREQUAL windows)
    if(BUILD_SERVER)
        target_compile_options(server PRIVATE -Wall -g -pthread -DSERVER_MODE)
        target_link_libraries(server PRIVATE
                ${Boost_LIBRARIES}
                ${WhisperCPP_LIBRARIES}
                ${LlamaCPP_LIBRARIES}
                OpenSSL::SSL
                OpenSSL::Crypto
                -lbluetooth -lstdc++ -lm -pthread
                i2c ${GLIB2_LIBRARIES}
                -lX11
                ${OpenCV_LIBS}
                nlohmann_json::nlohmann_json
                libhttpserver
                OpenMP::OpenMP_CXX
                ${TF_CC_LIB}
                ${TF_FW_LIB}
                dl
        )
        if(HAVE_TFLITE_FLEX_ALLOWLIST)
            target_link_libraries(server PRIVATE "${ALLOWLIST_A}")
        endif()

        # Explicitly avoid _GLIBCXX_DEBUG for this target
        target_compile_definitions(server PRIVATE $<$<CONFIG:Release>:NDEBUG>)
    elseif(BUILD_CLIENT)
        target_compile_options(client PRIVATE -Wall -g -pthread)
        target_link_libraries(client PRIVATE
                ${Boost_LIBRARIES}
                OpenSSL::SSL OpenSSL::Crypto
                -lbluetooth -lstdc++ -lm -pthread
                -lpigpio -lrt -lspidev-lib++
                i2c ${GLIB2_LIBRARIES}
                renderers airplay
                -lX11
                libhttpserver
                OpenMP::OpenMP_CXX
                ${TFLITE_LIB}
                ${FLEX_LIB}
                dl
        )
        if(HAVE_TFLITE_FLEX_ALLOWLIST)
            target_link_libraries(client PRIVATE "${ALLOWLIST_A}")
        endif()
    endif()
endif()

configure_file(config.json /usr/local/share/jarvis/  COPYONLY)
configure_file(config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)

# Compile options & install
if(${TARGET_OS} STREQUAL linux)
    if(BUILD_SERVER)
        target_compile_options(server PRIVATE -Wall -g -pthread -DSERVER_MODE)
        install(TARGETS server RUNTIME DESTINATION bin)

        install(TARGETS whisper
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin
                PUBLIC_HEADER DESTINATION include)

        if(TARGET ggml)
            set_target_properties(ggml PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/lib/ggml/include/ggml.h")
            install(TARGETS ggml
                    LIBRARY DESTINATION lib
                    ARCHIVE DESTINATION lib
                    RUNTIME DESTINATION bin
                    PUBLIC_HEADER DESTINATION include)
        endif()
        if(TARGET ggml-cpu)
            install(TARGETS ggml-cpu
                    LIBRARY DESTINATION lib
                    ARCHIVE DESTINATION lib
                    RUNTIME DESTINATION bin
                    PUBLIC_HEADER DESTINATION include)
        endif()
        if(TARGET ggml-base)
            install(TARGETS ggml-base
                    LIBRARY DESTINATION lib
                    ARCHIVE DESTINATION lib
                    RUNTIME DESTINATION bin
                    PUBLIC_HEADER DESTINATION include)
        endif()

        install(TARGETS libhttpserver
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin)

        # Optional TF lite/flex artifacts
        if(HAVE_TFLITE_FLEX_ALLOWLIST)
            install(FILES ${FLEX_LIB_DIR}/liballowlisted_flex_ops_lib.a
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
        endif()

        install(FILES
                ${TF_LIB_DIR}/libtensorflow_cc.so.2
                ${TF_LIB_DIR}/libtensorflow_framework.so.2
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

        if(TSL_LIB)
            install(FILES ${TSL_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
        endif()

        install(FILES resources/jarvis_server.service
                DESTINATION /lib/systemd/system)

        install(DIRECTORY ${PROJECT_SOURCE_DIR}/webserver
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/jarvis
                COMPONENT webfiles)
        add_definitions(-DWEBSERVER_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/jarvis/webserver\")

        install(DIRECTORY ${PROJECT_SOURCE_DIR}/models
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/jarvis
                COMPONENT models)
        add_definitions(-DMODELS_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/jarvis/models\")

        install(CODE "
            execute_process(COMMAND systemctl daemon-reload)
            execute_process(COMMAND systemctl enable jarvis_server.service)
        ")

    elseif(BUILD_CLIENT)
        target_compile_options(client PRIVATE -Wall -g -pthread)
        install(TARGETS client RUNTIME DESTINATION bin)
        install(TARGETS libhttpserver
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin)

        install(FILES resources/jarvis_client.service
                DESTINATION /lib/systemd/system)

        install(DIRECTORY ${PROJECT_SOURCE_DIR}/webserver
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/jarvis
                COMPONENT webfiles)
        add_definitions(-DWEBSERVER_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/jarvis/webserver\")

        install(DIRECTORY ${PROJECT_SOURCE_DIR}/models
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/jarvis
                COMPONENT models)
        add_definitions(-DMODELS_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/jarvis/models\")

        install(CODE "
            execute_process(COMMAND systemctl daemon-reload)
            execute_process(COMMAND systemctl enable jarvis_client.service)
        ")
    endif()
elseif(${TARGET_OS} STREQUAL windows)
    if(BUILD_SERVER)
        target_compile_options(server PRIVATE -Wall -g -pthread -DSERVER_MODE)
        install(TARGETS server RUNTIME DESTINATION bin)
        install(TARGETS whisper
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin)
        # (Windows-specific TF/TFLite linking omitted)
    elseif(BUILD_CLIENT)
        target_compile_options(client PRIVATE -Wall -g -pthread)
        install(TARGETS client RUNTIME DESTINATION bin)
        install(TARGETS libhttpserver
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin)
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/webserver
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/jarvis
                COMPONENT webfiles)
        add_definitions(-DWEBSERVER_RESOURCE_PATH=\"${CMAKE_INSTALL_PREFIX}/share/jarvis/webserver\")
        install(CODE "
            execute_process(
                COMMAND sc create JarvisServerService
                binPath= \"\${CMAKE_INSTALL_PREFIX}/bin/client.exe\"
                start= auto
            )
        ")
        install(CODE "
            execute_process(
                COMMAND sc start JarvisServerService
            )
        ")
    endif()
endif()