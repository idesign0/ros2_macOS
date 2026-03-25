cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-utils3 VERSION 3.1.1)

#============================================================================
# Find gz-cmake
#============================================================================
# If you get an error at this line, you need to install gz-cmake
find_package(gz-cmake4 REQUIRED)

#============================================================================
# Configure the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Set project-specific options
#============================================================================
option(
  GZ_UTILS_VENDOR_CLI11
  "If true, use the vendored version of CLI11, otherwise use an external one"
  true)

#============================================================================
# Search for project-specific dependencies
#============================================================================

if(NOT GZ_UTILS_VENDOR_CLI11)
  # PKGCONFIG_IGNORE can be removed once pkg-config support in CLI11 (see
  # https://github.com/CLIUtils/CLI11/pull/523) is released
  gz_find_package(CLI11 REQUIRED_BY cli PKGCONFIG_IGNORE)
endif()

gz_find_package(
  spdlog
  PKGCONFIG spdlog
  REQUIRED_BY log
  PURPOSE "Provide logging")

#============================================================================
# Configure the build
#============================================================================
gz_configure_build(QUIT_IF_BUILD_ERRORS
  COMPONENTS cli log)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()

#============================================================================
# Configure documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md")
