cmake_minimum_required(VERSION 3.16...3.28)
project(cx_tutorial_agents)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

install(DIRECTORY params clips DESTINATION share/${PROJECT_NAME})

ament_package()
