cmake_minimum_required(VERSION 3.8)
project(camera_aravis2)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to Release as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Default to C11
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)

find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(camera_info_manager REQUIRED)

find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(camera_aravis2_msgs REQUIRED)

find_package(ARAVIS REQUIRED)
find_package(GLIB2 REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(include
  ${ARAVIS_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIRS}
)

#if 'humble' and 'iron' within the rclcpp_DIR path, define to build with matched events
if(NOT ${rclcpp_DIR} MATCHES ".*/humble/.*" AND NOT ${rclcpp_DIR} MATCHES ".*/iron/.*")
  set(COMMON_COMPILE_DEFINITIONS
    ${COMMON_COMPILE_DEFINITIONS}
    -DWITH_MATCHED_EVENTS
  )
endif()

# Library

set(LIBRARY_NAME ${PROJECT_NAME}_core)

set(DEPENDENCIES
  rclcpp
  rclcpp_components
  cv_bridge
  image_transport
  camera_info_manager
  std_msgs
  sensor_msgs
  diagnostic_msgs
  camera_aravis2_msgs
)

set(LIBRARIES
  ${ARAVIS_LIBRARIES}
  glib-2.0
  gmodule-2.0
  gobject-2.0
  cv_bridge::cv_bridge
  image_transport::image_transport
  yaml-cpp
)

add_library(${LIBRARY_NAME} SHARED
  src/camera_aravis_node_base.cpp
  src/camera_driver_gv.cpp
  src/camera_driver_uv.cpp
  src/camera_driver.cpp
  src/camera_xml_exporter.cpp
  src/conversion_utils.cpp
  src/error.cpp
  src/image_buffer_pool.cpp
)
target_link_libraries(${LIBRARY_NAME} ${LIBRARIES})
target_compile_definitions(${LIBRARY_NAME} PRIVATE ${COMMON_COMPILE_DEFINITIONS})
ament_target_dependencies(${LIBRARY_NAME} ${DEPENDENCIES})

# camera_driver_gv node

set(CAMERA_DRIVER_GV_EXEC_NAME camera_driver_gv)
add_executable(${CAMERA_DRIVER_GV_EXEC_NAME} src/camera_driver_gv_main.cpp)
target_compile_definitions(${CAMERA_DRIVER_GV_EXEC_NAME} PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_link_libraries(${CAMERA_DRIVER_GV_EXEC_NAME} ${LIBRARY_NAME})
rclcpp_components_register_nodes(${LIBRARY_NAME} "camera_aravis2::CameraDriverGv")

# camera_driver_uv node

set(CAMERA_DRIVER_UV_EXEC_NAME camera_driver_uv)
add_executable(${CAMERA_DRIVER_UV_EXEC_NAME} src/camera_driver_uv_main.cpp)
target_compile_definitions(${CAMERA_DRIVER_UV_EXEC_NAME} PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_link_libraries(${CAMERA_DRIVER_UV_EXEC_NAME} ${LIBRARY_NAME})
rclcpp_components_register_nodes(${LIBRARY_NAME} "camera_aravis2::CameraDriverUv")

# camera_finder node

set(CAMERA_FINDER_EXEC_NAME camera_finder)
add_executable(${CAMERA_FINDER_EXEC_NAME} src/camera_finder_main.cpp)
target_compile_definitions(${CAMERA_FINDER_EXEC_NAME} PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_link_libraries(${CAMERA_FINDER_EXEC_NAME} ${LIBRARY_NAME})

# camera_xml_exporter node

set(CAMERA_XML_EXPORTER_EXEC_NAME camera_xml_exporter)
add_executable(${CAMERA_XML_EXPORTER_EXEC_NAME} src/camera_xml_exporter_main.cpp)
target_compile_definitions(${CAMERA_XML_EXPORTER_EXEC_NAME} PRIVATE ${COMMON_COMPILE_DEFINITIONS})
target_link_libraries(${CAMERA_XML_EXPORTER_EXEC_NAME} ${LIBRARY_NAME})
rclcpp_components_register_nodes(${LIBRARY_NAME} "camera_aravis2::CameraXmlExporter")

# Installs

install(TARGETS ${LIBRARY_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS
  ${CAMERA_DRIVER_GV_EXEC_NAME}
  ${CAMERA_DRIVER_UV_EXEC_NAME}
  ${CAMERA_FINDER_EXEC_NAME}
  ${CAMERA_XML_EXPORTER_EXEC_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include/)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})

# Tests

if(BUILD_TESTING)
  find_package(ament_cmake_copyright REQUIRED)
  ament_copyright()
  find_package(ament_cmake_cppcheck REQUIRED)
  ament_cppcheck()
  find_package(ament_cmake_cpplint REQUIRED)
  ament_cpplint(FILTERS -whitespace/newline,-readability/braces,-readability/todo)
  find_package(ament_cmake_flake8 REQUIRED)
  ament_flake8()
  find_package(ament_cmake_lint_cmake REQUIRED)
  ament_lint_cmake()
endif()

ament_package()
