cmake_minimum_required(VERSION 3.5)

project(cascade_lifecycle_msgs)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Activation.msg" 
  "msg/State.msg" 
 DEPENDENCIES builtin_interfaces lifecycle_msgs
 ADD_LINTER_TESTS
)

ament_export_dependencies(rosidl_default_runtime lifecycle_msgs builtin_interfaces)

ament_package()
