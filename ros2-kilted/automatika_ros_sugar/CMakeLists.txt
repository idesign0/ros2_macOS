cmake_minimum_required(VERSION 3.10)
project(automatika_ros_sugar)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

set(msg_files

  "msg/ComponentStatus.msg"
)


set(service_files

  "srv/ChangeParameter.srv"
  "srv/ChangeParameters.srv"
  "srv/ConfigureFromFile.srv"
  "srv/ReplaceTopic.srv"
  "srv/ExecuteMethod.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${service_files}
  DEPENDENCIES builtin_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

# Install Python modules
ament_python_install_package(ros_sugar)

# Add executables
# Create service from recipe script
install(PROGRAMS
scripts/create_service
DESTINATION lib/${PROJECT_NAME}
)

# Backend UI node executable
install(PROGRAMS
scripts/ui_node_executable
DESTINATION lib/${PROJECT_NAME}
)

#NOTE: Remove testing for ROS build farm until required package names are merged in rosdistro
# if(BUILD_TESTING)
#   find_package(ament_lint_auto REQUIRED)
#   find_package(ament_cmake_pytest REQUIRED)
#   ament_lint_auto_find_test_dependencies()
#
#   # pytest scripts are to be found in the source tree.
#   ament_add_pytest_test(events_pytest "test/events_test.py"
#     PYTHON_EXECUTABLE "${_PYTHON_EXECUTABLE}"
#     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test"
#   )
#
#   ament_add_pytest_test(actions_pytest "test/actions_test.py"
#     PYTHON_EXECUTABLE "${_PYTHON_EXECUTABLE}"
#     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test"
#   )
#
#   ament_add_pytest_test(component_timed_pytest "test/component/timed_run_test.py"
#     PYTHON_EXECUTABLE "${_PYTHON_EXECUTABLE}"
#     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test"
#   )
#
#   ament_add_pytest_test(component_server_pytest "test/component/server_run_test.py"
#     PYTHON_EXECUTABLE "${_PYTHON_EXECUTABLE}"
#     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test"
#   )
# endif()

ament_package()
