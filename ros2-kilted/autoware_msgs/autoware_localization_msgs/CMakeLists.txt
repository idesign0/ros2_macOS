cmake_minimum_required(VERSION 3.5)
project(autoware_localization_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(msg_files
  "msg/KinematicState.msg"
  "srv/InitializeLocalization.srv")

set(msg_dependencies
  autoware_common_msgs
  std_msgs
  geometry_msgs)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES ${msg_dependencies}
  ADD_LINTER_TESTS)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
