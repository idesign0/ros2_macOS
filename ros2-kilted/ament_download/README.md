# ament_download
Ament download package for ROS2-based CMakeLists

Largely taken from `catkin_download` available [here](https://github.com/ros/catkin/blob/kinetic-devel/cmake/catkin_download.cmake) under BSD-3.0. 

This is a temporary holding before potentially being merged into `ament/ament_cmake`.

Example use:

```
ament_download(http://download.ros.org/data/pcl/table_scene_lms400.pcd
  DESTINATION /home/s.macenski
  MD5 546b5b4822fb1de21b0cf83d41ad6683
  FILENAME table_scene_lms400.pcd
)
```
