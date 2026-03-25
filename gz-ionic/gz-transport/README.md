# Gazebo Transport

**Maintainer:** caguero AT honurobotics DOT com

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-transport.svg)](https://github.com/gazebosim/gz-transport/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-transport.svg)](https://github.com/gazebosim/gz-transport/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-transport/tree/gz-transport14/graph/badge.svg)](https://codecov.io/gh/gazebosim/gz-transport/tree/gz-transport14)
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_transport-ci-gz-transport14-noble-amd64)](https://build.osrfoundation.org/job/gz_transport-ci-gz-transport14-noble-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_transport-ci-gz-transport14-homebrew-amd64)](https://build.osrfoundation.org/job/gz_transport-ci-gz-transport14-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_transport-14-clowin)](https://build.osrfoundation.org/job/gz_transport-14-clowin/)

Gazebo Transport, a component of [Gazebo](https://gazebosim.org), provides fast and efficient asynchronous message passing, services, and data logging.

# Table of Contents

[Features](#features)

[Install](#install)

[Usage](#usage)

[Documentation](#documentation)

[Testing](#testing)

[Folder Structure](#folder-structure)

[Contributing](#contributing)

[Code of Conduct](#code-of-conduct)

[Versioning](#versioning)

[License](#license)

# Features

Gazebo Transport is an open source communication library that allows
exchanging data between clients. In our context, a client is called a node.
Nodes might be running within the same process in the same machine or in
machines located in different continents. Gazebo Transport is multi-platform
(Linux, Mac OS X, and Windows), so all the low level details, such as data
alignment or endianness are hidden for you.

Gazebo Transport uses Google Protocol buffers as the data serialization format
for communicating between nodes. Users can define their own messages using the
Protobuf utils, and then, exchange them between the nodes. Gazebo Transport
discovers, serializes and delivers messages to the destinations using a
combination of custom code and ZeroMQ.

# Install

See the [installation tutorial](https://gazebosim.org/api/transport/14/installation.html).

# Usage

See [tutorials](https://gazebosim.org/api/transport/14/tutorials.html)
and the [example directory](https://github.com/gazebosim/gz-transport/blob/main/example/)
in the source code.

## Known issue of command line tools

In the event that the installation is a mix of Debian and from source, command
line tools from `gz-tools` may not work correctly.

A workaround is to define the environment variable
`GZ_CONFIG_PATH` to point to the location of the Gazebo library installation,
where the YAML file for the package is found, such as
```
export GZ_CONFIG_PATH=/usr/local/share/gz
```

This issue is tracked [here](https://github.com/gazebosim/gz-tools/issues/61).

# Documentation

Visit the [documentation page](https://gazebosim.org/api/transport/14/index.html).

# Folder Structure

```
gz-transport
├── conf        Configuration file for the integration with the `gz` CLI tool.
├── docker      Dockerfile with gz-transport installed and scripts to build and run the code.
├── example     Example programs that use most of the Gazebo Transport API.
├── include     Header files that get installed.
├── log         All the code related with Gazebo Transport logging.
├── src         Source code of the core library.
├── test        A directory of integration, performance and regression tests.
└── tutorials   A set of tutorials about Gazebo Transport features.
```

# Contributing

Please see
[CONTRIBUTING.md](https://github.com/gazebosim/gz-sim/blob/main/CONTRIBUTING.md).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/gazebosim/gz-sim/blob/main/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally,
this library is part of the [Gazebo project](https://gazebosim.org)
which periodically releases a versioned set of compatible and complimentary
libraries. See the [Gazebo website](https://gazebosim.org) for
version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0).
See also the [LICENSE](https://github.com/gazebosim/gz-transport/raw/main/LICENSE)
file.
