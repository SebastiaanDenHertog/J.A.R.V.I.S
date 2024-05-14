@echo off
setlocal

REM Set TensorFlow Lite branch (adjust as needed)
set "TENSORFLOW_LITE_BRANCH=master"

REM Name of the build directory
set "BUILD_DIR=build"

REM Check if the build directory exists
if exist "%BUILD_DIR%" (
    echo Build directory exists. Clearing it...
    rd /s /q "%BUILD_DIR%"
) else (
    echo Build directory does not exist. Creating it...
    mkdir "%BUILD_DIR%"
)

REM Navigate to the build directory
cd "%BUILD_DIR%"

REM Run CMake to configure the project and generate the build system
echo Running CMake...
cmake -G "Unix Makefiles" -DTARGET_ARCH=aarch64 -DBUILD_COMPONENT=full ..

REM Build the project
echo Building the project...
cmake --build . -- -j %NUMBER_OF_PROCESSORS%

REM Navigate back to the root project directory
cd ..

echo Build process completed.

endlocal
