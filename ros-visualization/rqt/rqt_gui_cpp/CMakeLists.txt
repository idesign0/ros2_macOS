cmake_minimum_required(VERSION 3.5)
project(rqt_gui_cpp)

find_package(ament_cmake REQUIRED)

if(WIN32)
  message(STATUS "rqt_gui_cpp is not yet supported on Windows. Package will not be built.")
  ament_package()
  return()
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -fPIC)
endif()

# Qt5 boilerplate options from http://doc.qt.io/qt-5/cmake-manual.html
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(pluginlib REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(rclcpp REQUIRED)
find_package(qt_gui REQUIRED)
find_package(qt_gui_cpp REQUIRED)

ament_export_dependencies(
  Qt5Widgets
  pluginlib
  rclcpp
  qt_gui_cpp
  qt_gui)

add_library(${PROJECT_NAME} SHARED
  src/rqt_gui_cpp/nodelet_plugin_provider.cpp
  src/rqt_gui_cpp/roscpp_plugin_provider.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")

if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif()

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  Qt5Widgets
  pluginlib
  rclcpp
  qt_gui_cpp
  qt_gui)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(FILES plugin.xml
  DESTINATION share/${PROJECT_NAME}
)
pluginlib_export_plugin_description_file(qt_gui "plugin.xml")

# Export old-style CMake variables
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})

# Export modern CMake targets
ament_export_targets(export_${PROJECT_NAME})

ament_package()
