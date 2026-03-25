cmake_minimum_required(VERSION 3.8)
project(boost_geometry_util)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
find_package(rclcpp REQUIRED)

ament_auto_find_build_dependencies()

ament_auto_add_library(boost_geometry_util SHARED
  src/geometries/linestring.cpp
  src/geometries/point.cpp
)

install(
  DIRECTORY "include/"
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_boost_geometry_util test/src/test.cpp)
  target_link_libraries(test_boost_geometry_util boost_geometry_util)
endif()

ament_auto_package()
