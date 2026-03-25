\page installation Installation

Next Tutorial: \ref nodestopics
Previous Tutorial: \ref introduction

## Overview

This page contains instructions to install Gazebo Transport on all the
support platforms including major Linux distributions, Mac OS X and Windows.

You can find all Gazebo Transport versions at [https://gazebosim.org/libs/transport](https://gazebosim.org/libs/transport).

# Binary Install

## Ubuntu

Setup your computer to accept software from
*packages.osrfoundation.org*:
```
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
```

Setup keys:
```
wget https://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
```

Install Gazebo Transport:
```
sudo apt-get update
sudo apt-get install libgz-transport<#>-dev
```

Be sure to replace `<#>` with a number value, such as `8` or `9`, depending on
which version you need.

## Mac OS X

Gazebo Transport and several of its dependencies can be compiled on OS
X with [Homebrew](http://brew.sh/) using the [osrf/simulation
tap](https://github.com/osrf/homebrew-simulation). Gazebo Transport is
straightforward to install on Mac OS X 10.9 (Mavericks) or higher.
Installation on older versions requires changing the default standard
library and rebuilding dependencies due to the use of c++11. For
purposes of this documentation, assuming OS X 10.9 or greater is in
use. Here are the instructions:

After installing the [Homebrew package manager](https://brew.sh),
which should also prompt you to install the XCode command-line tools
add OSRF packages and run the install command:

```
brew tap osrf/simulation
brew install gz-transport<#>
```

Be sure to replace `<#>` with a number value, such as `8` or `9`, depending on
which version you need.

## Windows

Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
Miniconda suffices.

Create if necessary, and activate a Conda environment:
```
conda create -n gz-ws
conda activate gz-ws
```

Install:
```
conda install libgz-transport<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

# Source Install

## Install Prerequisites

### Ubuntu Linux

For compiling the latest version of Gazebo Transport you will need an
Ubuntu distribution equal to 24.04 (Noble) or newer.

Make sure you have removed the Ubuntu pre-compiled binaries before
installing from source:
```
sudo apt-get remove libgz-transport.*-dev
```

Install prerequisites. A clean Ubuntu system will need:
```
sudo apt-get install git cmake pkg-config python ruby-ronn libprotoc-dev libprotobuf-dev protobuf-compiler uuid-dev libzmq3-dev libgz-msgs11-dev libgz-utils3-cli-dev
```

### macOS

After installing the [Homebrew package manager](https://brew.sh),
which should also prompt you to install the XCode command-line tools
add OSRF packages and run the command to install dependencies:

```
brew tap osrf/simulation
brew install --only-dependencies gz-transport<#>
```

Be sure to replace `<#>` with a number value, such as 10 or 11, depending on
which version you need.

## Clone, Configure, and Build

Clone the repository
```
git clone https://github.com/gazebosim/gz-transport -b gz-transport<#>
```
Be sure to replace `<#>` with a number value, such as 10 or 11, depending on
which version you need. From version 12 use `gz-transport<#>` for lower versions
use `ign-transport<#>`

Configure and build
```
cd gz-transport
mkdir build
cd build
cmake ..
make
```

Optionally, install
```
sudo make install
```

### Configuration options

Configure Gazebo Transport (choose either method a or b below):

A.  Release mode (recommended): This will generate optimized code, but will not have
    debug symbols. Use this mode if you don't need to use [GDB](https://www.gnu.org/software/gdb/) (advanced).
```
cmake ..
```

Note: You can use a custom install path to make it easier to switch
between source and debian installs:
```
cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/local ..
```

B. Debug mode: This will generate code with debug symbols. Gazebo
Transport will run slower, but you'll be able to use GDB.
```
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

The output from `cmake ../` may generate a number of errors and warnings
about missing packages. You must install the missing packages that have
errors and re-run `cmake ../`. Make sure all the build errors are
resolved before continuing (they should be there from the earlier step
in which you installed prerequisites).

Make note of your install path, which is output from cmake and should
look something like:
```
-- Install path: /home/$USER/local
```

Build Gazebo Transport:
```
make -j4
```

Install Gazebo Transport:
```
sudo make install
```

If you decide to install gazebo in a local directory you'll need to
modify your `LD_LIBRARY_PATH`:
```
echo "export LD_LIBRARY_PATH=<install_path>/local/lib:$LD_LIBRARY_PATH" >> ~/.bashrc
```

### Build python bindings separately from main library

If you want to build Python bindings separately from the main gz-transport library
(for example if you want to build Python bindings for multiple versions of Python),
you can invoke cmake on the `python` folder instead of the root folder.
Specify the path to the python executable with which you wish to build bindings
in the `Python3_EXECUTABLE` cmake variable.
Specify the install path for the bindings in the `CMAKE_INSTALL_PREFIX`
variable, and be sure to set your `PYTHONPATH` accordingly after install.

```bash
cd sdformat
mkdir build_python3
cd build_python3
cmake ../python \
    -DPython3_EXECUTABLE=/usr/local/bin/python3.12 \
    -DCMAKE_INSTALL_PREFIX=<prefix>
```

See the homebrew [sdformat15 formula](https://github.com/osrf/homebrew-simulation/blob/027d06f5be49da1e40d01180aedae7f76dc7ff47/Formula/sdformat15.rb#L12-L56)
for an example of building bindings for multiple versions of Python.

### Uninstalling Source-based Install

If you need to uninstall Gazebo Transport or switch back to a
debian-based install when you currently have installed the library from
source, navigate to your source code directory's build folders and run
`make uninstall`:
```
cd /tmp/gz-transport/build
sudo make uninstall
```

## Windows

### Prerequisites

First, follow the [source installation](https://gazebosim.org/docs/ionic/install_windows_src/) tutorial until step 5 included for installing Conda, Visual Studio, CMake, etc., prerequisites, and creating a Conda environment.

Navigate to `condabin` if necessary to use the `conda` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of `condabin` in Anaconda Prompt, `where conda`).

Activate the Conda environment:
```
conda activate gz-ws
```

Install prerequisites:
```
conda install zeromq cppzmq --channel conda-forge
```

Install Gazebo dependencies:

You can view available versions and their dependencies:
```
conda search libgz-transport* --channel conda-forge --info
```

Install dependencies, replacing `<#>` with the desired versions:
```
conda install libgz-cmake<#> libgz-msgs<#> libgz-tools<#> --channel conda-forge
```

#### Building from Source

1. Navigate to where you would like to build the library, and clone the repository.
  ```bash
  # Optionally, append `-b gz-transport#` (replace # with a number) to check out a specific version.
  From version 12 use `gz-transport<#>` for lower versions use `ign-transport<#>`
  git clone https://github.com/gazebosim/gz-transport.git
  ```

2. Configure and build
  ```bash
  cd gz-transport
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

3. Optionally, install. You will likely need to run a terminal with admin privileges for this call to succeed.
  ```bash
  cmake --install . --config Release
  ```

4. Optionally, build the examples

  If you installed to a custom location, you may need to specify ``-DCMAKE_PREFIX_PATH``, pointing to the directory containing the file ``gz-transport<#>-config.cmake``.
  That file is installed to the ``CMAKE_INSTALL_PREFIX``, for example, ``path\to\install\gz-transport<#>\lib\cmake\gz-transport<#>``.
  ```bash
  cd gz-transport\example
  mkdir build
  cd build
  cmake ..  # Optionally, -DCMAKE_PREFIX_PATH=path\to\cmake\config
  cmake --build . --config Release
  ```

  Try an example (in the Release directory)
  ```bash
  responser
  ```

  In another terminal, run
  ```bash
  requester
  ```

# Documentation

Visit the [documentation page](https://gazebosim.org/api/transport/14/index.html).

## Build documentation
```bash
cd build
make doc
```

View the documentation by running the following command from the `build` directory.
```
firefox doxygen/html/index.html
```

# Testing

Tests can be run by building the `test` target. From your build directory you
can run:
```bash
make test
```

*Note:* Python tests require running `sudo make install` before running `make test`.
