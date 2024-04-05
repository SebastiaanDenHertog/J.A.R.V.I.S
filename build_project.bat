@echo off
setlocal

REM Directory where TensorFlow Lite should be cloned
set TENSORFLOW_DIR=lib\tensorflow
set TENSORFLOW_LITE_REPO=https://github.com/tensorflow/tensorflow.git
set TENSORFLOW_LITE_BRANCH=master

REM Name of the build directory
set BUILD_DIR=build

REM Check if TensorFlow Lite directory exists, if not, clone it
if not exist "%TENSORFLOW_DIR%" (
    echo TensorFlow Lite directory does not exist. Cloning it...
    git clone --branch %TENSORFLOW_LITE_BRANCH% --depth 1 %TENSORFLOW_LITE_REPO% %TENSORFLOW_DIR%
) else (
    echo TensorFlow Lite directory exists. Skipping cloning...
)

REM Check if the build directory exists
if exist %BUILD_DIR% (
    echo Build directory exists. Clearing it...
    rmdir /s /q %BUILD_DIR%
)

echo Creating build directory...
mkdir %BUILD_DIR%

REM Navigate to the build directory
cd %BUILD_DIR%

REM Run CMake to configure the project and generate the build system
echo Running CMake...
cmake ..

REM Build the project
echo Building the project...
cmake --build . --config Release

REM Navigate back to the root project directory
cd ..

echo Build process completed.

endlocal
