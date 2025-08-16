cmake_minimum_required(VERSION 3.5)
project(qt_gui_app)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

install(PROGRAMS scripts/qt_gui_app
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)

  # Disabling copyright test. The copyright used in this package does not conform to
  # ament's copyright tests
  set(ament_cmake_copyright_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
