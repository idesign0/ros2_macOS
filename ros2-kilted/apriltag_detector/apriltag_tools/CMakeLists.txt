#
# Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

cmake_minimum_required(VERSION 3.16)
project(apriltag_tools)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc)
find_package(apriltag_detector REQUIRED)
find_package(apriltag_draw REQUIRED)
find_package(apriltag_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosbag2_transport REQUIRED)

set(detect_from_bag_ament_dependencies
  apriltag_detector::apriltag_detector_component
  apriltag_draw::apriltag_draw
  ${apriltag_msgs_TARGETS}
  rclcpp::rclcpp
  rosbag2_transport::rosbag2_transport)


if(${rosbag2_transport_VERSION} VERSION_GREATER_EQUAL "0.20.0")
  add_definitions(-DHAS_GET_PUBLISHERS)
endif()

#
# -------- nodes
#
add_executable(detect_from_bag src/detect_from_bag.cpp)
target_link_libraries(detect_from_bag ${detect_from_bag_ament_dependencies})

# the node must go into the project specific lib directory

install(TARGETS detect_from_bag DESTINATION lib/${PROJECT_NAME}/)

if(BUILD_TESTING)
  find_package(ament_cmake REQUIRED)
  find_package(ament_cmake_copyright REQUIRED)
  find_package(ament_cmake_cppcheck REQUIRED)
  find_package(ament_cmake_cpplint REQUIRED)
  find_package(ament_cmake_clang_format REQUIRED)
  find_package(ament_cmake_flake8 REQUIRED)
  find_package(ament_cmake_lint_cmake REQUIRED)
  find_package(ament_cmake_pep257 REQUIRED)
  find_package(ament_cmake_xmllint REQUIRED)

  ament_copyright()
  ament_cppcheck(LANGUAGE c++)
  ament_cpplint(FILTERS "-build/include,-runtime/indentation_namespace")
  ament_clang_format(CONFIG_FILE .clang-format)
  ament_flake8()
  ament_lint_cmake()
  ament_pep257()
  ament_xmllint()
endif()

ament_package()
