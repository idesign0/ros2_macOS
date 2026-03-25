cmake_minimum_required(VERSION 3.5)
project(bag2_to_image)

find_package(ament_cmake REQUIRED)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rosbag2_storage REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(${PROJECT_NAME} SHARED
  src/bag2_to_image.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
)
target_link_libraries(${PROJECT_NAME}
  opencv_core
  opencv_imgcodecs
  rclcpp::rclcpp
  rclcpp_components::component
  rosbag2_cpp::rosbag2_cpp
  rosbag2_storage::rosbag2_storage
  "${sensor_msgs_TARGETS}"
)
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "bag2_to_image::Bag2ToImageNode"
  EXECUTABLE ${PROJECT_NAME}_exe
)

install(TARGETS
  ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(DIRECTORY include
  DESTINATION include/${PROJECT_NAME}
)

if(BUILD_TESTING)
  set(ament_cmake_clang_format_CONFIG_FILE .clang-format)
  find_package(ament_lint_auto REQUIRED)
  list(APPEND AMENT_LINT_AUTO_EXCLUDE
    ament_cmake_uncrustify
  )
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_dependencies(
  OpenCV
  rclcpp
  rclcpp_components
  rosbag2_cpp
  rosbag2_storage
  sensor_msgs
)

ament_package()
