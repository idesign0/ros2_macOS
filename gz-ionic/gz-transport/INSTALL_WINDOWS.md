# Installation on Windows

## Supported compilers

At this moment, compilation has been tested on Windows 7&10, and is supported
when using Visual Studio 2013. Patches for other versions are welcome.

## Installation

Using a combination of pre-compiled binaries and self-compiled dependencies in a
local workspace. To make things easier, use a MinGW shell for your editing work,
and only use the Windows `cmd` for configuring and building.  You might also
need to [disable the Windows firewall](http://windows.microsoft.com/en-us/windows/turn-windows-firewall-on-off#turn-windows-firewall-on-off=windows-7).

1. Install cmake, make sure to select the "Add CMake to system path for all users" option in the install dialog box.

    [CMake](https://cmake.org/download)

1. Make a directory to work in, e.g.:

        mkdir gz-ws
        cd gz-ws

1. Download the following dependencies into that directory:

    - [cppzmq](http://packages.osrfoundation.org/win32/deps/cppzmq-noarch.zip)
    - [Protobuf 2.6.0 (32-bit)](http://packages.osrfoundation.org/win32/deps/protobuf-2.6.0-win32-vc12.zip)
    - [Protobuf 2.6.0 (64-bit)](http://packages.osrfoundation.org/win32/deps/protobuf-2.6.0-win64-vc12.zip)

    Choose one of these options:

    - [ZeroMQ 4.0.4 (32-bit)](http://packages.osrfoundation.org/win32/deps/zeromq-4.0.4-x86.zip)
    - [ZeroMQ 4.0.4 (64-bit)](http://packages.osrfoundation.org/win32/deps/zeromq-4.0.4-amd64.zip)

1. Unzip each of them. The Windows unzip utility will likely create an incorrect
directory structure, where a directory with the name of the zip contains the
directory that has the source files. Here is an example:

        gz-ws/cppzmq-noarch/cppzmq

    The correct structure is

        gz-ws/cppzmq

    To fix this problem, manually move the nested directories up one level.

1. Clone and prepare the Gazebo Math dependency:

        git clone https://github.com/gazebosim/gz-math
        cd gz-math
        mkdir build

1. In a Windows Command Prompt, load your compiler setup, e.g.:

        "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64

1. In the Windows Command Promp, configure and build:

        cd gz-math\build
        ..\configure
        nmake install

1. Clone and prepare the Gazebo Msgs dependency:

        git clone https://github.com/gazebosim/gz-msgs
        cd gz-msgs
        mkdir build

1. In the Windows Command Prompt, configure and build:

        cd gz-msgs\build
        ..\configure
        nmake install

1. Clone gz-transport:

        git clone https://github.com/gazebosim/gz-transport
        cd gz-transport
        mkdir build

1. In the Windows Command Prompt, configure and build:

	    cd gz-transport\build
        ..\configure
        nmake install


    You should now have an installation of gz-transport in gz-ws/gz-transport/build/install.

1. Before running any executables, you need to modify your `PATH` to include the
directories including your DLL dependencies to let Windows find dynamic libs
(similar to `LD_LIBRARY_PATH` on Linux). Don't put quotes around the path, even
if it contains spaces.  E.g., if you're working in `C:\My Stuff\gz-ws`:

        set PATH=%PATH%;C:\My Stuff\gz-ws\ZeroMQ 4.0.4\bin;C:\My Stuff\gz-ws\gz-msgs\build\install\Release\lib

1. Now build the examples:

        cd gz-ws\gz-transport\example
        mkdir build
        cd build
        ..\configure
        nmake

1. Now try an example:

    1. In one Windows terminal run:

        responser

    1. In another Windows terminal run:

        requester


## Run tests

1. In the Windows Command Prompt, run the test suite:

    cd gz-transport\build
    ctest
