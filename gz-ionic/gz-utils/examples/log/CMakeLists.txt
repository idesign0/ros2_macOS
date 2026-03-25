cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-utils-logger-demo)

# Find the Gazebo Libraries used directly by the example
find_package(gz-utils3 REQUIRED COMPONENTS log)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(
  ${PROJECT_NAME}
  gz-utils${GZ_UTILS_VER}::log
)
