\page introduction Introduction

Next Tutorial: \ref installation

## What is Gazebo Rendering?

Gazebo Rendering is an open source library that provides an abstraction layer
to multiple rendering engines. The goal is to provide a set of unified C++
rendering APIs that are rendering-engine-agnostic. For instance, a scene created
through the Gazebo Rendering API could be rendered using an OpenGL based
rendering library or through a ray tracer. You have the option to choose which
rendering engine to use at run time.

Gazebo Rendering has a plugin architecture. Each rendering engine is
integrated in the form of a Gazebo Rendering plugin. In a nutshell, a plugin
is essentially an interface to the underlying engine. Gazebo Rendering exposes
a set of abstract public API interfaces and the engine plugins provide the
implementation. So when a user specifies a particular rendering engine to use,
the relevant plugin will be loaded and all rendering calls will be made through
that rendering engine.
