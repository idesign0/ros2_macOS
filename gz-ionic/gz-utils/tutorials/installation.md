\page install Installation

# Overview

This tutorial describes how to install Gazebo Utils on Linux, Mac OS X and
Windows via either a binary distribution or from source.

[Installation instructions](#installation-instructions)

- [Binary Install](#binary-install)

- [Source Install](#source-install)

[Documentation](#documentation)

[Testing](#testing)

# Installation instructions

We recommend following the [Binary Install](#binary-install) instructions to get up and running as quickly and painlessly as possible.

The [Source Install](#source-install) instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

## Binary Install

### Ubuntu Linux

Setup your computer to accept software from *packages.osrfoundation.org*:

```{.sh}
sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
```

Setup keys:

```{.sh}
wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
```

Install Gazebo Utils:

```{.sh}
sudo apt install libgz-utils<#>-dev
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

### macOS

On macOS, add OSRF packages:

```{.sh}
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew tap osrf/simulation
```

Install Gazebo utils:

```{.sh}
brew install gz-utils<#>
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

### Windows

Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
Miniconda suffices.

Create if necessary, and activate a Conda environment:

```{.sh}
conda create -n gz-ws
conda activate gz-ws
```

Install `gz-utils`:

```{.sh}
conda install libgz-utils<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
which version you need.

## Source Install

Source installation can be performed in UNIX systems by first installing the
necessary prerequisites followed by building from source.

### Ubuntu

1. Install tools

   ```{.sh}
   sudo apt install -y build-essential cmake git gnupg lsb-release wget
   ```

2. Clone the repository

   ```{.sh}
   git clone https://github.com/gazebosim/gz-utils -b gz-utils<#>
   ```

   Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
   which version you need.

3. Install dependencies

   ```{.sh}
   sudo apt -y install \
     $(sort -u $(find . -iname 'packages-'`lsb_release -cs`'.apt' -o -iname 'packages.apt' | tr '\n' ' '))
   ```

4. Configure and build

   ```{.sh}
   cd gz-utils; mkdir build; cd build; cmake ..; make
   ```

5. Optionally, install Gazebo Utils

   ```{.sh}
   sudo make install
   ```

### macOS

1. Clone the repository

   ```{.sh}
   git clone https://github.com/gazebosim/gz-utils -b gz-utils<#>
   ```

   Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
   which version you need.

2. Install dependencies

   ```{.sh}
   brew install --only-dependencies gz-utils<#>
   ```

   Be sure to replace `<#>` with a number value, such as 1 or 2, depending on
   which version you need.

3. Configure and build

   ```{.sh}
   cd gz-utils
   mkdir build
   cd build
   cmake ..
   make
   ```

4. Optionally, install

   ```{.sh}
   sudo make install
   ```

### Windows

This assumes you have created and activated a Conda environment while installing the Prerequisites.

1. Navigate to where you would like to build the library, and clone the repository.

   ```{.sh}
   # Optionally, append `-b gz-utils#` (replace # with a number) to check out a specific version
   git clone https://github.com/gazebosim/gz-utils.git
   ```

2. Configure and build

   ```{.sh}
   cd gz-utils
   mkdir build
   cd build
   cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
   cmake --build . --config Release
   ```

3. Optionally, install

   ```{.sh}
   cmake --install . --config Release
   ```

# Documentation

API and tutorials can be found at [https://gazebosim.org/libs/utils](https://gazebosim.org/libs/utils).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using

   ```{.sh}
   sudo apt-get install doxygen
   ```

2. Clone the repository

   ```{.sh}
   git clone https://github.com/gazebosim/gz-utils
   ```

3. Configure and build the documentation.

   ```{.sh}
   cd gz-utils
   mkdir build
   cd build
   cmake ../
   make doc
   ```

4. View the documentation by running the following command from the build directory.

   ```{.sh}
   firefox doxygen/html/index.html
   ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](#source-install).

2. Run tests.

   ```{.sh}
   make test
   ```

3. Static code checker.

   ```{.sh}
   make codecheck
   ```
