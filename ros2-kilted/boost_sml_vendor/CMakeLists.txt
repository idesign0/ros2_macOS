cmake_minimum_required(VERSION 3.18)
project(boost_sml_vendor)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_vendor_package REQUIRED)

ament_vendor(${PROJECT_NAME}
  VCS_URL https://github.com/boost-ext/sml.git
  VCS_VERSION v1.1.13
  VCS_TYPE git
)

# When this package is found, make sure to find the header files
ament_package(
  CONFIG_EXTRAS "boost_sml_vendor-extras.cmake"
)
