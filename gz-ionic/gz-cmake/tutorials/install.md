\page install Installation

These instructions are for installing only Gazebo CMake. If you're interested
in using all the Gazebo libraries, not only Gazebo CMake, check out this
[Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the binary install instructions to get up and running as
quickly and painlessly as possible.

The source install instructions should be used if you need the very latest
software improvements, if you need to modify the code, or if you plan to make a
contribution.

# Binary Install

## Ubuntu

On Ubuntu, it's possible to install Gazebo CMake as follows:

Add OSRF packages:
  ```
  echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list
  sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys D2486D2DD83DB69272AFE98867170598AF249743
  sudo apt update
  ```

Install Gazebo CMake:
  ```
  sudo apt install libgz-cmake<#>-dev
  ```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

## macOS

On macOS, add OSRF packages:
  ```
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  brew tap osrf/simulation
  ```

Install Gazebo CMake:
  ```
  brew install gz-cmake<#>
  ```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

## Windows

Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
Miniconda suffices.

Open a Visual Studio Command Prompt (search for "x64 Native Tools Command Prompt for VS 2019" in the Windows search near the Start button).
Optionally, right-click and pin to the task bar for quick access in the future.

If you did not add Conda to your `PATH` environment variable during Conda installation, you may need to navigate to the location of `condabin` in order to use the `conda` command.
To find `condabin`, search for "Anaconda Prompt" in the Windows search near the Start button, open it, run `where conda`, and look for a line containing the directory `condabin`.

1. Navigate to your `condabin` if necessary, and then create and activate a Conda environment:
  ```
  conda create -n gz-ws
  conda activate gz-ws
  ```

  Once you have activated an environment, a prefix like `(gz-ws)` will be prepended to your prompt, and you can use the `conda` command outside of `condabin`.

  You can use `conda info --envs` to see all your environments.

  To remove an environment, use `conda env remove --name <env_name>`.

2. Install Gazebo CMake:
  ```
  conda install libgz-cmake<#> --channel conda-forge
  ```

  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on which version you need.

  You can view all the versions with
  ```
  conda search libgz-cmake* --channel conda-forge
  ```

  and view their dependencies with
  ```
  conda search libgz-cmake* --channel conda-forge --info
  ```

  and install a specific minor version with
  ```
  conda install libgz-cmake=2.6.1 --channel conda-forge
  ```

# Source Install

## Ubuntu Focal 20.04 or above

### Prerequisites

Add OSRF packages:
```
sudo apt update
sudo apt -y install wget lsb-release gnupg
sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
wget http://packages.osrfoundation.org/gazebo.key -O - | apt-key add -
sudo apt-add-repository -s "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -c -s) main"
```

### Building from source

Clone source code:

```
# This checks out the `main` branch. You can append `-b gz-cmake#` (replace # with a number) to checkout a specific version
git clone http://github.com/gazebosim/gz-cmake
```

Install dependencies
```
sudo apt -y install $(sort -u $(find . -iname 'packages.apt') | tr '\n' ' ')
```

Build and install as follows:
```
cd gz-cmake
mkdir build
cd build
cmake ..
make -j4
sudo make install
```

## Windows 10

### Prerequisites

1. Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
   Miniconda suffices.

2. Install [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/).
   The Community version is free for students, open-source contributors, and individuals.
   Check "Desktop development with C++" in the "Workloads" tab, and uncheck "C++ CMake Tools". We will install `cmake` via Conda.

### Building from source

Open a Visual Studio Command Prompt (search for "x64 Native Tools Command Prompt for VS 2019" in the search field near the Windows button).
Optionally, right-click and pin to the task bar for quick access in the future.

If you did not add Conda to your `PATH` environment variable during Conda installation, you may need to navigate to the location of `condabin` in order to use the `conda` command.
To find `condabin`, search for "Anaconda Prompt" in the search field near the Windows button, open it, run `where conda`, and look for a line containing the directory `condabin`.

1. Navigate to your `condabin` if necessary, and then create and activate a Conda environment:
  ```
  conda create -n gz-ws
  conda activate gz-ws
  ```

   Once you have activated an environment, a prefix like `(gz-ws)` will be prepended to your prompt, and you can use the `conda` command outside of `condabin`.

   You can use `conda info --envs` to see all your environments.

   To remove an environment, use `conda env remove --name <env_name>`.

2. Install dependencies
  ```
  conda install git cmake pkg-config --channel conda-forge
  ```

3. Navigate to where you would like to build the library, and then clone the repository.
  ```
  # Optionally, append `-b gz-cmake#` (replace # with a number) to check out a specific version
  git clone https://github.com/gazebosim/gz-cmake.git
  ```

4. Build.
  ```
  cd gz-cmake
  mkdir build
  cd build
  cmake .. # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  cmake --install . --config Release
  ```

**Note** If you find that the build is failing due to failures in the `test` directory, then you may need to disable tests by adding `-DBUILD_TESTING=OFF` to the `cmake ..` command.

# Documentation

API documentation and tutorials can be accessed at
[https://gazebosim.org/libs/cmake](https://gazebosim.org/libs/cmake)

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need [Doxygen](http://www.doxygen.org/). On Ubuntu Doxygen can be installed using
  ```
  sudo apt-get install doxygen
  ```

2. Clone the repository

  ```
  git clone https://github.com/gazebosim/gz-cmake
  ```

3. Configure and build the documentation.
  ```
  cd gz-cmake
  mkdir build
  cd build
  cmake ..
  make doc
  ```

4. View the documentation by running the following command from the `build` directory.
  ```
  firefox doxygen/html/index.html
  ```

**Note** Alternatively, documentation for `gz-cmake` can be found within the source code, and also in the [MIGRATION.md guide](https://github.com/gazebosim/gz-cmake/blob/main/MIGRATION.md).

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

Additionally, a fuller suite of tests in the `examples` directory can be enabled by building with `BUILDSYSTEM_TESTING` enabled.
Tests can be run by building the `test` target. From your build directory you can run:

```
$ cmake .. -DBUILDSYSTEM_TESTING=1
$ make test
```


See the [Writing Tests section of the gz-cmake contributor documentation](https://gazebosim.org/docs/all/contributing#writing-tests) for help creating or modifying tests.
