cmake_minimum_required(VERSION 3.14)
project(autoware_v2x_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/VirtualGateAreaCommand.msg"
  "msg/VirtualGateAreaStatus.msg"
  "msg/VirtualGateCommand.msg"
  "msg/VirtualGateStatus.msg"
  DEPENDENCIES
    builtin_interfaces
    geometry_msgs
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
