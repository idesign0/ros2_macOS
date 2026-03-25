cmake_minimum_required(VERSION 3.14)
project(autoware_adapi_version_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/InterfaceVersion.srv"
)

ament_auto_package()
