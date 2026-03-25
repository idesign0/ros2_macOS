\page install Installation

Next Tutorial: \ref cppgetstarted

These instructions are for installing only Gazebo Messages.
If you're interested in using all the Gazebo libraries, check out this [Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the Binary Installation instructions to get up and running as quickly and painlessly as possible.

The Source Installation instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

# Binary Installation

## Ubuntu

On Ubuntu systems, `apt-get` can be used to install `gz-msgs`:
```
sudo apt install libgz-msgs<#>-dev
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

## macOS

On macOS, add OSRF packages:
  ```
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  brew tap osrf/simulation
  ```

Install Gazebo Msgs:
  ```
  brew install gz-msgs<#>
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

Install `gz-msgs`:
```
conda install libgz-msgs<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

# Source Installation

## Ubuntu

### Prerequisites

Install required dependencies as follows:
```
sudo apt-get install libprotobuf-dev protobuf-compiler libprotoc-dev libgz-math8-dev
```

### Building from Source

Standard installation can be performed in UNIX systems using the following
steps:
```
mkdir build/
cd build/
cmake ..
sudo make install
```

### Uninstallation

To uninstall the software installed with the previous steps:
```
cd build/
sudo make uninstall
```

## macOS

1. Clone the repository
  ```
  git clone https://github.com/gazebosim/gz-msgs -b ign-msgs<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need.

2. Install dependencies
  ```
  brew install --only-dependencies gz-msgs<#>
  ```
  Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
  which version you need.

3. Configure and build
  ```
  cd gz-msgs
  mkdir build
  cd build
  cmake ..
  make
  ```

4. Optionally, install
  ```
  sudo make install
  ```

  To uninstall the software installed with the previous steps:
  ```
  cd build/
  sudo make uninstall
  ```

## Windows

### Prerequisites

First, follow the [gz-cmake](https://github.com/gazebosim/gz-cmake) tutorial for installing Conda, Visual Studio, CMake, etc., prerequisites, and creating a Conda environment.

Navigate to `condabin` if necessary to use the `conda` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of `condabin` in Anaconda Prompt, `where conda`).

Activate the Conda environment:
```
conda activate gz-ws
```

Install prerequisites:
```
conda install tinyxml2 protobuf --channel conda-forge
```

Install Gazebo dependencies:

You can view lists of dependencies:
```
conda search libgz-msgs* --channel conda-forge --info
```

Install dependencies, replacing `<#>` with the desired versions:
```
conda install libgz-cmake<#> libgz-math<#> libgz-tools<#> --channel conda-forge
```

### Building from Source

This assumes you have created and activated a Conda environment while installing the Prerequisites.

1. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b ign-msgs#` (replace # with a number) to check out a specific version
  git clone https://github.com/gazebosim/gz-msgs.git
  ```

2. Configure and build
  ```
  cd gz-msgs
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

## Documentation Release

1. Upload documentation to gazebosim.org.
  ```
  cd build
  sh upload.sh
  ```

2. If you're creating a new release, then tell gazebosim.org about
   the new version. For example:
  ```
  curl -k -X POST -d '{"libName":"msgs", "version":"1.0.0", "releaseDate":"2017-10-09T12:10:13+02:00","password":"secret"}' https://api    .gazebosim.org/1.0/versions
  ```

## Testing

Run tests as follows:
  ```
  make test
  ```

Tests are automatically built. To disable them, run `cmake` as follows:
  ```
  cmake .. -DENABLE_TESTS_COMPILATION=false
  ```

### Test coverage

To run test coverage:

1. Install LCOV
  ```
  sudo apt-get install lcov
  ```

2. Build with coverage
  ```
  cd build/
  cmake .. -DCMAKE_BUILD_TYPE=coverage
  make
  ```

3. Run tests
  ```
  make test
  ```

4. Generate coverage
  ```
  make coverage
  ```

5. View results
  ```
  firefox coverage/index.html
  ```

See the [Writing Tests section of the Gazebo documentation](https://gazebosim.org/docs/all/contributing#writing-tests) for help creating or modifying tests.
