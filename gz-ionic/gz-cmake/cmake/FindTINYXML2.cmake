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
#
########################################
# Find tinyxml2. Only debian distributions package tinyxml with a pkg-config.

include(GzPkgConfig)

# Use pkg_check_modules to start
gz_pkg_check_modules_quiet(TINYXML2 tinyxml2)

# If that failed, then fall back to manual detection (necessary for MacOS)
if(NOT TINYXML2_FOUND)

  if(NOT TINYXML2_FIND_QUIETLY)
    message(STATUS "Attempting manual search for tinyxml2")
  endif()

  find_path(TINYXML2_INCLUDE_DIRS tinyxml2.h ${TINYXML2_INCLUDE_DIRS} ENV CPATH)
  find_library(TINYXML2_LIBRARIES NAMES tinyxml2)
  set(TINYXML2_FOUND true)

  if(NOT TINYXML2_INCLUDE_DIRS)

    if(NOT TINYXML2_FIND_QUIETLY)
      message(STATUS "Looking for tinyxml2 headers - not found")
    endif()

    set(TINYXML2_FOUND false)

  endif()

  if(NOT TINYXML2_LIBRARIES)

    if(NOT TINYXML2_FIND_QUIETLY)
      message (STATUS "Looking for tinyxml2 library - not found")
    endif()

    set(TINYXML2_FOUND false)

  endif()

  if(TINYXML2_FOUND)
    include(GzImportTarget)
    gz_import_target(TINYXML2)
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    TINYXML2
    REQUIRED_VARS TINYXML2_FOUND)

endif()

# On case-insensitive filesystem, it is possible that FindTINYXML2.cmake is used if the caller
# invoked find_package(TinyXML2), that is the signature used by tinyxml2_vendor, see
# https://github.com/ros2/tinyxml2_vendor/blob/0.10.0/cmake/Modules/FindTinyXML2.cmake
# If that is the case (and we detect it by checking the value of CMAKE_FIND_PACKAGE_NAME)
# we also define a tinyxml2::tinyxml2 target for tinyxml2_vendor compatibility
if(TARGET TINYXML2::TINYXML2 AND CMAKE_FIND_PACKAGE_NAME STREQUAL "TinyXML2" AND NOT TARGET tinyxml2::tinyxml2)
  add_library(tinyxml2::tinyxml2 INTERFACE IMPORTED)
  set_property(TARGET tinyxml2::tinyxml2 PROPERTY INTERFACE_LINK_LIBRARIES TINYXML2::TINYXML2)
endif()

