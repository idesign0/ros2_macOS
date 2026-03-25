# Gazebo Utils : Classes and functions for robot applications

**Maintainer:** addisu AT openrobotics DOT org

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-utils.svg)](https://github.com/gazebosim/gz-utils/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-utils.svg)](https://github.com/gazebosim/gz-utils/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-utils/branch/gz-utils3/graph/badge.svg)](https://app.codecov.io/gh/gazebosim/gz-utils/tree/gz-utils3)
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_utils-ci-gz-utils3-noble-amd64)](https://build.osrfoundation.org/job/gz_utils-ci-gz-utils3-noble-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_utils-ci-gz-utils3-homebrew-amd64)](https://build.osrfoundation.org/job/gz_utils-ci-gz-utils3-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/job/gz_utils-3-clowin/badge/icon)](https://build.osrfoundation.org/job/gz_utils-3-clowin/)

Gazebo Utils, a component of [Gazebo](https://gazebosim.org), provides general purpose
classes and functions designed for robotic applications.

# Table of Contents

[Features](#features)

[Install](#install)

[Usage](#usage)

[Documentation](#documentation)

[Testing](#testing)

[Folder Structure](#folder-structure)

[Code of Conduct](#code-of-conduct)

[Contributing](#code-of-contributing)

[Versioning](#versioning)

[License](#license)

# Features

Gazebo Utils provides a wide range of functionality, including:

* A helper class to implement the PIMPL pattern
* A command line parsing utility (vendored [CLI11](https://github.com/CLIUtils/CLI11/))
* Macros to suppress warnings

# Install

See the [installation tutorial](https://gazebosim.org/api/utils/2/install.html).

# Usage

Please refer to the [examples directory](https://github.com/gazebosim/gz-utils/blob/gz-utils3/examples/).

# Documentation

API and tutorials can be found at [https://gazebosim.org/libs/utils](https://gazebosim.org/libs/utils).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using

    ```
    sudo apt-get install doxygen
    ```

2. Clone the repository

    ```
    git clone https://github.com/gazebosim/gz-utils
    ```

3. Configure and build the documentation.

    ```
    cd gz-utils; mkdir build; cd build; cmake ../; make doc
    ```

4. View the documentation by running the following command from the build directory.

    ```
    firefox doxygen/html/index.html
    ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](https://gazebosim.org/api/utils/2/install.html#source-install).

2. Run tests.

    ```
    make test
    ```

3. Static code checker.

    ```
    make codecheck
    ```

# Folder Structure

Refer to the following table for information about important directories and files in this repository.

```
gz-utils
├── cli                      Gazebo CLI component. Vendored from https://github.com/CLIUtils/CLI11/
├── examples                 Example programs.
├── include/gz/utils         Header files.
├── src                      Source files and unit tests.
├── test
│    ├── integration         Integration tests.
│    ├── performance         Performance tests.
│    └── regression          Regression tests.
├── tutorials                Tutorials, written in markdown.
├── Changelog.md             Changelog.
└── CMakeLists.txt           CMake build script.
```
# Contributing

Please see
[CONTRIBUTING.md](https://gazebosim.org/docs/all/contributing).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/gazebosim/gz-utils/blob/main/CODE_OF_CONDUCT.md)

# Versioning

This library uses [Semantic Versioning](https://semver.org/).
Additionally, this library is part of the
[Gazebo project](https://gazebosim.org) which periodically
releases a versioned set of compatible and complimentary libraries. See the
[Gazebo website](https://gazebosim.org) for version and
release information.

# License

This library is licensed under
[Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0).
See also the
[LICENSE](https://github.com/gazebosim/gz-utils/blob/main/LICENSE)
file.
