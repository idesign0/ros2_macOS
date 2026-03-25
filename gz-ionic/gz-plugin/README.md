# Gazebo Plugin

**Maintainer:** ahcorde [AT] gmail [DOT] com

[![GitHub open issues](https://img.shields.io/github/issues-raw/gazebosim/gz-plugin.svg)](https://github.com/gazebosim/gz-plugin/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/gazebosim/gz-plugin.svg)](https://github.com/gazebosim/gz-plugin/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/gazebosim/gz-plugin/tree/gz-plugin3/graph/badge.svg)](https://codecov.io/gh/gazebosim/gz-plugin/tree/gz-plugin3)
Ubuntu Noble  | [![Build Status](https://build.osrfoundation.org/job/gz_plugin-ci-gz-plugin3-noble-amd64/badge/icon)](https://build.osrfoundation.org/job/gz_plugin-ci-gz-plugin3-noble-amd64/)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_plugin-ci-gz-plugin3-homebrew-amd64)](https://build.osrfoundation.org/job/gz_plugin-ci-gz-plugin3-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=gz_plugin-3-clowin)]([https://build.osrfoundation.org/job/gz_plugin-3-clowin](https://build.osrfoundation.org/job/gz_plugin-3-clowin/))


Gazebo Plugin is a component in the [Gazebo](http://gazebosim.org) framework, a set
of libraries designed to rapidly develop robot applications.
It is used to register plugin libraries and load them dynamically at runtime.

If you want to make a Gazebo Plugin, you can start learning how to [with a tutorial](https://gazebosim.org/api/sim/9/createsystemplugins.html)

[http://gazebosim.org](http://gazebosim.org)

## Features

* Registration of **templated and non-templated** classes from any translation unit.
* Querying of interfaces **by C++ type or name string**.
* **Dynamically load** plugins at runtime according to the interfaces they provide.
* **Reference counting** of plugin objects to unload libraries no longer in use.
* **Command line tool** `gz plugin` to inspect plugins.

## Installation

See the [installation tutorial](https://gazebosim.org/api/plugin/2/installation.html).

# Documentation

Visit the [documentation page](https://gazebosim.org/api/plugin/2/index.html).
