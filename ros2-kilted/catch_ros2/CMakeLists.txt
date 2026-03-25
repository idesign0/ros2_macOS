cmake_minimum_required(VERSION 3.8)
project(catch_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

##################
## DEPENDENCIES ##
##################
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)

###########
## Build ##
###########

# Base Library
add_library(catch_ros2 SHARED
  src/arguments.cpp src/catch_amalgamated.cpp
)
add_library(catch_ros2::catch_ros2 ALIAS catch_ros2)
target_include_directories(catch_ros2
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(catch_ros2 PUBLIC
  rclcpp::rclcpp
)
target_compile_definitions(catch_ros2
  PUBLIC CATCH_AMALGAMATED_CUSTOM_MAIN
)

# Library with Catch2 default main
add_library(catch_ros2_with_main SHARED
  src/default_main.cpp
)
add_library(catch_ros2::catch_ros2_with_main ALIAS catch_ros2_with_main)
target_link_libraries(catch_ros2_with_main PUBLIC
  catch_ros2
)
set_target_properties(catch_ros2_with_main
  PROPERTIES
  OUTPUT_NAME "catch_ros2_with_main"
)

# Library with custom main for running nodes
add_library(catch_ros2_with_node_main
  src/node_main.cpp
)
add_library(catch_ros2::catch_ros2_with_node_main ALIAS catch_ros2_with_node_main)
target_link_libraries(catch_ros2_with_node_main PUBLIC
  catch_ros2
)
set_target_properties(catch_ros2_with_node_main
  PROPERTIES
  OUTPUT_NAME "catch_ros2_with_node_main"
)

#############
## Install ##
#############

# install targets
install(TARGETS catch_ros2 catch_ros2_with_main catch_ros2_with_node_main
  EXPORT export_catch_ros2
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# install include directories
install(DIRECTORY include/ DESTINATION include/)

# Install launch utilities
ament_python_install_package(launch_catch_ros2
  PACKAGE_DIR launch_catch_ros2
  SETUP_CFG setup.cfg
)

# Install custom test runner
install(DIRECTORY scripts DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  # set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  # set(ament_cmake_cpplint_FOUND TRUE)

  # Exclude Catch2 files from linting
  list(APPEND CATCH2_LINT_EXCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/default_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/catch_amalgamated.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/catch_amalgamated.hpp
  )
  list(APPEND ament_cmake_cpplint_ADDITIONAL_EXCLUDE ${CATCH2_LINT_EXCLUDE_FILES})
  set(ament_cmake_uncrustify_ADDITIONAL_ARGS "--exclude")
  list(APPEND ament_cmake_uncrustify_ADDITIONAL_ARGS ${CATCH2_LINT_EXCLUDE_FILES})

  # Manually run copyright test, excluding Catch2 files since ROS 2 Humble does not have
  # any way to exclude files from an auto copyright test
  list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_copyright)
  find_package(ament_cmake_copyright)
  ament_copyright(
    TESTNAME manual_copyright
    EXCLUDE ${CATCH2_LINT_EXCLUDE_FILES}
  )

  ament_lint_auto_find_test_dependencies()




  # Set directories and include CMake function for use in examples
  set(catch_ros2_DIR ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/catch_ros2_add_integration_test.cmake)

  include(examples/integration_test/CMakeLists.txt)
  include(examples/unit_test/CMakeLists.txt)
  include(examples/custom_main/CMakeLists.txt)
endif()

# Export old-style CMake variables
ament_export_include_directories("include")
ament_export_libraries(catch_ros2 catch_ros2_with_main catch_ros2_with_node_main)

# Export modern CMake targets
ament_export_targets(export_catch_ros2)

ament_export_dependencies(
  rclcpp
)

ament_package(CONFIG_EXTRAS
  cmake/catch_ros2_add_integration_test.cmake
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)