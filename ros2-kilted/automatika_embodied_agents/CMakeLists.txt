cmake_minimum_required(VERSION 3.10)
project(automatika_embodied_agents)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

file(GLOB_RECURSE MSG_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "msg/*.msg" )
file(GLOB_RECURSE ACTION_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "action/*.action" )

rosidl_generate_interfaces(${PROJECT_NAME}
  ${MSG_FILES}
  ${ACTION_FILES}
  DEPENDENCIES builtin_interfaces std_msgs sensor_msgs
)

ament_export_dependencies(rosidl_default_runtime)

# Install Python module
ament_python_install_package(agents)

# Add executable
install(PROGRAMS
scripts/executable
DESTINATION lib/${PROJECT_NAME}
)

ament_package()
