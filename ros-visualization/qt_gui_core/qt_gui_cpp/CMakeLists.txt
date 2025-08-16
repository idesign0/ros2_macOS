cmake_minimum_required(VERSION 3.12)
project(qt_gui_cpp)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake REQUIRED)

if(WIN32)
  message(STATUS "${PROJECT_NAME} is not yet supported on Windows. Package will not be built.")
  ament_package()
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/plugin.xml"
    "<library path=\"src\"/>")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/plugin.xml"
    DESTINATION share/${PROJECT_NAME})
  return()
endif()

find_package(pluginlib REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(rcpputils REQUIRED)
find_package(tinyxml2_vendor REQUIRED)
find_package(TinyXML2 REQUIRED)

ament_python_install_package(${PROJECT_NAME}
  PACKAGE_DIR src/${PROJECT_NAME})

set(qt_gui_cpp_SRCS
  src/qt_gui_cpp/composite_plugin_provider.cpp
  src/qt_gui_cpp/generic_proxy.cpp
  src/qt_gui_cpp/plugin_bridge.cpp
  src/qt_gui_cpp/plugin_context.cpp
  src/qt_gui_cpp/plugin_descriptor.cpp
  src/qt_gui_cpp/plugin_provider.cpp
  src/qt_gui_cpp/recursive_plugin_provider.cpp
  src/qt_gui_cpp/settings.cpp
)

set(qt_gui_cpp_HDRS
  include/qt_gui_cpp/plugin.h
  include/qt_gui_cpp/plugin_bridge.h
  include/qt_gui_cpp/plugin_context.h
)

qt5_wrap_cpp(qt_gui_cpp_MOCS ${qt_gui_cpp_HDRS})

add_library(${PROJECT_NAME} SHARED ${qt_gui_cpp_SRCS} ${qt_gui_cpp_MOCS})
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")

target_link_libraries(${PROJECT_NAME}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  Qt5::Widgets
  pluginlib::pluginlib
  rcpputils::rcpputils
  tinyxml2::tinyxml2)

add_subdirectory(src/qt_gui_cpp_shiboken)
add_subdirectory(src/qt_gui_cpp_sip)

message(STATUS "Python binding generators: ${qt_gui_cpp_BINDINGS}")
if(NOT qt_gui_cpp_BINDINGS)
  message(FATAL_ERROR "No Python binding generator found.")
endif()

install(FILES plugin.xml
  DESTINATION share/${PROJECT_NAME})

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

ament_export_dependencies(pluginlib rcpputils TinyXML2)
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# TODO(brawner) add ament_lint_auto tests here. Much of the c++ code needs to
# be updated for ROS2 style

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)

  ament_add_pytest_test(qt_gui_cpp test
    APPEND_ENV PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 90)
endif()

ament_package(CONFIG_EXTRAS cmake/${PROJECT_NAME}-extras.cmake)
