# Introduction

Gazebo Fuel Tools provides both a command line tool and a C++ API which allow
interacting with Gazebo Fuel web servers such as
[https://app.gazebosim.org](https://app.gazebosim.org) and their hosted
simulation resources.

## Simulation resources

The Gazebo Fuel web server hosts several resource types relevant to robotics
simulation, such as models and worlds.

### Models

A common asset stored in a Gazebo Fuel server is a model. A model represents
an object in simulation. A model has a name and a description of its visual and
physical properties. Models can be as simple as a sphere or complicated as a
highly articulated humanoid robot. See the full model specification
[here](http://sdformat.org/spec?ver=1.6&elem=model).

### Worlds

Among the resources hosted at Gazebo Fuel there are "worlds", which describe a
everything inside a simulation, from the robots to the scene properties, lights,
physics properties, etc. You can see the full world description
<a href="http://sdformat.org/spec?elem=world">here</a>.
