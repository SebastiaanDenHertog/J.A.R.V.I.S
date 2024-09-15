  # Just a Rather Very Intelligent System

### news

## Highlights:


# Detailed description of J.A.R.V.I.S

This project include multiple projects:
* UxPlay [UxPlay site](https://github.com/FDH2/UxPlay)
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

git submodule init
git submodule update

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
All the resources in this repository are written using only freely available information from the internet. The code and related resources are meant for 
educational purposes only. It is the responsibility of the user to make sure all local laws are adhered to.
This project makes use of a third-party GPL library for handling FairPlay. The legal status of that library is unclear. Should you be a representative of 
Apple and have any objections against the legality of the library and its use in this project, please contact the developers and the appropriate steps 
will be taken.

Given the large number of third-party AirPlay receivers (mostly closed-source) available for purchase, it is our understanding that an open source 
implementation of the same functionality wouldn't violate any of Apple's rights either.

# UxPlay authors

_[adapted from fdraschbacher's notes on  RPiPlay antecedents]_

The code in this repository accumulated from various sources over time. Here
is an attempt at listing the various authors and the components they created:

UxPlay was initially created by **antimof** from RPiPlay, by replacing its Raspberry-Pi-adapted OpenMAX  video 
and audio rendering system with GStreamer rendering for
desktop Linux systems; the antimof work on code in `renderers/` was later backported to RPiPlay, and the antimof project became dormant, but was later 
revived at the [current GitHub site](http://github.com/FDH2/UxPlay)  to serve a wider community of users.

The previous authors of code included in UxPlay by inheritance from RPiPlay include:

* **EstebanKubata**: Created a FairPlay library called [PlayFair](https://github.com/EstebanKubata/playfair). Located in the `lib/playfair` folder. License: GNU GPL
* **Juho Vähä-Herttua** and contributors: Created an AirPlay audio server called [ShairPlay](https://github.com/juhovh/shairplay), including support for Fairplay based on PlayFair. Most of the code   in `lib/` originally stems from this project. License: GNU LGPLv2.1+
* **dsafa22**: Created an AirPlay 2 mirroring server [AirplayServer](https://github.com/dsafa22/AirplayServer) (seems gone now), for Android based on ShairPlay.   Code is 
  preserved [here](https://github.com/jiangban/AirplayServer), and [see here](https://github.com/FDH2/UxPlay/wiki/AirPlay2) for the description 
  of the analysis of the AirPlay 2 mirror protocol that made RPiPlay possible, by the AirplayServer author. All 
  code in `lib/` concerning mirroring is dsafa22's work. License: GNU LGPLv2.1+
* **Florian Draschbacher** (FD-) and contributors: adapted dsafa22's Android project for the Raspberry Pi, with extensive cleanups, debugging and improvements.  The project [RPiPlay](https://github.com/FD-/RPiPlay) is basically a port of dsafa22's code to the Raspberry Pi, utilizing OpenMAX and OpenSSL for better performance on the Pi.

# webserver implentation
For the website we use a forked version of [libhttpserver](https://github.com/etr/libhttpserver)

# Prometheus implementation

The prometheus imlementation is based on the [prometheus-cpp](https://github.com/jupp0r/prometheus-cpp) project. This is a unofficial project that is supported by prometheus themself.