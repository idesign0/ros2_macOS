cmake_minimum_required(VERSION 3.16...3.28)
project(cx_ros_comm_gen)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY templates
  DESTINATION share/${PROJECT_NAME}
)

install(
  FILES generator.py
  DESTINATION lib/${PROJECT_NAME}
)

install(FILES cmake/cx_generate_comm.cmake
        DESTINATION share/${PROJECT_NAME}/cmake)

ament_package(
CONFIG_EXTRAS cmake/cx_generate_comm.cmake
)
