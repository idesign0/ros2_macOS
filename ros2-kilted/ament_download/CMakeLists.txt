cmake_minimum_required(VERSION 3.5)
project(ament_download)

find_package(ament_cmake REQUIRED)


set(AMENT_BUILD_CONFIGURATION_KEYWORD_SEPARATOR ":")
include("cmake/ament_download.cmake")

# install cmake files for ament
install(DIRECTORY cmake 
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(DIRECTORY src/ 
  DESTINATION share/${PROJECT_NAME}/cmake
  USE_SOURCE_PERMISSIONS
)

ament_export_dependencies(ament_cmake)
ament_package(CONFIG_EXTRAS "ament_download_dependencies-extras.cmake")

