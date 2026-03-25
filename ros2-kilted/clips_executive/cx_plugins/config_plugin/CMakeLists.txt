cmake_minimum_required(VERSION 3.16...3.28)
project(cx_config_plugin)

# Default to C++20
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(cx_plugin REQUIRED)
find_package(cx_utils REQUIRED)
find_package(clips_vendor REQUIRED)
find_package(clips REQUIRED)
find_package(yaml-cpp REQUIRED)

set(dependencies
  ament_cmake
  ament_index_cpp
  cx_plugin
  cx_utils
  yaml-cpp
  pluginlib
)

include_directories(include)

pluginlib_export_plugin_description_file(cx_plugin config_plugin.xml)

# END OF PLUGINS REGISTRATION
add_library(${PROJECT_NAME} SHARED src/config_plugin.cpp)
ament_target_dependencies(${PROJECT_NAME} ${dependencies})
target_link_libraries(${PROJECT_NAME} ClipsNS::libclips_ns)
target_link_libraries(${PROJECT_NAME} yaml-cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

install(
  DIRECTORY include/
  DESTINATION include/
)

install(
  DIRECTORY clips/
  DESTINATION share/${PROJECT_NAME}/clips/${PROJECT_NAME}
)

install(
  FILES config_plugin.xml
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS
  ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(${dependencies})

ament_package()
