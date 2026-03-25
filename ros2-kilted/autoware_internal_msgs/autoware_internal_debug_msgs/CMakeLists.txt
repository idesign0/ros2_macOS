cmake_minimum_required(VERSION 3.14)
project(autoware_internal_debug_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(msg_files
  "msg/BoolStamped.msg"
  "msg/Float32Stamped.msg"
  "msg/Float32MultiArrayStamped.msg"
  "msg/Float64Stamped.msg"
  "msg/Float64MultiArrayStamped.msg"
  "msg/Int32Stamped.msg"
  "msg/Int32MultiArrayStamped.msg"
  "msg/Int64Stamped.msg"
  "msg/Int64MultiArrayStamped.msg"
  "msg/StringStamped.msg"
  "msg/ProcessingTimeNode.msg"
  "msg/ProcessingTimeTree.msg"
  "msg/ServiceLog.msg"
)

set(srv_files
  "srv/String.srv"
)

set(msg_dependencies
  builtin_interfaces
  std_msgs
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES ${msg_dependencies}
  ADD_LINTER_TESTS
)

ament_auto_package()
