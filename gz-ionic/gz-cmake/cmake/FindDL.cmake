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
# Find libdl

if(MSVC)
  # The dlfcn-win32 library installs a config module, so we should leverage that
  find_package(dlfcn-win32)

  # Note: Ideally, we would create an alias target for dlfcn-win32::dl, but
  # cmake does not (currently) support aliasing imported targets
  # (see: https://gitlab.kitware.com/cmake/cmake/issues/15569)
  #
  # For now, we will use a variable named DL_TARGET for linking, even though
  # it is old-fashioned. If a day comes when cmake supported aliasing
  # imported targets, then we should migrate to something like
  #
  # add_library(DL::DL ALIAS dlfcn-win32::dl)
  #
  # And then link to DL::DL.
  set(DL_TARGET dlfcn-win32::dl)

  if(dlfcn-win32_FOUND)
    set(DL_FOUND true)
  else()
    set(DL_FOUND false)
  endif()

else()

  # NOTE: libdl is a system library on UNIX, so it does not come with pkgconfig metadata

  # If we cannot find the header or the library, we will switch this to false
  set(DL_FOUND true)

  # Search for the header
  find_path(DL_INCLUDE_DIRS dlfcn.h)
  if(DL_INCLUDE_DIRS)

    if(NOT DL_FIND_QUIETLY)
      message(STATUS "Looking for dlfcn.h - found")
    endif()

  else(DL_INCLUDE_DIRS)

    if(NOT DL_FIND_QUIETLY)
      message(STATUS "Looking for dlfcn.h - not found")
    endif()

    set(DL_FOUND false)

  endif()
  mark_as_advanced(DL_INCLUDE_DIRS)

  # Search for the library
  find_library(DL_LIBRARIES dl)
  if(DL_LIBRARIES)

    if(NOT DL_FIND_QUIETLY)
      message(STATUS "Looking for libdl - found")
    endif()

  else(DL_LIBRARIES)

    if(NOT DL_FIND_QUIETLY)
      message(STATUS "Looking for libdl - not found")
    endif()

    set(DL_FOUND false)

  endif()
  mark_as_advanced(DL_LIBRARIES)

  if(DL_FOUND)
    include(GzImportTarget)
    gz_import_target(DL)
    set(DL_TARGET DL::DL)
  endif()

endif()

# We need to manually specify the pkgconfig entry (and type of entry) for dl,
# because gz_pkg_check_modules does not work for it.
include(GzPkgConfig)
gz_pkg_config_library_entry(DL dl)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  DL
  REQUIRED_VARS DL_FOUND)
