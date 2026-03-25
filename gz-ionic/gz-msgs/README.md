# Gazebo Messages: Protobuf messages and functions for robot applications

**Maintainer:** caguero AT honurobotics DOT com

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-msgs.svg)](https://github.com/gazebosim/gz-msgs/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-msgs.svg)](https://github.com/gazebosim/gz-msgs/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-msgs/tree/gz-msgs11/graph/badge.svg)](https://codecov.io/gh/gazebosim/gz-msgs/tree/gz-msgs11)
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_msgs-ci-gz-msgs11-noble-amd64)](https://build.osrfoundation.org/job/gz_msgs-ci-gz-msgs11-noble-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_msgs-ci-gz-msgs11-homebrew-amd64)](https://build.osrfoundation.org/job/gz_msgs-ci-gz-msgs11-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_msgs-11-clowin)](https://build.osrfoundation.org/job/gz_msgs-11-clowin)

Gazebo Messages is a component in the [Gazebo](http://gazebosim.org)
framework, a set of libraries designed to rapidly develop robot applications.

## Installation

See the [installation tutorial](https://gazebosim.org/api/msgs/11/install.html).

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

