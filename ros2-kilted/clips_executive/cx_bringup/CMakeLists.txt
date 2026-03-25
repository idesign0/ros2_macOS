cmake_minimum_required(VERSION 3.16...3.28)
project(cx_bringup)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(cx_ros_comm_gen REQUIRED)
find_package(cx_protobuf_plugin REQUIRED)

set(dependencies
  ament_cmake
  cx_ros_comm_gen
  cx_protobuf_plugin
)

# generate bindings to interface with ROS messages, such as std_msgs::msg::String
cx_generate_msg_bindings("std_msgs" "String")

# generate bindings to interface with ROS services, such as std_srvs::srv::SetBool
cx_generate_srv_bindings("std_srvs" "SetBool")

# generate bindings to interface with ROS services, such as example_interfaces::action::Fibonacci
cx_generate_action_bindings("example_interfaces" "Fibonacci")

# generate protobuf plugin with linked proto files
cx_generate_linked_protobuf_plugin_from_proto("BringupProtobufPlugin" proto/cx_bringup/SearchRequest.proto)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY launch params clips proto DESTINATION share/${PROJECT_NAME})

install(PROGRAMS
  scripts/test_service.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_export_dependencies(${dependencies})

ament_package()
