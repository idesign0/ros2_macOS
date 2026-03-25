# ROS Apriltag Detector

![banner image](images/apriltags.png)

This repository holds the following ROS2 packages for detecting and displaying [Apriltags](https://april.eecs.umich.edu/software/apriltag):

  - [apriltag_detector](./apriltag_detector/README.md): base class definitions for 
    plugable detector libraries, node, composable component, and
    launch files for detecting and displaying apriltags. This is the package directly accessed by the end user.\
    [![Build Status](https://build.ros2.org/buildStatus/icon?job=Hdev__apriltag_detector__ubuntu_jammy_amd64&subject=Humble)](https://build.ros2.org/job/Hdev__apriltag_detector__ubuntu_jammy_amd64/)
    [![Build Status](https://build.ros2.org/buildStatus/icon?job=Jdev__apriltag_detector__ubuntu_noble_amd64&subject=Jazzy)](https://build.ros2.org/job/Jdev__apriltag_detector__ubuntu_noble_amd64/)
    [![Build Status](https://build.ros2.org/buildStatus/icon?job=Rdev__apriltag_detector__ubuntu_noble_amd64&subject=Rolling)](https://build.ros2.org/job/Rdev__apriltag_detector__ubuntu_noble_amd64/)
 
  The following packages are accessed mostly through the above [apriltag_detector](./apriltag_detector/README.md) package.

  - [apriltag_draw](./apriltag_draw/README.md): components for drawing detected Apriltags onto images.
  - [apriltag_detector_umich](./apriltag_detector_umich/README.md): provides plugin for running the UMich tag detector.
  - [apriltag_detector_mit](./apriltag_detector_mit/README.md): provides plugin for runninng the MIT tag detector.

The software in this repository does strictly perception, *no camera pose estimation*!
It is typically used when no camera calibration is available, or is not needed.
If you want perception and camera pose together, use [this package](https://github.com/christianrauch/apriltag_ros),
which uses the same tag message format.

## Installation

### From packages

```
apt install ros-${ROS_DISTRO}-apriltag-detector ros-${ROS_DISTRO}-apriltag-draw \
            ros-${ROS_DISTRO}-apriltag-detector-umich ros-${ROS_DISTRO}-apriltag-detector-mit
```

### From source

The build instructions follow the standard procedure for ROS2. Set the following shell variables:

```bash
repo=apriltag_detector
url=https://github.com/ros-misc-utilities/${repo}.git
```
and follow the ROS2 build instructions [here](https://github.com/ros-misc-utilities/.github/blob/master/docs/build_ros_repository.md)

Make sure to source your workspace's ``install/setup.bash`` afterwards.

## License

This software is issued under the Apache License Version 2.0.
