#===============================================================================
# Copyright (C) 2020 Open Source Robotics Foundation
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
# gz_manual_search(<package> [INTERFACE]
#     [HEADER_NAMES <header_names>]
#     [LIBRARY_NAMES <library_names>]
#     [TARGET_NAME <target_name>]
#     [PATH_SUFFIXES <path_suffixes>]])
#
# This macro will find a library based on the name of one of its headers,
# and the library name.
# It is used inside Find***.cmake scripts, typicall as fallback for a
# gz_pkg_check_modules_quiet call.
# It will create an imported target for the  library
#
# INTERFACE: Optional. Use INTERFACE when the target does not actually provide
#            a library that needs to be linked against (e.g. it is a header-only
#            library, or the target is just used to specify compiler flags).
#
# HEADER_NAMES: Optional. Explicitly specify the header names to search with find_path.
#              Default is <package>.h.
#
# LIBRARY_NAMES: Optional. Explicitly specify the names of the library to search with find_library.
#              Default is <package>.
#
# TARGET_NAME: Optional. Explicitly specify the desired imported target name.
#              Default is <package>::<package>.
#
# PATH_SUFFIXES: Optional. Parameter forwarded to the find_path and find_library calls.
#
macro(gz_manual_search package)
  # Define the expected arguments
  set(options INTERFACE)
  set(oneValueArgs "TARGET_NAME")
  set(multiValueArgs "HEADER_NAMES" "LIBRARY_NAMES" "PATH_SUFFIXES")
  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_manual_search "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(gz_manual_search_INTERFACE)
    set(_gz_manual_search_interface_option INTERFACE)
  else()
    set(_gz_manual_search_interface_option) # Intentionally blank
  endif()

  if(NOT gz_manual_search_HEADER_NAMES)
    set(gz_manual_search_HEADER_NAMES "${package}.h")
  endif()

  if(NOT gz_manual_search_LIBRARY_NAMES)
    set(gz_manual_search_LIBRARY_NAMES "${package}")
  endif()

  if(NOT gz_manual_search_TARGET_NAME)
    set(gz_manual_search_TARGET_NAME "${package}::${package}")
  endif()

  find_path(${package}_INCLUDE_DIRS
            NAMES ${gz_manual_search_HEADER_NAMES}
            PATH_SUFFIXES ${gz_manual_search_PATH_SUFFIXES})
  find_library(${package}_LIBRARIES
               NAMES ${gz_manual_search_LIBRARY_NAMES}
               PATH_SUFFIXES ${gz_manual_search_PATH_SUFFIXES})

  mark_as_advanced(${package}_INCLUDE_DIRS)
  mark_as_advanced(${package}_LIBRARIES)

  set(${package}_FOUND true)

  if(NOT ${package}_INCLUDE_DIRS)
    set(${package}_FOUND false)
  endif()

  if(NOT ${package}_LIBRARIES)
    set(${package}_FOUND false)
  endif()

  if(${package}_FOUND)
    include(GzImportTarget)
    gz_import_target(${package} ${_gz_pkg_check_modules_interface_option}
      TARGET_NAME ${gz_pkg_check_modules_TARGET_NAME})
  endif()

endmacro()
