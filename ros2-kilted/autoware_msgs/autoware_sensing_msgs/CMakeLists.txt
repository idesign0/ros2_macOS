cmake_minimum_required(VERSION 3.5)
project(autoware_sensing_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(msg_files
  "msg/ConcatenatedPointCloudInfo.msg"
  "msg/GnssInsOrientation.msg"
  "msg/GnssInsOrientationStamped.msg"
  "msg/RadarClassification.msg"
  "msg/RadarFieldInfo.msg"
  "msg/RadarInfo.msg"
  "msg/RadarObject.msg"
  "msg/RadarObjects.msg"
  "msg/SourcePointCloudInfo.msg")

set(msg_dependencies
  geometry_msgs
  sensor_msgs
  std_msgs)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES ${msg_dependencies}
  ADD_LINTER_TESTS)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
