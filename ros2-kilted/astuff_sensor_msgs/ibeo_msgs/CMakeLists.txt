project(ibeo_msgs)

find_package(ros_environment REQUIRED)

set(ROS_VERSION $ENV{ROS_VERSION})

set(MSG_FILES
  "CameraImage.msg"
  "ContourPointSigma.msg"
  "DeviceStatus.msg"
  "ErrorWarning.msg"
  "HostVehicleState2805.msg"
  "HostVehicleState2806.msg"
  "HostVehicleState2807.msg"
  "IbeoDataHeader.msg"
  "MountingPositionF.msg"
  "Object2221.msg"
  "Object2225.msg"
  "Object2270.msg"
  "Object2271.msg"
  "Object2280.msg"
  "ObjectData2221.msg"
  "ObjectData2225.msg"
  "ObjectData2270.msg"
  "ObjectData2271.msg"
  "ObjectData2280.msg"
  "Point2Df.msg"
  "Point2Di.msg"
  "Point2Dui.msg"
  "ResolutionInfo.msg"
  "ScanData2202.msg"
  "ScanData2204.msg"
  "ScanData2205.msg"
  "ScanData2208.msg"
  "ScannerInfo2204.msg"
  "ScannerInfo2205.msg"
  "ScanPoint2202.msg"
  "ScanPoint2204.msg"
  "ScanPoint2205.msg"
  "ScanPoint2208.msg"
  "Sigma2D.msg"
  "Size2D.msg"
  "TrackedProperties.msg"
  "UntrackedProperties.msg"
  "Velocity2D.msg"
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

  foreach(MSG_FILE ${MSG_FILES})
    execute_process(
      COMMAND /bin/bash -c "sed -i 's/builtin_interfaces\\/Time/time/g' msg/${MSG_FILE}"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
  endforeach()

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
