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
# Find uuid
if (UNIX)
  if(NOT APPLE)
    include(GzPkgConfig)
    gz_pkg_check_modules_quiet(UUID uuid)

    if(NOT UUID_FOUND)
      include(GzManualSearch)
      gz_manual_search(UUID
                       HEADER_NAMES uuid.h
                       LIBRARY_NAMES uuid libuuid
                       PATH_SUFFIXES uuid)
    endif()

    # The pkg-config or the manual search will place
    # <uuid_install_prefix>/include/uuid in INTERFACE_INCLUDE_DIRECTORIES,
    # but some projects exepect to use <uuid_install_prefix>/include, so
    # we add it as well.
    # See https://github.com/gazebosim/gz-cmake/issues/103
    if(TARGET UUID::UUID)
      get_property(uuid_include_dirs
        TARGET UUID::UUID
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

      set(uuid_include_dirs_extended ${uuid_include_dirs})

      foreach(include_dir IN LISTS uuid_include_dirs)
        if(include_dir MATCHES "uuid$")
          get_filename_component(include_dir_parent ${include_dir} DIRECTORY)
          list(APPEND uuid_include_dirs_extended ${include_dir_parent})
        endif()
      endforeach()

      list(REMOVE_DUPLICATES uuid_include_dirs_extended)

      set_property(
        TARGET UUID::UUID
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        ${uuid_include_dirs_extended})
    endif()
  else()
    # On Apple platforms the UUID library is provided by the OS SDK
    # See https://github.com/gazebosim/gz-cmake/issues/127
    set(UUID_FOUND TRUE)
    if(NOT TARGET UUID::UUID)
      add_library(UUID::UUID INTERFACE IMPORTED)
    endif()
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    UUID
    REQUIRED_VARS UUID_FOUND)
endif()
