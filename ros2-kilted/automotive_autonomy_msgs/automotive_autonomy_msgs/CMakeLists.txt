project(automotive_autonomy_msgs)

find_package(ros_environment REQUIRED)

set(ROS_VERSION $ENV{ROS_VERSION})

if(${ROS_VERSION} EQUAL 1)
  cmake_minimum_required(VERSION 2.8.3)
  find_package(catkin REQUIRED)
  catkin_package()
elseif(${ROS_VERSION} EQUAL 2)
  cmake_minimum_required(VERSION 3.5)
  find_package(ament_cmake REQUIRED)
  ament_package()
endif()
