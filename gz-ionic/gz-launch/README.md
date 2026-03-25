# Gazebo Launch : Run and manage programs and plugins

**Maintainer:** nate AT openrobotics DOT org

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-launch.svg)](https://github.com/gazebosim/gz-launch/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-launch.svg)](https://github.com/gazebosim/gz-launch/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-launch/tree/gz-launch8/graph/badge.svg)](https://codecov.io/gh/gazebosim/gz-launch/branch/gz-launch8)
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_launch-ci-gz-launch8-noble-amd64)](https://build.osrfoundation.org/job/gz_launch-ci-gz-launch8-noble-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_launch-ci-gz-launch8-homebrew-amd64)](https://build.osrfoundation.org/job/gz_launch-ci-gz-launch8-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_launch-8-clowin)](https://build.osrfoundation.org/job/gz_launch-8-clowin)

Gazebo Launch, a component of [Gazebo](https://gazebosim.org), provides a command line interface
to run and manage application and plugins.

# Table of Contents

[Features](#features)

[Install](#install)

[Usage](#usage)

[Documentation](#documentation)

[Folder Structure](#folder-structure)

[Contributing](#contributing)

[Code of Conduct](#code-of-conduct)

[Versioning](#versioning)

[License](#license)

# Features

Gazebo Launch is used to run and manage plugins and programs. A
configuration script can be used to specify which programs and plugins to
execute. Alternatively, individual programs and plugins can be run from the
command line. Example configuration scripts are located in the `examples`
directory.

1. Automatic ERB parsing of configuration files.
1. Pass arguments to launch files from the command line.
1. Plugins to launch Gazebo Sim, joystick interface, and a websocket server for
   simulation.

# Install

See the [installation tutorial](https://gazebosim.org/api/launch/8/install.html).

# Usage

Sample launch configuration files are in the [examples directory](https://github.com/gazebosim/gz-launch/blob/gz-launch8/examples/).

**Example**

1. Run a configuration that launches [Gazebo](https://gazebosim.org/libs/sim/).

    ```
    gz launch sim.gzlaunch
    ```

## Known issue of command line tools

In the event that the installation is a mix of Debian and from source, command
line tools from `gz-tools` may not work correctly.

A workaround is to define the environment variable
`GZ_CONFIG_PATH` to point to the different locations of the Gazebo libraries installations,
where the YAML files for the packages are found, such as
```
export GZ_CONFIG_PATH=/usr/local/share/gz:$HOME/ws/install/share/gz
```

where `$HOME/ws` is an example colcon workspace used to build Gazebo

# Documentation

See the [installation tutorial](https://gazebosim.org/api/launch/8/install.html).

# Folder Structure

Refer to the following table for information about important directories and files in this repository.

```
gz-launch
├── examples                 Example launch configurations.
├── include/gz/launch  Header files.
├── plugins                  Launch plugins, one per subdirectory.
├── src                      Source files and unit tests.
├── test
│    ├── integration         Integration tests.
│    ├── performance         Performance tests.
│    └── regression          Regression tests.
├── tools                    Some useful tools such as linters
├── tutorials                Tutorials
├── Changelog.md             Changelog.
└── CMakeLists.txt           CMake build script.
```

# Contributing

Please see the [contribution guide](https://gazebosim.org/docs/all/contributing).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/gazebosim/gz-sim/blob/main/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally, this library is part of the [Gazebo project](https://gazebosim.org) which periodically releases a versioned set of compatible and complimentary libraries. See the [Gazebo website](https://gazebosim.org) for version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0). See also the [LICENSE](https://github.com/gazebosim/gz-launch/blob/main/LICENSE) file.
