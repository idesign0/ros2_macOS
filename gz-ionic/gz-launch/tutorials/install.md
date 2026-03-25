\page install Installation

# Install

These instructions are for installing only Gazebo Launch. If you're interested
in using all the Gazebo libraries, not only Gazebo Launch, check out this
[Gazebo installation](https://gazebosim.org/docs/latest/install).

We recommend following the binary install instructions to get up and running as
quickly and painlessly as possible.

The source install instructions should be used if you need the very latest
software improvements, if you need to modify the code, or if you plan to make a
contribution.

# Binary Install

We recommend following the [Binary Install](#binary-install) instructions to get up and running as quickly and painlessly as possible.

The [Source Install](#source-install) instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

## Ubuntu 20.04 or above

On Ubuntu systems, `apt` can be used to install `gz-launch`:

```
sudo apt install libgz-launch<#>
```

Be sure to replace `<#>` with a number value, such as 6 or 7, depending on
which version you need.

## macOS

On macOS, add OSRF packages:
  ```
  ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  brew tap osrf/simulation
  ```

Install Gazebo GUI:
  ```
  brew install gz-launch<#>
  ```

Be sure to replace `<#>` with a number value, such as 4 or 5, depending on
which version you need.

# Source Install


## Ubuntu 20.04 or above

Source installation can be performed in UNIX systems by first installing the
necessary prerequisites followed by building from source.

### Prerequisites

**[Ubuntu Focal](http://releases.ubuntu.com/20.04/)** or higher

1. Install third-party libraries:

    ```sh
    sudo apt -y install \
      $(sort -u $(find . -iname 'packages-'`lsb_release -cs`'.apt' -o -iname 'packages.apt' | grep -v '/\.git/') | sed '/gz\|sdf/d' | tr '\n' ' ')
    ```

1. Install required Gazebo libraries

    ```sh
    sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
    wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
    sudo apt-get update
    sudo apt-get -y install libgz-cmake4-dev libgz-sim9-dev
    ```

### Building from source

1. Clone the repository

    ```sh
    git clone https://github.com/gazebosim/gz-launch
    ```

2. Configure and build

    ```sh
    cd gz-launch; mkdir build; cd build; cmake ..; make
    ```

3. Optionally, install Gazebo Launch

    ```sh
    sudo make install
    ```

## macOS

1. Clone the repository

  ```sh
  git clone https://github.com/gazebosim/gz-launch -b gz-launch<#>
  ```

  Be sure to replace `<#>` with a number value, such as 4 or 5, depending on
  which version you need.

2. Install dependencies

  ```sh
  brew install --only-dependencies gz-launch<#>
  ```

  Be sure to replace `<#>` with a number value, such as 5 or 6, depending on
  which version you need.

3. Configure and build

  ```sh
  cd gz-launch
  mkdir build
  cd build
  cmake ..
  make
  ```

4. Optionally, install

  ```sh
  sudo make install
  ```

# Documentation

API and tutorials can be found at [https://gazebosim.org/libs/launch](https://gazebosim.org/libs/launch).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using

    ```sh
    sudo apt-get install doxygen
    ```

2. Clone the repository

    ```sh
    git clone https://github.com/gazebosim/gz-launch
    ```

3. Configure and build the documentation.

    ```sh
    cd gz-launch; mkdir build; cd build; cmake ../; make doc
    ```

4. View the documentation by running the following command from the build directory.

    ```sh
    firefox doxygen/html/index.html
    ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](#source-install).

2. Run tests.

    ```sh
    make test
    ```

3. Static code checker.

    ```sh
    make codecheck
    ```
