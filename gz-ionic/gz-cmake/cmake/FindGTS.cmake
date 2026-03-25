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
# Find GNU Triangulation Surface Library

if (NOT WIN32)
  # Configuration using pkg-config modules
  include(GzPkgConfig)
  gz_pkg_check_modules(GTS gts)
else()
  # true by default, change to false when a failure appears
  set(GTS_FOUND true)

  # 1. look for GTS headers
  find_path(GTS_INCLUDE_DIRS gts.h
    hints
      ${CMAKE_FIND_ROOT_PATH}
    paths
      ${CMAKE_FIND_ROOT_PATH}
    doc "GTS header include dir"
    path_suffixes
      include
  )

  if (GTS_INCLUDE_DIRS)
    if(NOT GTS_FIND_QUIETLY)
      message(STATUS "Looking for gts.h gtsconfig.h - found")
    endif()
  else()
    if(NOT GTS_FIND_QUIETLY)
      message(STATUS "Looking for gts.h gtsconfig.h - not found")
    endif()

    set(GTS_FOUND false)
  endif()
  mark_as_advanced(GTS_INCLUDE_DIRS)

  # 2. look for GTS libraries
  find_library(GTS_LIBRARIES gts)
  mark_as_advanced(GTS_LIBRARIES)

  if (GTS_LIBRARIES)
    if(NOT GTS_FIND_QUIETLY)
      message(STATUS "Looking for gts library - found")
    endif()
  else()
    if(NOT GTS_FIND_QUIETLY)
      message(STATUS "Looking for gts library - not found")
    endif()

    set (GTS_FOUND false)
  endif()

  # 2.1 Need glib library
  find_library(GLIB_LIBRARY glib-2.0)
  if (NOT GLIB_LIBRARY)
    if(NOT GTS_FIND_QUIETLY)
      message(STATUS "Looking for glib library - not found")
    endif()
  else()
    if(NOT GTS_FIND_QUIETLY)
      message(STATUS "Looking for glib library - found")
    endif()
  endif()
  find_path(GLIB_INCLUDE_DIR
    NAMES glib.h
    PATH_SUFFIXES glib-2.0)
  if (GLIB_INCLUDE_DIR)
    list(APPEND GTS_INCLUDE_DIRS "${GLIB_INCLUDE_DIR}")
  endif()
  find_path(GLIBCONFIG_INCLUDE_DIR
    NAMES glibconfig.h
    PATH_SUFFIXES lib/glib-2.0/include)
  if (GLIBCONFIG_INCLUDE_DIR)
    list(APPEND GTS_INCLUDE_DIRS "${GLIBCONFIG_INCLUDE_DIR}")
  endif()
  list(APPEND GTS_LIBRARIES "${GLIB_LIBRARY}")

  if (GTS_FOUND)
    # We need to manually specify the pkgconfig entry (and type of entry),
    # because gz_pkg_check_modules does not work for it.
    include(GzPkgConfig)
    gz_pkg_config_library_entry(GTS gts)
    include(GzImportTarget)
    gz_import_target(GTS)
  endif()
endif()
