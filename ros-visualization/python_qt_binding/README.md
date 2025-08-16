# python_qt_binding

This stack provides Python bindings for Qt.
There are two providers: pyside and pyqt.
PySide2 is available under the GPL, LGPL and a commercial license.
PyQt is released under the GPL.

Both the bindings and tools to build bindings are included from each available provider.
For PySide, it is called "Shiboken".
For PyQt, this is called "SIP".

Also provided is adapter code to make the user's Python code independent of which binding provider was actually used.

# Contributing

For ROS 2 changes, please open pull requests on the branch `main`.
For ROS 1 changes, please open pull requests on the `melodic-devel` branch.
If you would like changes made to older ROS distro branches, backporting will be considered once the changes are made to the latest branch.
Changes will only be made to [active ROS Distros](https://dlu.github.io/ros_clock/index.html).

# Branches

The branches on this repo are for different ROS distros.

* `main` - ROS Rolling
* `galactic_devel` - ROS Galactic
* `crystal-devel` - ROS Foxy, ROS Eloquent, ROS Dashing, ROS Crystal
* `melodic-devel` - ROS Noetic, ROS Melodic
* `kinetic-devel` - ROS Lunar, ROS Kinetic
* `groovy-devel` - ROS Jade, ROS Indigo, ROS Hydro, ROS Groovy
* `fuerte-devel` - ROS Fuerte
