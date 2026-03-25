# All rights reserved.
cmake_minimum_required(VERSION 3.5)

### Export headers
project(autoware_vehicle_msgs)

# Generate messages
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
    "msg/ControlModeReport.msg"
    "msg/Engage.msg"
    "msg/GearCommand.msg"
    "msg/GearReport.msg"
    "msg/HazardLightsCommand.msg"
    "msg/HazardLightsReport.msg"
    "msg/SteeringReport.msg"
    "msg/TurnIndicatorsCommand.msg"
    "msg/TurnIndicatorsReport.msg"
    "msg/VelocityReport.msg"
    "srv/ControlModeCommand.srv"
  DEPENDENCIES
    "autoware_planning_msgs"
    "builtin_interfaces"
    "std_msgs"
  ADD_LINTER_TESTS
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
