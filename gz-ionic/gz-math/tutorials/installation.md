\page install Installation

Next Tutorial: \ref cppgetstarted

These instructions are for installing only Gazebo Math.
If you're interested in using all the Gazebo libraries, check out this [Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the Binary Installation instructions to get up and running as quickly and painlessly as possible.

The Source Installation instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

# Binary Installation

## Ubuntu Linux
First install some necessary tools:

```bash
sudo apt-get update
sudo apt-get install lsb-release wget gnupg
```

Then, setup your computer to accept software from
*packages.osrfoundation.org*:
```bash
sudo wget https://packages.osrfoundation.org/gazebo.gpg -O /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
sudo apt-get update
```

Install Gazebo Math:
```
sudo apt install libgz-math<#>-dev
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need. From version 7 you should use `gz-math<#>` but for lower versions
you should use `ign-math<#>`.

### macOS

On macOS, after installing the [Homebrew package manager](https://brew.sh),
add OSRF packages:
  ```
  brew tap osrf/simulation
  ```

Install Gazebo Math:
  ```
  brew install gz-math<#>
  ```

Be sure to replace `<#>` with a number value, such as 6 or 7, depending on
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
conda install libgz-math<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need. From version 7 you should use `gz-math<#>` but for lower versions
you should use `ign-math<#>`.

# Source Installation

Source installation can be performed by first installing the necessary
prerequisites followed by building from source.

## Prerequisites

Gazebo Math requires:

* [Gazebo CMake](https://gazebosim.org/libs/cmake)

### Ubuntu Linux

The optional Eigen component of Gazebo Math requires:

* [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page). Refer to the [Eigen Documentation](http://eigen.tuxfamily.org/index.php?title=Main_Page#Documentation) for installation instructions. On Ubuntu systems, `apt-get` can be used to install Eigen:
  ```
  sudo apt-get install libeigen3-dev
  ```

The optional Python bindings of Gazebo Math require:

* [Pybind11](https://pybind11.readthedocs.io/en/stable/index.html). Refer to the [Pybind11 Documentation](https://pybind11.readthedocs.io/en/stable/installing.html) for installation instructions. On Ubuntu systems, `apt-get` can be used to install Pybind11:
  ```
  sudo apt-get install python3-pybind11
  ```

The optional Ruby tests of Gazebo Math require:

* [Ruby](https://www.ruby-lang.org/). Refer to the [Ruby Documentation](https://www.ruby-lang.org/downloads/) for installation instructions. On Ubuntu systems `apt-get` can be used to install Ubuntu Package `ruby-dev`:
  ```
  sudo apt-get install ruby-dev
  ```

* [Swig](http://www.swig.org/). Refer to the [Swig Documentation](http://www.swig.org/download.html) for installation instructions. On Ubuntu systems `apt-get` can be used to install Swig:
  ```
  sudo apt-get install swig
  ```

### Windows 10

First, follow the [gz-cmake](https://github.com/gazebosim/gz-cmake) tutorial for installing Conda, Visual Studio, CMake, and other prerequisites, and also for creating a Conda environment.

The optional Eigen component of Gazebo Math requires:

* [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page). Refer to the [Eigen Documentation](http://eigen.tuxfamily.org/index.php?title=Main_Page#Documentation) for installation instructions. On Windows, we will use `conda` to install Eigen:
  ```
  conda install eigen --channel conda-forge
  ```

## Building from Source

### Ubuntu

1. Install tools
  ```
  sudo apt install -y build-essential cmake g++-8 git gnupg lsb-release wget
  ```

2. Install dependencies
  ```
  sudo apt -y install \
    $(sort -u $(find . -iname 'packages-'`lsb_release -cs`'.apt' -o -iname 'packages.apt' | tr '\n' ' '))
  ```

3. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-math -b gz-math<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need. From version 7 you should use `gz-math<#>` but for lower versions
  you should use `ign-math<#>`.

4. Configure and build
  ```
  cd gz-math
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
  git clone https://github.com/gazebosim/gz-math -b gz-math<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need. From version 7 you should use `gz-math<#>` but for lower versions
  you should use `ign-math<#>`.

2. Install dependencies
  ```
  brew install --only-dependencies gz-math<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need. From version 7 you should use `gz-math<#>` but for lower versions
  you should use `ign-math<#>`.

3. Configure and build
  ```
  cd gz-math
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
  conda search libgz-math* --channel conda-forge --info
  ```
  From version 7 you should use `gz-math<#>` but for lower versions you should use `ign-math<#>`.
  See the [Conda release repository](https://github.com/conda-forge/libignition-math4-feedstock) for more information.

  Install dependencies, replacing `<#>` with the desired version:
  ```
  conda install libgz-cmake<#> --channel conda-forge
  ```

3. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b gz-math#` (replace # with a number) to check out a specific version
  git clone https://github.com/gazebosim/gz-math.git
  ```
  From version 7 you should use `gz-math<#>` but for lower versions
  you should use `ign-math<#>`.

4. Configure and build
  ```
  cd gz-math
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

5. Optionally, install
  ```
  cmake --install . --config Release
  ```

### cmake parameters

| Name            | Type | Default | Description                                |
|-----------------|------|---------|--------------------------------------------|
| `SKIP_PYBIND11` | BOOL | OFF     | Set to ON to skip building python bindings |

### Building Python bindings separately from main library

If you want to build Python bindings separately from the main gz-math library
(for example if you want to build Python bindings for multiple versions of Python),
you can invoke cmake on the `src/python_pybind11` folder instead of the root folder.
Specify the path to the python executable with which you wish to build bindings
in the `Python3_EXECUTABLE` cmake variable.
Specify the install path for the bindings in the `CMAKE_INSTALL_PREFIX`
variable, and be sure to set your `PYTHONPATH` accordingly after install.

```bash
cd gz-math
mkdir build_python3
cd build_python3
cmake ../src/python_pybind11 \
    -DPython3_EXECUTABLE=/usr/local/bin/python3.12 \
    -DCMAKE_INSTALL_PREFIX=<prefix>
```

See the homebrew [gz-math8 formula](https://github.com/osrf/homebrew-simulation/blob/ccda47647ed9aeb38f0ea1ec8804fd1501058de1/Formula/gz-math8.rb#L12-L52)
for an example of building bindings for multiple versions of Python.

# Documentation

API and tutorials can be found at [https://gazebosim.org/libs/math](https://gazebosim.org/libs/math).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using
 ```
 sudo apt-get install doxygen
 ```

2. Clone the repository
 ```
 git clone https://github.com/gazebosim/gz-math
 ```

3. Configure and build the documentation.
 ```
 cd gz-math; mkdir build; cd build; cmake ../; make doc
 ```

4. View the documentation by running the following command from the build directory.
 ```
 firefox doxygen/html/index.html
 ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](https://gazebosim.org/libs/math#source-install).

2. Run tests.
 ```
 make test
 ```

3. Static code checker.
 ```
 make codecheck
 ```

## Ruby Tests

### Usage

The C++ classes are available in Ruby code by interface files (`.i`) used by swig to build a C++ extension module.

The interfaces and Ruby test codes are in the `src` folder. To use a C++ class in Ruby you need to:

1. Create an interface file describing the class as in Swig and Ruby reference at [The Ruby-to-C/C++ Mapping](http://www.swig.org/Doc1.3/Ruby.html#Ruby_nn11)

2. Include the interface file in `/src/ing_math.i`

3. Create the Ruby file and import the class as in Swig and Ruby reference at [C++ Classes](http://www.swig.org/Doc1.3/Ruby.html#Ruby_nn18)

### Tests

`make test` already runs all tests, including the ones made in Ruby, but you can run a Ruby test individually using
  ```
  ctest -R Ruby_TEST.rb
  ```
