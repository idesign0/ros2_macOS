cmake_minimum_required(VERSION 3.5)
project(adaptive_component)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unknown-pragmas)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)

include_directories(include)

# create ament index resource which references the libraries in the binary dir
set(node_plugins "")

set(COMPONENT_AMENT_DEPENDENCIES
  rclcpp
  rclcpp_components)

## Component
add_library(${PROJECT_NAME} SHARED src/adaptive_component.cpp)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE "COMPOSITION_BUILDING_DLL")
ament_target_dependencies(${PROJECT_NAME} ${COMPONENT_AMENT_DEPENDENCIES})
rclcpp_components_register_nodes(${PROJECT_NAME} "composition::AdaptiveComponent")
set(node_plugins "${node_plugins}composition::AdaptiveComponent;$<TARGET_FILE:adaptive_component>\n")

# Make the component library reusable in other downstream packages
#  see https://docs.ros.org/en/foxy/How-To-Guides/Ament-CMake-Documentation.html#building-a-library
#
# exports the targets for CMake, aka, allow library’s clients to use
# the target_link_libraries(client my_library::my_library) syntax.
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})

# install components
install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# install headers
install(
      DIRECTORY include/
      DESTINATION include/${PROJECT_NAME}
)


ament_package()
