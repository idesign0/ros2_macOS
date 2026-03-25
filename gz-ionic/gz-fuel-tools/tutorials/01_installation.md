\page install Installation

Next Tutorial: \ref configuration

## Overview

Instructions to install Gazebo Fuel Tools on all the platforms supported.

## Binary Install

### Ubuntu Linux

Setup your computer to accept software from
*packages.osrfoundation.org*:
```
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
```

Setup keys:
```
wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
```

Install Gazebo Fuel Tools:
```
sudo apt-get update
sudo apt-get install libgz-fuel-tools<#>-dev
```

Be sure to replace `<#>` with a number value, such as 6 or 7, depending on
which version you need.

### Mac OS X

Gazebo Fuel Tools and several of its dependencies can be compiled on OS
X with [Homebrew](http://brew.sh/) using the [osrf/simulation
tap](https://github.com/osrf/homebrew-simulation). Gazebo Fuel Tools can
be installed on Mac OS X 10.11 (El Capitan) or higher.
Installation on older versions requires changing the default standard
library and rebuilding dependencies due to the use of c++11. For
purposes of this documentation, I will assume OS X 10.11 or greater is in use.
Here are the instructions:

Install Homebrew, which should also prompt you to install the XCode
command-line tools:
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Run the following commands:
```bash
brew tap osrf/simulation
brew install gz-fuel-tools<#>
```

### Windows

Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
Miniconda suffices.

Create if necessary, and activate a Conda environment:
```
conda create -n gz-ws
conda activate gz-ws
```

Install:

```
conda install libgz-fuel-tools<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 4 or 5, depending on
which version you need.

## Source Install

### Ubuntu Linux

For compiling the latest version of Gazebo Fuel Tools you will need an
Ubuntu distribution equal to 16.04 (Xenial) or newer.

Make sure you have removed the Ubuntu pre-compiled binaries before
installing from source:

```
sudo apt-get remove libgz-fuel-tools<#>-dev
```

Install prerequisites. A clean Ubuntu system will need:

```
sudo apt-get install git cmake pkg-config python ruby-ronn libgz-cmake4-dev libgz-common6-dev libgz-math8-dev libgz-msgs11-dev libgz-tools2-dev libzip-dev libjsoncpp-dev libcurl4-openssl-dev libyaml-dev
```

Clone the repository into a directory and go into it:

```
git clone https://github.com/gazebosim/gz-fuel-tools /tmp/gz-fuel-tools
cd /tmp/gz-fuel-tools
```

Create a build directory and go there:

```
mkdir build
cd build
```

Configure Gazebo Fuel Tools (choose either method a or b below):

* A.  Release mode: This will generate optimized code, but will not have debug symbols. Use this mode if you don't need to use GDB.
  ```
  cmake ../
  ```

  Note: You can use a custom install path to make it easier to switch
  between source and Debian installs:
  ```
  cmake -DCMAKE_INSTALL_PREFIX=/home/$USER/local ../
  ```

* B. Debug mode: This will generate code with debug symbols. Gazebo Fuel Tools will run slower, but you'll be able to use GDB.
  ```
  cmake -DCMAKE_BUILD_TYPE=Debug ../
  ```

The output from `cmake ../` may generate a number of errors and warnings
about missing packages. You must install the missing packages that have
errors and re-run `cmake ../`. Make sure all the build errors are
resolved before continuing (they should be there from the earlier step
in which you installed prerequisites).

Make note of your install path, which is output from cmake and should
look something like:
```
-- Install prefix: /home/$USER/local
```

Build Gazebo Fuel Tools:
```
make -j4
```

Install Gazebo Fuel Tools:
```
sudo make install
```

If you decide to install the library in a local directory you'll need to
modify your `LD_LIBRARY_PATH`:
```
echo "export LD_LIBRARY_PATH=<install_path>/local/lib:$LD_LIBRARY_PATH" >> ~/.bashrc
```

#### Uninstalling Source Install

If you need to uninstall Gazebo Fuel Tools or switch back to a
Debian-based install when you currently have installed the library from
source, navigate to your source code directory's build folders and run
`make uninstall`:
```
cd /tmp/gz-fuel-tools/build
sudo make uninstall
```

### macOS

1. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-fuel-tools -b gz-fuel-tools<#>
  ```
  Be sure to replace `<#>` with a number value, such as 8 or 9, depending on
  which version you need. 
  
  **Note:** If the version is 7 or downwards replace `gz-fuel-tools<#>` for `ign-fuel-tools<#>` in the command line. For more information take a look [here](https://community.gazebosim.org/t/a-new-era-for-gazebo/1356).

2. Install dependencies
  ```
  brew install --only-dependencies gz-fuel-tools<#>
  ```
  Be sure to replace `<#>` with a number value, such as 8 or 9, depending on
  which version you need.

3. Configure and build
  ```
  cd gz-fuel-tools
  mkdir build
  cd build
  cmake ..
  make
  ```

### Windows

#### Prerequisites

First, follow the [gz-cmake](https://github.com/gazebosim/gz-cmake) tutorial for installing Conda, Visual Studio, CMake, etc., prerequisites, and creating a Conda environment.

Navigate to ``condabin`` if necessary to use the ``conda`` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of ``condabin`` in Anaconda Prompt, ``where conda``).

Create if necessary, and activate a Conda environment:
```
conda create -n gz-ws
conda activate gz-ws
```

Install dependencies:
```
conda install jsoncpp libzip --channel conda-forge
```

Install Gazebo dependencies:

You can view available versions and their dependencies:
```
conda search libgz-fuel-tools* --channel conda-forge --info
```

Install Gazebo dependencies, replacing `<#>` with the desired versions:
```
conda install libgz-cmake<#> libgz-common<#> libgz-msgs<#> libgz-tools<#> --channel conda-forge
```

#### Building from source

1. Activate the Conda environment created in the prerequisites:
  ```
  conda activate gz-ws
  ```

2. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b gz-fuel-tools#` (replace # with a number) to check out a specific version
  git clone https://github.com/gazebosim/gz-fuel-tools.git
  ```

3. Configure and build
  ```
  cd gz-fuel-tools
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

4. Optionally, install. You will likely need to run a terminal with admin privileges for this call to succeed.
  ```
  cmake --install . --config Release
  ```
