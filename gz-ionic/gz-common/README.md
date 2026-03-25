# Gazebo Common : AV, Graphics, Events, and much more.

**Maintainer:** nate AT openrobotics DOT org

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-common.svg)](https://github.com/gazebosim/gz-common/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-common.svg)](https://github.com/gazebosim/gz-common/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-common/tree/gz-common6/graph/badge.svg)](https://codecov.io/gh/gazebosim/gz-common/tree/gz-common6)
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_common-ci-gz-common6-noble-amd64)](https://build.osrfoundation.org/job/gz_common-ci-gz-common6-noble-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_common-ci-gz-common6-homebrew-amd64)](https://build.osrfoundation.org/job/gz_common-ci-gz-common6-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/job/gz_common-6-clowin/badge/icon)](https://build.osrfoundation.org/job/gz_common-6-clowin/)

Gazebo Common, a component of [Gazebo](https://gazebosim.org), provides a set of libraries that
cover many different use cases. An audio-visual library supports
processing audio and video files, a graphics library can load a variety 3D
mesh file formats into a generic in-memory representation, and the core
library of Gazebo Common contains functionality that spans Base64
encoding/decoding to thread pools.

# Table of Contents

[Features](#features)

[Install](#install)

[Usage](#usage)

[Folder Structure](#folder-structure)

[Contributing](#contributing)

[Code of Conduct](#code-of-conduct)

[Versioning](#versioning)

[License](#license)

# Features

Some of the many capabilities contained in Gazebo Common are:

* **AV**: FFMpeg based audio decoder, and video encoder and decoder.
* **Core**: Base64 encoding and decoding, battery model, console logging,
  cross-platform filesystem interface, URI processing, and a thread pool.
* **Events**: Mouse and keyboard events, and a high-performance signal and
callback system.
* **Graphics**: Collada, SVG, STL, OBJ, and DEM loaders. In-memory mesh,
  image, and material representations. Animation processing and BVH loader.
* **Profiler**: A common profiler abstraction that can be used to measure and
  visualize run time of various pieces of Gazebo software.

# Install

See the [installation tutorial](https://gazebosim.org/api/common/6/install.html).

# Usage

Please refer to the [examples directory](https://github.com/gazebosim/gz-common/tree/gz-common6/examples).

# Folder Structure

Refer to the following table for information about important directories and files in this repository.

```
+-- av                       Header and source files for the AV component.
+-- events                   Header and source files for the Event component.
+-- examples                 Example programs.
+-- geospatial               Geospatial component for heightmap & DEMs
+-- graphics                 Header and source files for the Graphics component.
+-- include/gz/common        Header files for the core component.
+-- io                       IO component. Read object from stream.
+-- profiler                 Header and source files for the Profiler component.
+-- src                      Core source files and unit tests.
+-- test
|    +-- integration         Integration tests.
|    +-- performance         Performance tests.
|    +-- static_assertions   Static assertion tests.
|    +-- regression          Regression tests.
+-- tutorials                Tutorials, written in markdown.
+-- Changelog.md             Changelog.
+-- CMakeLists.txt           CMake build script.
+-- Migration.md             Migration guide.
```
# Contributing

Please see the
[contributing guide](https://gazebosim.org/docs/all/contributing).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/gazebosim/gz-sim/blob/main/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally, this library is part of the [Gazebo project](https://gazebosim.org) which periodically releases a versioned set of compatible and complimentary libraries. See the [Gazebo website](https://gazebosim.org) for version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0). See also the [LICENSE](https://github.com/gazebosim/gz-common/blob/main/LICENSE) file.
