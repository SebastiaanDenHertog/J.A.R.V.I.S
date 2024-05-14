# Specify the target system
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Specify the compiler
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

# Optional: Specify the Windows SDK or Visual Studio paths if needed
# set(CMAKE_WINDOWS_SDK_ROOT "C:/Path/To/WindowsSDK")
# set(CMAKE_VS_PLATFORM_TOOLSET "v142")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
