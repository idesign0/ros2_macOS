project(kartech_linear_actuator_msgs)

find_package(ros_environment REQUIRED)

set(ROS_VERSION $ENV{ROS_VERSION})

set(MSG_FILES
  "ActuatorUniqueIdReq.msg"
  "AutoZeroCalCmd.msg"
  "ConfigureOutputsKdFreqDeadbandCmd.msg"
  "ConfigureOutputsKpKiCmd.msg"
  "ConfigureOutputsPwmFreqCmd.msg"
  "EnhancedPositionRpt.msg"
  "MotorCurrentRpt.msg"
  "MotorOverCurrentConfigCmd.msg"
  "PositionCmd.msg"
  "PositionReachErrorTimeConfigCmd.msg"
  "PositionRpt.msg"
  "PriorityConfigCmd.msg"
  "ReassignCommandIdCmd.msg"
  "ReassignReportIdCmd.msg"
  "ReportIndex.msg"
  "ReportPollReq.msg"
  "ResetCmd.msg"
  "ScheduledReportRatesReq.msg"
  "SoftwareRevisionRpt.msg"
  "SoftwareVersionReq.msg"
  "UniqueDeviceIdRpt.msg"
  "ZeroingMessageRpt.msg"
)

if(${ROS_VERSION} EQUAL 1)

  cmake_minimum_required(VERSION 2.8.3)

  # Default to C++11
  if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
  endif()

  find_package(catkin REQUIRED
    COMPONENTS
    message_generation
    std_msgs
  )

  add_message_files(FILES
    ${MSG_FILES}
    DIRECTORY msg
  )

  generate_messages(DEPENDENCIES std_msgs)

  catkin_package(
    CATKIN_DEPENDS message_runtime
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
    DEPENDENCIES builtin_interfaces std_msgs
    ADD_LINTER_TESTS
  )

  ament_export_dependencies(rosidl_default_runtime)

  if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()
  endif()

  ament_package()

endif()
