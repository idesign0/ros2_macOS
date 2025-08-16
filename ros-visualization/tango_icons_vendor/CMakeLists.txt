cmake_minimum_required(VERSION 3.5)
project(tango_icons_vendor)

find_package(ament_cmake REQUIRED)

if(WIN32 OR APPLE)
  set(INSTALL_TANGO_ICONS_DEFAULT_VALUE TRUE)
else()
  set(INSTALL_TANGO_ICONS_DEFAULT_VALUE FALSE)
endif()
option(INSTALL_TANGO_ICONS "Install Tango icons" ${INSTALL_TANGO_ICONS_DEFAULT_VALUE})

if(INSTALL_TANGO_ICONS)
  install(DIRECTORY resource
    DESTINATION share/${PROJECT_NAME})
endif()

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
