cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(no_gz_prefix VERSION 0.1.0)
find_package(gz-cmake4 REQUIRED)
gz_configure_project(
  NO_PROJECT_PREFIX
  REPLACE_INCLUDE_PATH no_gz)
gz_configure_build(QUIT_IF_BUILD_ERRORS)
gz_create_packages()
gz_create_docs()
