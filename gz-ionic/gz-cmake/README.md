# Gazebo CMake : CMake Modules for Gazebo Projects

**Maintainer:** scpeters AT openrobotics.org

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-cmake.svg)](https://github.com/gazebosim/gz-cmake/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-cmake.svg)](https://github.com/gazebosim/gz-cmake/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_cmake-ci-gz-cmake4-noble-amd64)](https://build.osrfoundation.org/job/gz_cmake-ci-gz-cmake4-noble-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_cmake-ci-gz-cmake4-homebrew-amd64)](https://build.osrfoundation.org/job/gz_cmake-ci-gz-cmake4-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_cmake-4-clowin)](https://build.osrfoundation.org/job/gz_cmake-4-clowin)

# Table of Contents

[Features](#features)

[Install](#install)

* [Binary Install](#binary-install)

* [Source Install](#source-install)

    * [Prerequisites](#prerequisites)

    * [Building from Source](#building-from-source)

[Usage](#usage)

[Documentation](#documentation)

[Testing](#testing)

[Folder Structure](#folder-structure)

[Contributing](#contributing)

[Code of Conduct](#code-of-conduct)

[Versioning](#versioning)

[License](#license)

# Features

gz-cmake provides a set of cmake modules that are used by the C++ based Gazebo projects. These modules help to control the quality and consistency of the Gazebo projects build systems.

These modules are tailored to the Gazebo projects, so their use for non-Gazebo projects might be limited, but they may serve as a useful reference for setting up a modern cmake build system using good practices.

# Install

We recommend following the [Binary Install](#binary-install) instructions to get up and running as quickly and painlessly as possible.

The [Source Install](#source-install) instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

## Binary Install

On Ubuntu systems, `apt-get` can be used to install `gz-cmake`:

```bash
sudo apt install libgz-cmake<#>-dev
```

Be sure to replace `<#>` with a number value, such as `1` or `2`, depending on which version you need.

## Source Install

### Prerequisites

The only prerequisite of `gz-cmake` is `cmake`. Ubuntu users can install cmake with the package manager:

```bash
sudo apt install cmake
```

### Building from source

To build and install from source, you can clone the repo and use cmake to install the modules as though this is a regular cmake project:

```bash
git clone https://github.com/gazebosim/gz-cmake
cd gz-cmake
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install/dir
make -j8
make install
```

Replace `/path/to/install/dir` to whatever directory you want to install this package to.

# Usage

Documentation can be accessed at https://gazebosim.org/libs/cmake

[Examples](https://github.com/gazebosim/gz-cmake/tree/gz-cmake4/examples) are available in this repository.

[Tutorials](https://github.com/gazebosim/gz-cmake/tree/gz-cmake4/tutorials) are also available in this repository.

# Documentation

Documentation for `gz-cmake` can be found within the source code, and also in the [MIGRATION.md guide](https://github.com/gazebosim/gz-cmake/blob/gz-cmake4/Migration.md).

# Testing

A fuller suite of tests in the `examples` directory can be enabled by building with `BUILDSYSTEM_TESTING` enabled.
Tests can be run by building the `test` target. From your build directory you can run:

```bash
cmake .. -DBUILDSYSTEM_TESTING=1
make test
```

# Folder Structure

* `cmake`: cmake modules that get installed by this package
* `codecheck`: code linting and static analyzing utilities that get installed by this package
* `config`: template files for producing the config-files of `gz-cmake`; these are only used internally
* `doc`: template files to help Gazebo projects generate their own documentation
* `examples`: fake projects that are used to test `gz-cmake`
* `include`: C++ utility header files that get installed with `gz-cmake`
* `test`: a directory of tests for the C++ utility component of `gz-cmake`
* `tools`: scripts for continuous integration testing

# Contributing

Please see the
[contribution guide](https://gazebosim.org/docs/all/contributing).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/gazebosim/gz-sim/blob/main/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally, this library is part of the [Gazebo project](https://gazebosim.org) which periodically releases a versioned set of compatible and complementary libraries. See the [Gazebo website](https://gazebosim.org) for version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0). See also the [LICENSE](https://github.com/gazebosim/gz-cmake/blob/main/LICENSE) file.
