\page installation Installation

These instructions are for installing only Gazebo Rendering.
If you're interested in using all the Gazebo libraries, check out this [Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the Binary Install instructions to get up and running as quickly and painlessly as possible.

The Source Install instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

# Ubuntu

## Binary Installation

Install dependencies:
```
sudo apt-get update
sudo apt-get -y install wget lsb-release gnupg
```

Setup your computer to accept software from
[packages.osrfoundation.org](http://packages.osrfoundation.org):
```
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
```

Setup keys:
```
wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
```

Install:
```
sudo apt-get update
sudo apt-get install libgz-rendering<#>-dev
```

Be sure to replace `<#>` with a number value, such as `7` or `8`, depending on which version you need.

## Source Installation

### Prerequisites

Ubuntu Focal 20.04 or above:

Install dependencies:
```
sudo apt -y install wget lsb-release gnupg
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
sudo apt update
sudo apt install -y \
    cmake \
    pkg-config \
    git \
    libglew-dev  \
    libfreeimage-dev \
    freeglut3-dev \
    libxmu-dev \
    libxi-dev \
    libgz-cmake4-dev \
    libgz-math8-dev \
    libgz-common6-dev \
    libgz-plugin3-dev
```

### Supported Rendering Engines

Gazebo Rendering will look for rendering libraries installed in the system and
build the relevant plugins if dependencies are found.

**OGRE 1.x**
```
# this installs ogre 1.9
sudo apt-get install libogre-1.9-dev
```

**OGRE-Next 2.x**

Add OSRF packages if you have not done so already:
```
sudo apt -y install wget lsb-release gnupg
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
sudo apt update
```

Install OGRE-Next 2.3 debs
```
sudo apt install libogre-next-dev
```

**OptiX (experimental)**

Download and install by following instructions on NVIDIA website

CUDA: http://docs.nvidia.com/cuda

OptiX: https://developer.nvidia.com/optix

Update `LD_LIBRARY_PATH` and add an `OPTIX_INSTALL_DIR` environment variables
so that gz-rendering can find Optix, e.g. if you installed version 4.0.2 in HOME/optix:
```
export LD_LIBRARY_PATH=${HOME}/optix/NVIDIA-OptiX-SDK-4.0.2-linux64/lib64:${LD_LIBRARY_PATH}
export OPTIX_INSTALL_DIR=${HOME}/optix/NVIDIA-OptiX-SDK-4.0.2-linux64
```

Note: If you encounter errors about different exception specifiers in optix math
when building Gazebo Rendering OptiX plugin, edit
`[optix_install_dir]/include/optixu/optixu_math_namespace.h` and comment
out the section that defines `fminf`, fmaxf, and `copysignf` (for optix
sdk 4.0.2, comment out lines 167-206).

### Build from Source

1. Clone the repository
  ```
  # Optionally, append `-b gz-rendering#` (replace # with a number) to check out a specific version
  git clone http://github.com/gazebosim/gz-rendering
  ```

2. Configure and build
  ```
  cd gz-rendering
  mkdir build
  cd build
  cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install/dir
  make
  ```
  Replace `/path/to/install/dir` to whatever directory you want to install this package to

3. Optionally, install
  ```
  make install
  ```

# Windows

## Install Prerequisites

First, follow the [source installation](https://gazebosim.org/docs/ionic/install_windows_src/) tutorial until step 5 included for installing Conda, Visual Studio, CMake, etc., prerequisites, and creating a Conda environment.

Navigate to ``condabin`` if necessary to use the ``conda`` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of ``condabin`` in Anaconda Prompt, ``where conda``).

Create if necessary, and activate a Conda environment:
```
conda create -n gz-ws
conda activate gz-ws
```

## Binary Installation

```bash
conda install libgz-rendering<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 7 or 8, depending on
which version you need.

## Source Installation

This assumes you have created and activated a Conda environment while [installing the Prerequisites](#install-prerequisites).

1. Install Gazebo dependencies:

  You can view available versions and their dependencies:
  ```
  conda search libgz-rendering* --channel conda-forge --info
  ```

  Install dependencies, replacing `<#>` with the desired versions:
  ```
  conda install libgz-cmake<#> libgz-common<#> libgz-math<#> libgz-plugin<#> --channel conda-forge
  ```

2. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b gz-rendering#` (replace # with a number) to check out a specific version
  git clone https://github.com/gazebosim/gz-rendering.git
  ```

3. Configure and build
  ```
  cd gz-rendering
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

4. Optionally, install
  ```
  cmake --install . --config Release
  ```

# macOS

## Binary Installation

Install [Homebrew](https://brew.sh/).

Add OSRF packages:

  ```
  brew tap osrf/simulation
  ```

Install Gazebo Rendering:
  ```
  brew install gz-rendering<#>
  ```

Be sure to replace `<#>` with a number value, such as 8 or 9, depending on
which version you need.

## Source Installation

1. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-rendering -b gz-rendering<#>
  ```
  Be sure to replace `<#>` with a number value, such as 7 or 8, depending on
  which version you need.

2. Install dependencies
  ```
  brew install --only-dependencies gz-rendering<#>
  ```
  Be sure to replace `<#>` with a number value, such as 7 or 8, depending on
  which version you need.

3. Configure and build
  ```
  cd gz-rendering
  mkdir build
  cd build
  cmake ..
  make
  ```

4. Optionally, install
  ```
  sudo make install
  ```

# Documentation

API documentation can be generated using Doxygen
  ```
  sudo apt install -y doxygen
  ```

Build documentation
  ```
  cd build
  make doc
  ```

View documentation
  ```
  firefox doxygen/html/index.html
  ```

# Testing

Tests can be run by building the `test` target:
  ```
  cd build
  make test
  ```

Most tests can be run against multiple render engine configurations (if available).
To control the testing configuration, use the following environment variables:

  ```
  # Specify the rendering engine to use (ogre, ogre2, optix)
  GZ_ENGINE_TO_TEST=ogre2

  # Specify the ogre2 backend to use (vulkan, gl3plus, metal (macOS))
  GZ_ENGINE_BACKEND=vulkan

  # Specify if using "headless mode" (EGL or vulkan NULL window)
  GZ_ENGINE_HEADLESS=1
  ```

A full invocation of a test would be
  ```
  GZ_ENGINE_TO_TEST=ogre2 GZ_ENGINE_BACKEND=gl3plus ./bin/UNIT_Camera_TEST
  ```

Additionally, each test is registered with `ctest` for each engine/backend configuration
available at build time.  These can then be filtered with the `ctest` command line.

  ```
  # See a list of all available tests
  ctest -N

  # Run all ogre2 tests (verbose)
  ctest -R ogre2 -V

  # Run all ogre2/vulkan tests (verbose)
  ctest -R ogre2_vulkan -V

  # Run all OpenGL tests (verbose)
  ctest -R gl3plus -V

  # Run all INTEGRATION tests (verbose)
  ctest -R INTEGRATION -V
  ```
