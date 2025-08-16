cmake_minimum_required(VERSION 3.5)
project(interactive_markers)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)

ament_python_install_package(${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED
  src/interactive_marker_server.cpp
  src/tools.cpp
  src/menu_handler.cpp
  src/interactive_marker_client.cpp
  src/message_context.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")
target_link_libraries(${PROJECT_NAME} PUBLIC
  rclcpp::rclcpp
  tf2::tf2
  ${visualization_msgs_TARGETS})
target_link_libraries(${PROJECT_NAME} PRIVATE
  rmw::rmw
  ${tf2_geometry_msgs_TARGETS})

install(TARGETS ${PROJECT_NAME} EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME}
  PRIVATE "INTERACTIVE_MARKERS_BUILDING_LIBRARY")

ament_export_dependencies("rclcpp")
ament_export_dependencies("tf2")
ament_export_dependencies("visualization_msgs")
ament_export_targets(export_${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  find_package(geometry_msgs REQUIRED)

  ament_lint_auto_find_test_dependencies()

  ament_add_gtest(test_interactive_marker_server
    test/${PROJECT_NAME}/test_interactive_marker_server.cpp
    TIMEOUT 90
  )
  target_link_libraries(test_interactive_marker_server
    ${PROJECT_NAME}
    ${geometry_msgs_TARGETS})

  ament_add_gtest(test_interactive_marker_client
    test/${PROJECT_NAME}/test_interactive_marker_client.cpp
  )
  target_link_libraries(test_interactive_marker_client
    ${PROJECT_NAME}
    ${geometry_msgs_TARGETS})
endif()

ament_package()
