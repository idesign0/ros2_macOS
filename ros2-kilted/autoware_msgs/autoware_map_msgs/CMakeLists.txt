cmake_minimum_required(VERSION 3.5)
project(autoware_map_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(msg_files
  "msg/AreaInfo.msg"
  "msg/LaneletMapBin.msg"
  "msg/LaneletMapMetaData.msg"
  "msg/LaneletMapCellMetaData.msg"
  "msg/MapProjectorInfo.msg"
  "msg/PointCloudMapCellWithID.msg"
  "msg/PointCloudMapCellMetaData.msg"
  "msg/PointCloudMapCellMetaDataWithID.msg"
  "msg/PointCloudMapMetaData.msg"
  "srv/GetPartialPointCloudMap.srv"
  "srv/GetDifferentialPointCloudMap.srv"
  "srv/GetSelectedPointCloudMap.srv"
  "srv/GetSelectedLanelet2Map.srv")

set(msg_dependencies
  std_msgs
  geometry_msgs
  geographic_msgs
  sensor_msgs)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES ${msg_dependencies}
  ADD_LINTER_TESTS)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
