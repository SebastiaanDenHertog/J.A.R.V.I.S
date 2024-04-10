@echo off
setlocal

REM Name of the build directory
set BUILD_DIR=build

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
