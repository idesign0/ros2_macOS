cmake_minimum_required(VERSION 3.5)
project(aruco_opencv)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(aruco_opencv_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV ${OpenCV_VERSION} REQUIRED COMPONENTS aruco)
find_package(yaml-cpp REQUIRED)

add_library(${PROJECT_NAME} SHARED
  src/aruco_tracker.cpp
  src/detector.cpp
  src/board_loader.cpp
  src/parameters.cpp
  src/utils.cpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC
  rclcpp::rclcpp
  rclcpp_lifecycle::rclcpp_lifecycle
  rclcpp_components::component
  tf2_ros::tf2_ros
  tf2_geometry_msgs::tf2_geometry_msgs
  image_transport::image_transport
  opencv_aruco
  ${YAML_CPP_LIBRARIES}
  ${aruco_opencv_msgs_TARGETS}
  cv_bridge::cv_bridge
)
target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${YAML_CPP_INCLUDE_DIRS}
)
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "aruco_opencv::ArucoTracker"
  EXECUTABLE "aruco_tracker"
)
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "aruco_opencv::ArucoTrackerAutostart"
  EXECUTABLE "aruco_tracker_autostart"
)

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(
  DIRECTORY
    config
    launch
  DESTINATION share/${PROJECT_NAME}
)

install(
  PROGRAMS
    scripts/create_board
    scripts/create_marker
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
