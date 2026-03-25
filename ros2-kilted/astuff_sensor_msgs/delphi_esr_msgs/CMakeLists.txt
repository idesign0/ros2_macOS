project(delphi_esr_msgs)

find_package(ros_environment REQUIRED)

set(ROS_VERSION $ENV{ROS_VERSION})

set(MSG_FILES
  "EsrEthTx.msg"
  "EsrStatus1.msg"
  "EsrStatus2.msg"
  "EsrStatus3.msg"
  "EsrStatus4.msg"
  "EsrStatus5.msg"
  "EsrStatus6.msg"
  "EsrStatus7.msg"
  "EsrStatus8.msg"
  "EsrStatus9.msg"
  "EsrTrack.msg"
  "EsrTrackMotionPowerTrack.msg"
  "EsrTrackMotionPowerGroup.msg"
  "EsrValid1.msg"
  "EsrValid2.msg"
  "EsrVehicle1.msg"
  "EsrVehicle2.msg"
  "EsrVehicle3.msg"
  "EsrVehicle4.msg"
  "EsrVehicle5.msg"
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

  install(DIRECTORY migration
    DESTINATION share/${PROJECT_NAME}/
  )

  ament_package()

endif()
