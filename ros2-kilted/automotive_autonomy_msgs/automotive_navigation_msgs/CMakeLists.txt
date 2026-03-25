project(automotive_navigation_msgs)

find_package(ros_environment REQUIRED)

set(ROS_VERSION $ENV{ROS_VERSION})

set(MSG_FILES
  "CommandWithHandshake.msg"
  "DesiredDestination.msg"
  "Direction.msg"
  "DistanceToDestination.msg"
  "LaneBoundary.msg"
  "LaneBoundaryArray.msg"
  "ModuleState.msg"
  "PointOfInterestArray.msg"
  "PointOfInterest.msg"
  "PointOfInterestRequest.msg"
  "PointOfInterestResponse.msg"
  "PointOfInterestStatus.msg"
  "RoadNetworkBoundaries.msg"
)

if(${ROS_VERSION} EQUAL 1)

  cmake_minimum_required(VERSION 2.8.3)

  find_package(catkin REQUIRED
    COMPONENTS
    message_generation
    std_msgs
    geometry_msgs
  )

  ## Declare ROS messages and services
  add_message_files(DIRECTORY msg FILES
    ${MSG_FILES}
    DIRECTORY msg
  )

  add_service_files(DIRECTORY srv FILES
    GetImageForMapTile.srv
  )

  ## Generate added messages and services
  generate_messages(DEPENDENCIES std_msgs geometry_msgs)

  ## Declare a catkin package
  catkin_package(CATKIN_DEPENDS
    message_runtime
    std_msgs
    geometry_msgs
  )

  install(DIRECTORY migration
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/
  )

elseif(${ROS_VERSION} EQUAL 2)

  cmake_minimum_required(VERSION 3.5)

  if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD 14)
  endif()

  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
  endif()

  find_package(ament_cmake REQUIRED)
  find_package(builtin_interfaces REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(rosidl_default_generators REQUIRED)

  # Apend "msg/" to each file name
  set(TEMP_LIST "")
  foreach(MSG_FILE ${MSG_FILES})
    list(APPEND TEMP_LIST "msg/${MSG_FILE}")
  endforeach()
  set(MSG_FILES ${TEMP_LIST})

  rosidl_generate_interfaces(${PROJECT_NAME}
    ${MSG_FILES}
    DEPENDENCIES
      builtin_interfaces
      geometry_msgs
      std_msgs
    ADD_LINTER_TESTS
  )

  ament_export_dependencies(rosidl_default_runtime)

  if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()
  endif()

  install(DIRECTORY migration
    DESTINATION share/${PROJECT_NAME}/
  )

  ament_package()

endif()
