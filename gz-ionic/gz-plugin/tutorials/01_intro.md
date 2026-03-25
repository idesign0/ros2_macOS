\page introduction Introduction

Next Tutorial: \ref installation

Gazebo Plugin is a component in Gazebo, a set of libraries
designed to rapidly develop robot and simulation applications. The main
goal of the library is to simplify the process of registering plugin
libraries and dynamically loading them at runtime. It is based on the
following concepts:

* An "Interface" is a unit of functionality with a specified API that may be
  provided by a plugin. Interfaces may be defined using templates.
* A Plugin may provide multiple Interfaces, including different specializations
  of templated Interfaces.
* A "Library" may contain multiple plugins, which provides a convenient
  way to distribute different types of functionality in a single file.

The **loader** component can dynamically load plugin interfaces at runtime
either by specifying a desired plugin or by listing plugins within a library
file that provide a specific interface.
There are highly performant ways to get direct access to a plugin interface
that can be anticipated ahead of time by specializing the `Plugin` object at
compile time.
The loader also can use reference counting of plugin objects to unload
libraries that are no longer in use.

The **register** component provides macros for registering templated and
non-templated classes as plugins and/or interfaces.
A plugin can be registered anywhere from within any translation unit.
Interfaces can be queried by their C++ type, or they can be queried by
name with a string.

The [Gazebo Physics](https://gazebosim.org/libs/physics) Feature
system provides a good example of the use of these unique aspects of
gz-plugin.
