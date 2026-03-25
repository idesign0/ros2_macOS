cmake_minimum_required(VERSION 3.5)
project(autoware_internal_planning_msgs)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CandidateTrajectories.msg"
  "msg/CandidateTrajectory.msg"
  "msg/GeneratorInfo.msg"
  "msg/ScoredCandidateTrajectories.msg"
  "msg/ScoredCandidateTrajectory.msg"
  "msg/PathPointWithLaneId.msg"
  "msg/PathWithLaneId.msg"
  "msg/Scenario.msg"
  "msg/ControlPoint.msg"
  "msg/SafetyFactor.msg"
  "msg/SafetyFactorArray.msg"
  "msg/PlanningFactor.msg"
  "msg/PlanningFactorArray.msg"
  "msg/RouteState.msg"
  "msg/VelocityLimit.msg"
  "msg/VelocityLimitConstraints.msg"
  "msg/VelocityLimitClearCommand.msg"
  "srv/ClearRoute.srv"
  "srv/LoadPlugin.srv"
  "srv/SetLaneletRoute.srv"
  "srv/SetWaypointRoute.srv"
  "srv/UnloadPlugin.srv"
  DEPENDENCIES
    builtin_interfaces
    geometry_msgs
    std_msgs
    unique_identifier_msgs
    autoware_common_msgs
    autoware_perception_msgs
    autoware_planning_msgs
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
