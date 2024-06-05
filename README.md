# Just a Rather Very Intelligent System

### news

## Highlights:


# Detailed description of J.A.R.V.I.S

This project include multiple projects:
* UxPlay [UxPlay site](https://github.com/FDH2/UxPlay))
  * It was initially developed by
[antimof](http://github.com/antimof/Uxplay) using code 
from OpenMAX-based [RPiPlay](https://github.com/FD-/RPiPlay), which in turn derives from
[AirplayServer](https://github.com/KqsMea8/AirplayServer),
[shairplay](https://github.com/juhovh/shairplay), and [playfair](https://github.com/EstebanKubata/playfair).


# Install or run:
## Linux
For running UxPlay:
If you use X11 Windows on Linux or *BSD, and wish to toggle in/out of fullscreen mode with a keypress
(F11 or Alt_L+Enter)
UxPlay needs to be built with a dependence on X11.  Starting with UxPlay-1.59, this will be done by
default **IF** the X11 development libraries are installed and detected.   Install these with
"`sudo apt install libx11-dev`".    If GStreamer < 1.20 is detected, a fix needed by 
screen-sharing apps (_e.g._, Zoom) will also be made.

* If X11 development libraries are present, but you
wish to build UxPlay *without* any X11 dependence, use
the cmake option `-DNO_X11_DEPS=ON`.

1. `sudo apt install libssl-dev libplist-dev`".
    (_unless you need to build OpenSSL and libplist from source_).
2.  `sudo apt install libavahi-compat-libdnssd-dev`
3.  `sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev`. (\*_Skip if you built Gstreamer from source_)



# How to run
Linux: chmod +x build_project.sh
./build_project.sh

do this in the lib folder:
wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip
unzip libtorch-shared-with-deps-latest.zip
rm libtorch-shared-with-deps-latest.zip

in the mlpackchange cmakefile.txt to 
file(READ ${CMAKE_SOURCE_DIR}/src/mlpack/config.hpp CONFIG_CONTENTS) -> file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/mlpack/config.hpp CONFIG_CONTENTS)

for debug mode add -DDEBUG_MODE=ON to the build_project cmake

Building the Project change this in the build_project file you use

You can specify the target architecture, build component, and operating system when running CMake:

For AArch64 Linux:
``cmake -DTARGET_ARCH=aarch64 -DTARGET_OS=linux -DBUILD_COMPONENT=full ..
make -j$(nproc)``

For x86_64 Linux:
``cmake -DTARGET_ARCH=x86_64 -DTARGET_OS=linux -DBUILD_COMPONENT=full ..
make -j$(nproc)``

For Windows x86:
``cmake -DTARGET_ARCH=x86 -DTARGET_OS=windows -DBUILD_COMPONENT=full ..
cmake --build . -- -j %NUMBER_OF_PROCESSORS%``

-bash: ./build_project.sh: /bin/bash^M: bad interpreter: No such file or directory: FIX -> sed -i -e 's/\r$//' build_project.sh

sudo apt install libboost-all-dev
sudo apt install libssl-dev

# Changelog

# Disclaimer

