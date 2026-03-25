cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-c_no_deps VERSION 0.1.0)
find_package(gz-cmake4 REQUIRED)
gz_configure_project(
  REPLACE_INCLUDE_PATH gz/c_no_deps
)
gz_configure_build(QUIT_IF_BUILD_ERRORS)
gz_create_packages()
gz_create_docs()
