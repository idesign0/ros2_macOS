\page install Installation

## Overview

These instructions are for installing only Gazebo Tools.
If you're interested in using all the Gazebo libraries, check out this [Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the Binary Installation instructions to get up and running as quickly and painlessly as possible.

The Source Installation instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

# Binary Installation

## Ubuntu Linux

Setup your computer to accept software from
*packages.osrfoundation.org*:
```
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
```

Setup keys:
```
sudo curl https://packages.osrfoundation.org/gazebo.gpg --output /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
```

You can check available version of libgz-tools using following command:
```bash
apt-cache search libgz-tools
```

Install Gazebo Tools:
```
sudo apt-get update
sudo apt-get install libgz-tools<#>-dev
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

### macOS

On macOS, add OSRF packages:
  ```
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  brew tap osrf/simulation
  ```

Install Gazebo Tools:
  ```
  brew install gz-tools<#>
  ```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
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
conda install libgz-tools<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

# Source Installation

Source installation can be performed by first installing the necessary
prerequisites followed by building from source.

## Prerequisites

Gazebo Tools requires:

* [Gazebo CMake](https://gazebosim.org/libs/cmake)

* [Ruby](https://www.ruby-lang.org/). Refer to the [Ruby Documentation](https://www.ruby-lang.org/downloads/) for installation instructions. On Ubuntu systems `apt-get` can be used to install Ubuntu Package `ruby-dev`:


### Ubuntu Linux

```
sudo apt-get install ruby-dev libgz-cmake<#>-dev
```

### Windows 10

First, follow the [gz-cmake](https://github.com/gazebosim/gz-cmake) tutorial for installing Conda, Visual Studio, CMake, and other prerequisites, and also for creating a Conda environment.

## Building from Source

### Ubuntu

1. Install tools
  ```
  sudo apt install -y build-essential cmake git gnupg lsb-release wget
  ```

2. Install dependencies
  ```
  sudo apt -y install \
    $(sort -u $(find . -iname 'packages-'`lsb_release -cs`'.apt' -o -iname 'packages.apt' | tr '\n' ' '))
  ```

3. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-tools -b gz-tools<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need.

4. Configure and build
  ```
  cd gz-tools
  mkdir build
  cd build
  cmake ..
  make
  ```

5. Optionally, install
  ```
  sudo make install
  ```

### macOS

1. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-tools -b gz-tools<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need.

2. Install dependencies
  ```
  brew install --only-dependencies gz-tools<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need.

3. Configure and build
  ```
  cd gz-tools
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

1. Navigate to `condabin` if necessary to use the `conda` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of `condabin` in Anaconda Prompt, `where conda`).
  Activate the Conda environment created in the prerequisites:
  ```
  conda activate gz-ws
  ```

2. Install dependencies

  You can view available versions and their dependencies:
  ```
  conda search libgz-tools* --channel conda-forge --info
  ```
  See the [Conda release repository](https://github.com/conda-forge/gz-tools2-feedstock) for more information.

  Install dependencies, replacing `<#>` with the desired version:
  ```
  conda install backward-cpp ruby libgz-cmake<#> --channel conda-forge
  ```

3. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b gz-tools#` (replace # with a number) to check out a specific version
  git clone https://github.com/gazebosim/gz-tools.git
  ```

4. Configure and build
  ```
  cd gz-tools
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

5. Optionally, install
  ```
  cmake --install . --config Release
  ```

# Documentation

API and tutorials can be found at [https://gazebosim.org/libs/tools](https://gazebosim.org/libs/tools).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using
 ```
 sudo apt-get install doxygen
 ```

2. Clone the repository
 ```
 git clone https://github.com/gazebosim/gz-tools
 ```

3. Configure and build the documentation.
 ```
 cd gz-tools; mkdir build; cd build; cmake ../; make doc
 ```

4. View the documentation by running the following command from the build directory.
 ```
 firefox doxygen/html/index.html
 ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](https://gazebosim.org/libs/tools#source-install).

2. Run tests.
 ```
 make test
 ```

3. Static code checker.
 ```
 make codecheck
 ```

