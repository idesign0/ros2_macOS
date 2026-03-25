# Gazebo Tools: Entrypoint to Gazebo's command line interface

**Maintainer:** caguero AT honurobotics DOT com

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-tools.svg)](https://github.com/gazebosim/gz-tools/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-tools.svg)](https://github.com/gazebosim/gz-tools/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-tools/branch/ign-tools2/graph/badge.svg)](https://codecov.io/gh/gazebosim/gz-tools)
Ubuntu Jammy  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_tools-ci-gz-tools2-jammy-amd64)](https://build.osrfoundation.org/job/gz_tools-ci-gz-tools2-jammy-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_tools-ci-gz-tools2-homebrew-amd64)](https://build.osrfoundation.org/job/gz_tools-ci-gz-tools2-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_tools-2-clowin)](https://build.osrfoundation.org/job/gz_tools-2-clowin)

Gazebo Tools provide the `gz` command line tool that accepts multiple
subcommands. Each subcommand is implemented in a plugin that belongs to a
specific Gazebo project. For example, all the commands that start with
`gz topic ...` are implemented by the Gazebo Transport library.

# Table of Contents

[Features](#features)

[Install](#install)

[Contributing](#contributing)

[Code of Conduct](#code-of-conduct)

[Versioning](#versioning)

[License](#license)

# Features

Gazebo Tools provides the `gz` CLI for the entire Gazebo project.

# Install

See the [installation tutorial](https://gazebosim.org/api/tools/2/install.html).

# Contributing

Please see the [contribution guide](https://gazebosim.org/docs/all/contributing).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/gazebosim/gz-sim/blob/main/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally, this library is part of the [Gazebo project](https://gazebosim.org) which periodically releases a versioned set of compatible and complimentary libraries. See the [Gazebo website](https://gazebosim.org) for version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0). See also the [LICENSE](https://github.com/gazebosim/gz-tools/blob/main/LICENSE) file.
