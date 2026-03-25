cmake_minimum_required(VERSION 3.5)

project(ament_vitis NONE)

find_package(ament_cmake_core REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(ament_acceleration REQUIRED)

ament_export_dependencies(ament_acceleration)

ament_package(
  CONFIG_EXTRAS "ament_vitis-extras.cmake"
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)
