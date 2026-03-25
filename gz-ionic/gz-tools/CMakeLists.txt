cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-tools2 VERSION 2.0.3)

#============================================================================
# Find gz-cmake
#============================================================================
find_package(gz-cmake3 QUIET)
if (NOT ${gz-cmake3_FOUND})
  find_package(gz-cmake4 QUIET)
  if (NOT ${gz-cmake4_FOUND})
    find_package(gz-cmake QUIET)
    if (NOT ${gz-cmake_FOUND})
      message(FATAL_ERROR "Could not find gz-cmake3, gz-cmake4, or gz-cmake")
    endif()
  endif()
endif()

#============================================================================
# Configure the project
#============================================================================
gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Set project-specific options
#============================================================================

# No project-specific options yet

#============================================================================
# Search for project-specific dependencies
#============================================================================
message(STATUS "\n\n-- ====== Finding Dependencies ======")

# Rubocop for Ruby linting
find_program(RUBOCOP_PROGRAM rubocop)

# Backtrace
# There appears to be a bug that causes pretty printing backtrace to fail in
# some instances when using BFD, so we disable it here.

option(USE_SYSTEM_BACKWARDCPP "Use system installed backward-cpp, otherwise use vendored version" FALSE)
if (NOT USE_SYSTEM_BACKWARDCPP)
  list(APPEND CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/src/Backward)
  set(STACK_DETAILS_AUTO_DETECT FALSE CACHE BOOL "Auto detect backward's stack details dependencies")
endif()
find_package(Backward)

add_subdirectory(etc)

#============================================================================
# Configure the build
#============================================================================
gz_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()

#============================================================================
# Configure documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md")
