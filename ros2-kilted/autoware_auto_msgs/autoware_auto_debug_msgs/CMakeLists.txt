cmake_minimum_required(VERSION 3.5)

project(autoware_auto_debug_msgs)

# Generate messages
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MultiArrayDimension.idl"
  "msg/MultiArrayLayout.idl"
  "msg/BoolStamped.idl"
  "msg/Float32Stamped.idl"
  "msg/Float32MultiArrayStamped.idl"
  "msg/Float64Stamped.idl"
  "msg/Float64MultiArrayStamped.idl"
  "msg/Int32Stamped.idl"
  "msg/Int32MultiArrayStamped.idl"
  "msg/Int64Stamped.idl"
  "msg/Int64MultiArrayStamped.idl"
  "msg/StringStamped.idl"
  DEPENDENCIES
    "builtin_interfaces"
  ADD_LINTER_TESTS
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
