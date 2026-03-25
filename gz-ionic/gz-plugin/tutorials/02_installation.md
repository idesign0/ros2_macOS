\page installation Installation

These instructions are for installing only Gazebo Plugin.
If you're interested in using all the Gazebo libraries, check out this [Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the Binary Installation instructions to get up and running as quickly and painlessly as possible.

The Source Installation instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

# Ubuntu

## Prerequisites

Ubuntu Focal or higher.

If you don't already have the `lsb-release` package installed, please do so now:
```bash
sudo apt-get update
sudo apt-get install lsb-release
```

Setup your computer to accept software from
[packages.osrfoundation.org](http://packages.osrfoundation.org):
```bash
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
```

Setup keys:
```bash
wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
```

## Binary Installation

On Ubuntu systems, `apt-get` can be used to install `gz-plugin`:
```bash
sudo apt-get update
sudo apt install libgz-plugin3-dev
```

## Source Installation

1. Install Gazebo dependencies
  ```
  sudo apt-get install libgz-cmake4-dev libgz-tools2-dev libgz-utils3-cli-dev
  ```

1. Install Gazebo Tools if you want to use the `gz plugin` command line tool:
  ```bash
  sudo apt-get install gz-tools2
  ```

2. Clone the repository
  ```bash
  # Optionally, append `-b gz-plugin#` (replace # with a number) to check out a specific version
  # From version 2 use `gz-plugin#`, lower versions `ign-plugin#`
  git clone https://github.com/gazebosim/gz-plugin
  ```

3. Configure and build
  ```bash
  cd gz-plugin
  mkdir build
  cd build
  cmake ..
  make
  ```

4. Optionally, install
  ```bash
  sudo make install
  ```

# macOS

## Prerequisites

Gazebo Plugin and several of its dependencies can be installed on macOS
with [Homebrew](http://brew.sh/) using the [osrf/simulation
tap](https://github.com/osrf/homebrew-simulation). Gazebo Plugin versions 1+
require macOS High Sierra (10.13) or later, while version 0 supports
Sierra (10.12) or later.

## Binary Installation

1. Install Homebrew, which should also prompt you to install the XCode command-line tools
  ```bash
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  ```

2. Run the following commands
  ```bash
  brew tap osrf/simulation
  brew install gz-plugin3
  ```

## Source Installation

1. Install dependencies
  ```bash
  brew tap osrf/simulation
  brew install gz-plugin3 --only-dependencies
  ```

2. Clone the repository
  ```bash
  # Optionally, append `-b gz-plugin#` (replace # with a number) to check out a specific version
  # From version 2 use `gz-plugin#`, lower versions `ign-plugin#`
  git clone https://github.com/gazebosim/gz-plugin
  ```

3. Configure and build
  ```bash
  cd gz-plugin
  mkdir build
  cd build
  cmake ..
  make
  ```

4. Optionally, install
  ```bash
  sudo make install
  ```

# Windows

## Prerequisites

First, follow the [gz-cmake](https://github.com/gazebosim/gz-cmake) tutorial for installing Conda, Visual Studio, CMake, and other prerequisites, and also for creating a Conda environment.

Navigate to `condabin` if necessary to use the `conda` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of `condabin` in Anaconda Prompt, `where conda`).

Create if necessary, and activate a Conda environment:
```bash
conda create -n gz-ws
conda activate gz-ws
```

## Binary Installation

```bash
conda install libgz-plugin<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

## Source Installation

This assumes you have created and activated a Conda environment while installing the Prerequisites.

1. Install Gazebo dependencies:

  You can view available versions and their dependencies:
  ```bash
  conda search libgz-plugin* --channel conda-forge --info
  ```

  Install dependencies, replacing `<#>` with the desired version:
  ```bash
  conda install libgz-cmake<#> --channel conda-forge
  ```

2. Navigate to where you would like to build the library, and clone the repository.
  ```bash
  # Optionally, append `-b gz-plugin#` (replace # with a number) to check out a specific version
  # From version 2 use `gz-plugin#`, lower versions `ign-plugin#`
  git clone https://github.com/gazebosim/gz-plugin
  ```

3. Configure and build
  ```bash
  cd gz-plugin
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

4. Optionally, install
  ```bash
  cmake --install . --config Release
  ```

# Documentation

API documentation and tutorials can be accessed at
[Gazebo website](https://gazebosim.org/libs/plugin)

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need [Doxygen](http://www.doxygen.org/). On Ubuntu Doxygen can be installed using
  ```bash
  sudo apt-get install doxygen
  ```

2. Clone the repository
  ```bash
  # Optionally, append `-b gz-plugin#` (replace # with a number) to check out a specific version
  # From version 2 use `gz-plugin#`, lower versions `ign-plugin#`
  git clone https://github.com/gazebosim/gz-plugin
  ```

3. Configure and build the documentation.
  ```bash
  cd gz-plugin
  mkdir build
  cd build
  cmake ..
  make doc
  ```

4. View the documentation by running the following command from the `build` directory.
  ```bash
  firefox doxygen/html/index.html
  ```

**Note** Alternatively, documentation for `gz-plugin` can be found within the source code, and also in the MIGRATION.md guide.


# Test

Run tests as follows:
```bash
make test
```

Tests are automatically built. To disable them, run `cmake` as follows:
```bash
cmake .. -DBUILD_TESTING=false
```

## Test coverage

To run test coverage:

1. Install LCOV

  ```bash
  sudo apt-get install lcov
  ```

2. Build with coverage

  ```bash
  cd build/
  cmake .. -DCMAKE_BUILD_TYPE=coverage
  make
  ```

3. Run tests
  ```bash
  make test
  ```

4. Generate coverage
  ```bash
  make coverage
  ```

5. View results
  ```bash
  firefox coverage/index.html
  ```
