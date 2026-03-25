#
# Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
#

cmake_minimum_required(VERSION 3.16)

project(apriltag_mit VERSION 1.0.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# set(CMAKE_CXX_CLANG_TIDY clang-tidy)

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED core calib3d)
find_package(Boost REQUIRED headers)

file(GLOB CC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)
add_library(${PROJECT_NAME} SHARED ${CC_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC opencv_core opencv_calib3d Eigen3::Eigen Boost::headers)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)

target_include_directories(
    ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION include
  )

  install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# generate the ConfigVersion.cmake file that will be included by Config.cmake
write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
  )

# generate the *Config.cmake file
configure_file(cmake/${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake @ONLY)

# generate Targets.cmake file from exports learned during the installation
install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

# install the generated version and config files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )
