cmake_minimum_required(VERSION 3.5)
project(angles)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # we dont use add_compile_options with pedantic in message packages
  # because the Python C extensions dont comply with it
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

find_package(ament_cmake REQUIRED)

add_library(angles INTERFACE)
target_include_directories(angles INTERFACE
  "$<INSTALL_INTERFACE:include/angles>")
if(WIN32)
  target_compile_definitions(angles INTERFACE _USE_MATH_DEFINES)
endif()

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(${PROJECT_NAME}_utest test/utest.cpp)
  target_include_directories(${PROJECT_NAME}_utest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(${PROJECT_NAME}_utest angles)

  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(utest test/utest.py)
endif()

install(TARGETS angles EXPORT export_angles)

# TODO(sloretz) remove when old-style CMake interface is no longer supported
ament_export_include_directories("include/angles")

ament_export_targets(export_angles)

install(DIRECTORY include/ DESTINATION include/angles)

ament_python_install_package(${PROJECT_NAME})

ament_package()
