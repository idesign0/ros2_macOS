cmake_minimum_required(VERSION 3.8)

project(camera_aravis2_msgs)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)

file(GLOB msg_files
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/"
  "msg/*.msg"
)
message(TRACE "Message files found: ${msg_files}")

file(GLOB srv_files
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/"
  "srv/*.srv"
)
message(TRACE "Service files found: ${srv_files}")

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES 
  builtin_interfaces 
  std_msgs 
  sensor_msgs
  diagnostic_msgs
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
