#
# Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.16)
project(apriltag_detector)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(apriltag_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(pluginlib)
find_package(rclcpp)
find_package(rclcpp_components)
find_package(sensor_msgs REQUIRED)

if(${rclcpp_VERSION} VERSION_GREATER_EQUAL "28.0.0")
  add_definitions(-DUSE_MATCHED_EVENTS)
endif()

if(${image_transport_VERSION} VERSION_GREATER_EQUAL "6.3.0")
  add_definitions(-DIMAGE_TRANSPORT_USE_QOS)
endif()

if(${image_transport_VERSION} VERSION_GREATER_EQUAL "6.4.0")
  add_definitions(-DIMAGE_TRANSPORT_USE_NODEINTERFACE)
endif()

#
# --------- header-only library
#
add_library(${PROJECT_NAME} INTERFACE include/apriltag_detector/detector.hpp)
target_link_libraries(${PROJECT_NAME} INTERFACE ${sensor_msgs_TARGETS} ${apriltag_msgs_TARGETS})

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
#
# --------- library with detector component
#
add_library(${PROJECT_NAME}_component SHARED src/detector_component.cpp)
target_link_libraries(${PROJECT_NAME}_component ${sensor_msgs_TARGETS}
  ${apriltag_msgs_TARGETS}
  image_transport::image_transport
  cv_bridge::cv_bridge
  pluginlib::pluginlib
  rclcpp_components::component)

target_include_directories(
  ${PROJECT_NAME}_component
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

rclcpp_components_register_nodes(${PROJECT_NAME}_component
  "${PROJECT_NAME}::DetectorComponent")
#
# -------- node
#
add_executable(${PROJECT_NAME}_node src/detector_node.cpp)
target_link_libraries(${PROJECT_NAME}_node ${PROJECT_NAME}_component)

# package-wide ament exports
ament_export_targets(${PROJECT_NAME}_export HAS_LIBRARY_TARGET)
ament_export_dependencies(sensor_msgs apriltag_msgs image_transport cv_bridge
                          pluginlib rclcpp_components)

# the shared library goes into the global lib dir so it can be used as a
# composable node by other projects

install(TARGETS
  ${PROJECT_NAME}
  ${PROJECT_NAME}_component
  EXPORT ${PROJECT_NAME}_export
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# the node must go into the project specific lib directory or else the launch
# file will not find it

install(TARGETS ${PROJECT_NAME}_node DESTINATION lib/${PROJECT_NAME}/)

install(DIRECTORY include/ DESTINATION include)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
  FILES_MATCHING
  PATTERN "*.py")

if(BUILD_TESTING)
  find_package(ament_cmake REQUIRED)
  find_package(ament_cmake_copyright REQUIRED)
  find_package(ament_cmake_cppcheck REQUIRED)
  find_package(ament_cmake_cpplint REQUIRED)
  find_package(ament_cmake_clang_format REQUIRED)
  find_package(ament_cmake_lint_cmake REQUIRED)
  find_package(ament_cmake_xmllint REQUIRED)

  ament_copyright()
  ament_cppcheck(LANGUAGE c++)
  ament_cpplint(FILTERS "-build/include,-runtime/indentation_namespace")
  ament_clang_format(CONFIG_FILE .clang-format)
  ament_lint_cmake()
  ament_xmllint()
endif()

ament_package()
