cmake_minimum_required(VERSION 3.16...3.28)
project(cx_pddl_bringup)

# find dependencies
find_package(ament_cmake REQUIRED)

install(DIRECTORY params pddl launch clips DESTINATION share/${PROJECT_NAME})

ament_package()
