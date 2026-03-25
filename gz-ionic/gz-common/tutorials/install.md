\page install Installation

Next Tutorial: \ref profiler

These instructions are for installing only Gazebo Common.
If you're interested in using all the Gazebo libraries, check out this [Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the Binary Installation instructions to get up and running as quickly and painlessly as possible.

The Source Installation instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

# Binary Installation

## Ubuntu

On Ubuntu systems, `apt-get` can be used to install `gz-common`:
```
sudo apt install libgz-common<#>-dev
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

### macOS

On macOS, add OSRF packages:
  ```
  /bin/bash", "-c", '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  brew tap osrf/simulation
  ```

Install Gazebo Common:
  ```
  brew install gz-common<#>
  ```

Be sure to replace `<#>` with a number value, such as 3 or 4, depending on
which version you need.

## Windows

Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
Miniconda suffices.

Create if necessary, and activate a Conda environment:
```
conda create -n gz-ws
conda activate gz-ws
```

Install `gz-common`:
```
conda install libgz-common<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

# Source Installation

Source installation can be performed by first installing the necessary
prerequisites followed by building from source.

## Prerequisites

Gazebo Common requires:

  * [Gazebo CMake](https://gazebosim.org/libs/cmake)
  * [Gazebo Math](https://gazebosim.org/libs/math).

The Graphics component requires:

  * [FreeImage](http://freeimage.sourceforge.net/)

The AV component requires:

  * [libswscale](https://www.ffmpeg.org/libswscale.html)
  * [libavdevice](https://www.ffmpeg.org/libavdevice.html)
  * [libavformat](https://www.ffmpeg.org/libavformat.html)
  * [libavcodec](https://www.ffmpeg.org/libavcodec.html)
  * [libavutil](https://www.ffmpeg.org/libavutil.html)

The Geospatial component requires:

  * [gdal](https://gdal.org/)

### Windows

First, follow the [gz-cmake](https://github.com/gazebosim/gz-cmake) tutorial for installing Conda, Visual Studio, CMake, etc., prerequisites, and creating a Conda environment.

Navigate to `condabin` if necessary to use the `conda` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of `condabin` in Anaconda Prompt, `where conda`).

Activate the Conda environment:
```
conda activate gz-ws
```

Install prerequisites:
```
conda install freeimage gdal glib dlfcn-win32 ffmpeg --channel conda-forge
```

Install Gazebo dependencies:

You can view available versions and their dependencies:
```
conda search libgz-common* --channel conda-forge --info
```

Install dependencies, replacing `<#>` with the desired versions:
```
conda install libgz-cmake<#> libgz-math<#> --channel conda-forge
```

## Build from Source

### Ubuntu

1. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-common
  ```

2. Configure and build
  ```
  cd gz-common
  mkdir build
  cd build
  cmake ..
  make
  ```

3. Optionally, install
  ```
  sudo make install
  ```

### macOS

1. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-common -b gz-common<#>
  ```
  Be sure to replace `<#>` with a number value, such as 3 or 4, depending on
  which version you need. From version 5 use `gz-common<#>` for lower versions
  use `ign-common<#>`.

2. Install dependencies
  ```
  brew install --only-dependencies gz-common<#>
  ```
  Be sure to replace `<#>` with a number value, such as 3 or 4, depending on
  which version you need.

3. Configure and build
  ```
  cd gz-common
  mkdir build
  cd build
  cmake ..
  make
  ```

4. Optionally, install
  ```
  sudo make install
  ```

### Windows

This assumes you have created and activated a Conda environment while installing the Prerequisites.

1. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b gz-common#` (replace # with a number) to check out a specific version.
  # From version 5 use `gz-common<#>` for lower versions use `ign-common<#>`.
  git clone https://github.com/gazebosim/gz-common.git
  ```

2. Configure and build
  ```
  cd gz-common
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

3. Optionally, install
  ```
  cmake --install . --config Release
  ```

# Documentation

API and tutorials can be found at [https://gazebosim.org/libs/common](https://gazebosim.org/libs/common).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using
  ```
  sudo apt-get install doxygen
  ```

2. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-common
  ```

3. Configure and build the documentation.
  ```
  cd gz-common
  mkdir build
  cd build
  cmake ../
  make doc
  ```

4. View the documentation by running the following command from the build directory.
  ```
  firefox doxygen/html/index.html
  ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](#source-install).

2. Run tests.
  ```
  make test
  ```

3. Static code checker.
  ```
  make codecheck
  ```
