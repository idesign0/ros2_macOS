#.rst
# GzConfigureProject
# -------------------
#
# gz_configure_project([NO_PROJECT_PREFIX]
#                      [REPLACE_INCLUDE_PATH <new_include_path>]
#                      [CONFIG_EXTRAS <extra_config_files>]
#                      [VERSION_SUFFIX <pre|alpha|beta|etc>])
#
# Sets up a Gazebo library project.
#
# CONFIG_EXTRAS: Optional. If provided, the list that follows should indicate
#     extra cmake template files that will be configured and installed to the
#     same folder as the cmake configuration files for the core library target.
# NO_PROJECT_PREFIX: Optional. Don't use gz- as prefix in
#     cmake project name.
# REPLACE_INCLUDE_PATH: Optional. Specify include folder
#     names to replace the default value of
#     gz/${GZ_DESIGNATION}
# VERSION_SUFFIX: Optional. Specify a prerelease version suffix.
#
# The following variables are automatically defined by project(~) in cmake 3:
#   PROJECT_NAME
#   PROJECT_VERSION_MAJOR
#   PROJECT_VERSION_MINOR
#   PROJECT_VERSION_PATCH
#
# This macro defines the following variables as well:
#   GZ_DESIGNATION
#   GZ_DESIGNATION_LOWER
#   GZ_DESIGNATION_UPPER
#   PKG_NAME
#   PROJECT_CMAKE_EXTRAS_RELATIVE_INSTALL_DIR
#   PROJECT_CMAKE_EXTRAS_INSTALL_DIR
#   PROJECT_CMAKE_EXTRAS_PATH_TO_PREFIX
#   PROJECT_INCLUDE_DIR
#   PROJECT_NAME_NO_VERSION
#   PROJECT_NAME_NO_VERSION_LOWER
#   PROJECT_NAME_NO_VERSION_UPPER
#   PROJECT_NAME_LOWER
#   PROJECT_NAME_UPPER
#   PROJECT_VERSION
#   PROJECT_VERSION_FULL
#   PROJECT_VERSION_FULL_NO_SUFFIX
#   PROJECT_VERSION_SUFFIX
#
#===============================================================================
# Copyright (C) 2017 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#################################################
# Initialize the Gazebo project
macro(gz_configure_project)
  # Define the expected arguments
  set(options NO_PROJECT_PREFIX)
  set(oneValueArgs REPLACE_INCLUDE_PATH VERSION_SUFFIX)
  set(multiValueArgs CONFIG_EXTRAS)

  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_configure_project "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Note: The following are automatically defined by project(~) in cmake v3:
  # PROJECT_NAME
  # PROJECT_VERSION_MAJOR
  # PROJECT_VERSION_MINOR
  # PROJECT_VERSION_PATCH

  if(gz_configure_project_VERSION_SUFFIX)
    set(PROJECT_VERSION_SUFFIX ${gz_configure_project_VERSION_SUFFIX})
  endif()

  #============================================================================
  # Extract the designation
  #============================================================================
  set(GZ_DESIGNATION ${PROJECT_NAME})
  # Remove the leading project prefix ("gz-" by default)
  set(PROJECT_PREFIX "gz")
  string(REGEX REPLACE "${PROJECT_PREFIX}-" "" GZ_DESIGNATION ${GZ_DESIGNATION})

  # Remove the trailing version number
  string(REGEX REPLACE "[0-9]+" "" GZ_DESIGNATION ${GZ_DESIGNATION})

  #============================================================================
  # Set project variables
  #============================================================================

  if(gz_configure_project_NO_PROJECT_PREFIX)
    set(PROJECT_NAME_NO_VERSION ${GZ_DESIGNATION})
  else()
    set(PROJECT_NAME_NO_VERSION "${PROJECT_PREFIX}-${GZ_DESIGNATION}")
  endif()
  string(TOLOWER ${PROJECT_NAME_NO_VERSION} PROJECT_NAME_NO_VERSION_LOWER)
  string(TOUPPER ${PROJECT_NAME_NO_VERSION} PROJECT_NAME_NO_VERSION_UPPER)
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
  string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
  string(TOLOWER ${GZ_DESIGNATION} GZ_DESIGNATION_LOWER)
  string(TOUPPER ${GZ_DESIGNATION} GZ_DESIGNATION_UPPER)

  string(SUBSTRING ${GZ_DESIGNATION} 0 1 GZ_DESIGNATION_FIRST_LETTER)
  string(TOUPPER ${GZ_DESIGNATION_FIRST_LETTER} GZ_DESIGNATION_FIRST_LETTER)
  string(REGEX REPLACE "^.(.*)" "${GZ_DESIGNATION_FIRST_LETTER}\\1"
         GZ_DESIGNATION_CAP "${GZ_DESIGNATION}")

  set(PROJECT_EXPORT_NAME ${PROJECT_NAME_LOWER})
  set(PROJECT_LIBRARY_TARGET_NAME ${PROJECT_NAME_LOWER})

  if(gz_configure_project_REPLACE_INCLUDE_PATH)
    set(PROJECT_INCLUDE_DIR ${gz_configure_project_REPLACE_INCLUDE_PATH})
  else()
    set(PROJECT_INCLUDE_DIR gz/${GZ_DESIGNATION})
  endif()

  # version <major>.<minor>
  set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

  # version <major>.<minor>.<patch>
  set(PROJECT_VERSION_FULL
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

  # The full version of the project, but without any prerelease suffix
  set(PROJECT_VERSION_FULL_NO_SUFFIX ${PROJECT_VERSION_FULL})

  if(PROJECT_VERSION_SUFFIX)
    # Append the prerelease suffix to PROJECT_VERSION_FULL if this has a suffix
    # version <major>.<minor>.<patch>~<suffix>
    set(PROJECT_VERSION_FULL ${PROJECT_VERSION_FULL}~${PROJECT_VERSION_SUFFIX})
  endif()

  set(PKG_NAME ${PROJECT_NAME_LOWER})

  message(STATUS "${PROJECT_NAME} version ${PROJECT_VERSION_FULL}")

  #============================================================================
  # Handle extra cmake configurations
  set(PACKAGE_CONFIG_EXTRA_FILES "")
  set(extras)

  if (DEFINED gz_configure_project_CONFIG_EXTRAS)
    list(APPEND extras ${gz_configure_project_CONFIG_EXTRAS})
  endif()

  #============================================================================
  # Identify the operating system
  gz_check_os()

  #============================================================================
  # Create package information
  _gz_setup_packages()

  #============================================================================
  # Configure and install cmake extras files
  # Do this after _gz_setup_packages() to ensure GNUInstallDirs has been called
  set(PROJECT_CMAKE_EXTRAS_RELATIVE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
  set(PROJECT_CMAKE_EXTRAS_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/${PROJECT_NAME})
  file(RELATIVE_PATH
    PROJECT_CMAKE_EXTRAS_PATH_TO_PREFIX
    "${PROJECT_CMAKE_EXTRAS_INSTALL_DIR}"
    "${CMAKE_INSTALL_PREFIX}"
  )

  foreach(extra ${extras})
    _gz_assert_file_exists("${extra}"
      "gz_configure_project() called with extra file '${extra}' which does not exist")
    _gz_stamp("${extra}")

    # expand template
    _gz_string_ends_with("${extra}" ".cmake.in" is_template)
    if(is_template)
      get_filename_component(extra_filename "${extra}" NAME)
      # cut off .in extension
      string(LENGTH "${extra_filename}" length)
      math(EXPR offset "${length} - 3")
      string(SUBSTRING "${extra_filename}" 0 ${offset} extra_filename)
      configure_file(
        "${extra}"
        ${CMAKE_CURRENT_BINARY_DIR}/gz-cmake/${extra_filename}
        @ONLY
      )
      set(extra
        "${CMAKE_CURRENT_BINARY_DIR}/gz-cmake/${extra_filename}")
    endif()

    # install cmake file and register for CMake config file
    _gz_string_ends_with("${extra}" ".cmake" is_cmake)
    if(is_cmake)
      install(FILES
        ${extra}
        DESTINATION ${PROJECT_CMAKE_EXTRAS_RELATIVE_INSTALL_DIR}
      )
      get_filename_component(extra_filename "${extra}" NAME)
      list(APPEND PACKAGE_CONFIG_EXTRA_FILES "${extra_filename}")
    else()
      message(FATAL_ERROR "gz_configure_project() the CONFIG_EXTRAS file '${extra}' "
        "does neither end with '.cmake' nor with '.cmake.in'.")
    endif()
  endforeach()

  #============================================================================
  # Initialize build errors/warnings
  # NOTE: We no longer use CACHE for these variables because it was set to
  # "INTERNAL", making it unnecessary to cache them. As long as this macro is
  # called from the top-level scope, these variables will effectively be global,
  # even without putting them in the cache. If this macro is not being called
  # from the top-level scope, then it is being used incorrectly.
  set(build_errors "")
  set(build_warnings "")


  #============================================================================
  # Initialize the list of <PROJECT_NAME>-config.cmake dependencies
  set(PROJECT_CMAKE_DEPENDENCIES)

  # Initialize the list of <PROJECT_NAME>.pc Requires
  set(PROJECT_PKGCONFIG_REQUIRES)

  # Initialize the list of <PROJECT_NAME>.pc Requires.private
  set(PROJECT_PKGCONFIG_REQUIRES_PRIVATE)

  # Initialize the list of <PROJECT_NAME>.pc Libs
  set(PROJECT_PKGCONFIG_LIBS)

  # Initialize the list of <PROJECT_NAME>.pc Libs.private
  set(PROJECT_PKGCONFIG_LIBS_PRIVATE)


  #============================================================================
  # We turn off extensions because (1) we do not ever want to use non-standard
  # compiler extensions, and (2) this variable is on by default, causing cmake
  # to choose the flag -std=gnu++14 instead of -std=c++14 when the C++14
  # features are requested. Explicitly turning this flag off will force cmake to
  # choose -std=c++14.
  set(CMAKE_CXX_EXTENSIONS off)

  #============================================================================
  # Put all runtime objects (executables and DLLs) into a single directory.
  # This helps executables (e.g. tests) to run from the build directory on
  # Windows. The DLLs that we build for this library needs to be available to
  # the executables that depend on them.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  # Put all libraries (.so, static .lib) into a single directory. This is just
  # for convenience.
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

  # Put all archive libraries (export .lib) into the lib directory. This is
  # just for convenience.
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

endmacro()

#################################################
# Check the OS type.
macro(gz_check_os)

  # CMake does not distinguish Linux from other Unices.
  string(REGEX MATCH "Linux" PLAYER_OS_LINUX ${CMAKE_SYSTEM_NAME})
  # Nor *BSD
  string(REGEX MATCH "BSD" PLAYER_OS_BSD ${CMAKE_SYSTEM_NAME})
  # Or Solaris. I'm seeing a trend, here
  string(REGEX MATCH "SunOS" PLAYER_OS_SOLARIS ${CMAKE_SYSTEM_NAME})

  # Windows is easy (for once)
  if(WIN32)
    set(PLAYER_OS_WIN TRUE BOOL INTERNAL)
  endif()

  # Check if it's an Apple OS
  if(APPLE)
    # Check if it's OS X or another MacOS (that's got to be pretty unlikely)
    string(REGEX MATCH "Darwin" PLAYER_OS_OSX ${CMAKE_SYSTEM_NAME})
    if(NOT PLAYER_OS_OSX)
      set(PLAYER_OS_MACOS TRUE BOOL INTERNAL)
    endif()
  endif()

  # QNX
  if(QNXNTO)
    set(PLAYER_OS_QNX TRUE BOOL INTERNAL)
  endif()

  if(PLAYER_OS_LINUX)
    message(STATUS "Operating system is Linux")
  elseif(PLAYER_OS_BSD)
    message(STATUS "Operating system is BSD")
  elseif(PLAYER_OS_WIN)
    message(STATUS "Operating system is Windows")
  elseif(PLAYER_OS_OSX)
    message(STATUS "Operating system is Apple MacOS X")
  elseif(PLAYER_OS_MACOS)
    message(STATUS "Operating system is Apple MacOS (not OS X)")
  elseif(PLAYER_OS_QNX)
    message(STATUS "Operating system is QNX")
  elseif(PLAYER_OS_SOLARIS)
    message(STATUS "Operating system is Solaris")
  else(PLAYER_OS_LINUX)
    message(STATUS "Operating system is generic Unix")
  endif()

  #################################################
  # Check for non-case-sensitive filesystems
  execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/case_sensitive_filesystem
                  RESULT_VARIABLE FILESYSTEM_CASE_SENSITIVE_RETURN)
  if (${FILESYSTEM_CASE_SENSITIVE_RETURN} EQUAL 0)
    set(FILESYSTEM_CASE_SENSITIVE TRUE)
  else()
    set(FILESYSTEM_CASE_SENSITIVE FALSE)
  endif()

endmacro()
