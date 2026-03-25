#===============================================================================
# Copyright (C) 2018 Open Source Robotics Foundation
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
# Find SQLite3
#
# Usage of this module is as follows:
#
#     find_package(SQLite3 [VERSION <major>[.<minor>[.<patch>]]])
#
# Variables defined by this module:
#
# SQLite3::SQLite3          Imported target for sqlite3
# SQLite::SQLite3           Imported target for sqlite3, for
#                           compatibility with upstream CMake
# SQLite3_FOUND             System has sqlite3 library and headers

if(SQLite3_FIND_VERSION)
  gz_pkg_check_modules_quiet(SQLite3 "sqlite3 >= ${SQLite3_FIND_VERSION}")
else()
  gz_pkg_check_modules_quiet(SQLite3 "sqlite")
endif()

if(MSVC)

  set(SQLite3_FOUND TRUE)

  find_library(SQLite3_LIBRARIES sqlite3)
  if(NOT SQLite3_LIBRARIES)
    set(SQLite3_FOUND FALSE)
    if(NOT SQLite3_FIND_QUIETLY)
      message(STATUS "Looking for sqlite3 library - not found")
    endif()
  endif()

  find_path(SQLite3_INCLUDE_DIRS sqlite3.h)
  if(NOT SQLite3_INCLUDE_DIRS)
    set(SQLite3_FOUND FALSE)
    if(NOT SQLite3_FIND_QUIETLY)
      message(STATUS "Looking for sqlite header (sqlite3.h) - not found")
    endif()
  endif()

  gz_import_target(SQLite3)

endif()

# Workaround for https://github.com/gazebosim/gz-cmake/issues/312
if(TARGET SQLite3::SQLite3 AND NOT TARGET SQLite::SQLite3)
  add_library(SQLite::SQLite3 INTERFACE IMPORTED)
  # Equivalent to target_link_libraries INTERFACE, but compatible with CMake 3.10
  set_target_properties(SQLite::SQLite3 PROPERTIES INTERFACE_LINK_LIBRARIES SQLite3::SQLite3)
endif()
