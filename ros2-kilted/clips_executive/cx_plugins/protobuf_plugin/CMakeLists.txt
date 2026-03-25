cmake_minimum_required(VERSION 3.16...3.28)
project(cx_protobuf_plugin)

# Uses c++ 20 or above
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(cx_plugin REQUIRED)
find_package(cx_utils REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ProtobufComm REQUIRED)
find_package(clips_vendor REQUIRED)
find_package(clips REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

set(dependencies
  ament_cmake
  rclcpp
  cx_plugin
  cx_utils
  pluginlib
)
install(
  FILES generator.py
  DESTINATION lib/${PROJECT_NAME}
)

include_directories(include)

pluginlib_export_plugin_description_file(cx_plugin protobuf_plugin.xml)

add_library(clips_protobuf_comm SHARED src/communicator.cpp)
target_link_libraries(
  clips_protobuf_comm protobuf::libprotobuf ProtobufComm::protobuf_comm
  ClipsNS::libclips_ns)
ament_target_dependencies(clips_protobuf_comm rclcpp_lifecycle cx_utils)

set_property(TARGET clips_protobuf_comm PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

add_library(${PROJECT_NAME} SHARED src/protobuf_plugin.cpp)
ament_target_dependencies(${PROJECT_NAME} ${dependencies})
target_link_libraries(${PROJECT_NAME} clips_protobuf_comm)
target_link_libraries(${PROJECT_NAME} ClipsNS::libclips_ns)

# END OF PLUGINS REGISTRATION

install(
  DIRECTORY include/
  DESTINATION include/
)

install(
  DIRECTORY templates
  DESTINATION share/${PROJECT_NAME}
)

install(
  FILES protobuf_plugin.xml
  DESTINATION share/${PROJECT_NAME}
)

install(
  DIRECTORY clips/
  DESTINATION share/${PROJECT_NAME}/clips/${PROJECT_NAME}
)

install(TARGETS
  ${PROJECT_NAME}
  clips_protobuf_comm
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)
install(FILES cmake/cx_protobuf_sub_plugin.cmake
        DESTINATION share/${PROJECT_NAME}/cmake)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME} clips_protobuf_comm)
ament_export_dependencies(${dependencies})

ament_package(
CONFIG_EXTRAS cmake/cx_protobuf_sub_plugin.cmake
)
