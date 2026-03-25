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
# Find Protobuf

# This is a Gazebo wrapper for finding Protobuf. The purpose of this find
# module is to search for a config-file for Protobuf before resorting to using
# the native CMake find-module for Protobuf. This ensures that if a specially
# configured version of Protobuf is installed, then its exported targets will be
# correctly imported. This is especially important on Windows in order to
# support shared library versions of Protobuf.

include(GzPkgConfig)
gz_pkg_config_entry(GzProtobuf "protobuf >= ${GzProtobuf_FIND_VERSION}")

find_package(Protobuf ${GzProtobuf_FIND_VERSION} QUIET CONFIG)

if(NOT ${Protobuf_FOUND})
  # If a config-file was not found, then fall back on the system-installed
  # find-module that comes with CMake.
  find_package(Protobuf ${GzProtobuf_FIND_VERSION})
endif()

set(GzProtobuf_missing_components "")
foreach(component ${GzProtobuf_FIND_COMPONENTS})

  # If specific components are requested, check that each one is accounted for.
  # If any component is missing, then we should not consider this package to be
  # found.

  # If a requested component is not required, then we can just skip this
  # iteration. We don't do anything special for optional components.
  if(NOT GzProtobuf_FIND_REQUIRED_${component})
    continue()
  endif()

  if((${component} STREQUAL "libprotobuf") OR (${component} STREQUAL "all"))
    if((NOT PROTOBUF_LIBRARY) AND (NOT TARGET protobuf::libprotobuf))
      set(Protobuf_FOUND false)
      gz_string_append(GzProtobuf_missing_components "libprotobuf" DELIM " ")
    endif()
  endif()

  if((${component} STREQUAL "libprotoc") OR (${component} STREQUAL "all"))
    if((NOT PROTOBUF_PROTOC_LIBRARY) AND (NOT TARGET protobuf::libprotoc))
      set(Protobuf_FOUND false)
      gz_string_append(GzProtobuf_missing_components "libprotoc" DELIM " ")
    endif()
  endif()

  if((${component} STREQUAL "protoc") OR (${component} STREQUAL "all"))
    if((NOT PROTOBUF_PROTOC_EXECUTABLE) AND (NOT TARGET protobuf::protoc))
      set(Protobuf_FOUND false)
      gz_string_append(GzProtobuf_missing_components "protoc" DELIM " ")
    endif()
  endif()

endforeach()

if(GzProtobuf_missing_components AND NOT GzProtobuf_FIND_QUIETLY)
  message(STATUS "Missing required protobuf components: ${GzProtobuf_missing_components}")
endif()

if(${Protobuf_FOUND})
  # If we have found Protobuf, then set the GzProtobuf_FOUND flag to true so
  # that gz_find_package(~) knows that we were successful.
  set(GzProtobuf_FOUND true)

  # Older versions of protobuf don't create imported targets, so we will create
  # them here if they have not been provided.
  include(GzImportTarget)

  if(NOT TARGET protobuf::libprotobuf)
    gz_import_target(protobuf
      TARGET_NAME protobuf::libprotobuf
      LIB_VAR PROTOBUF_LIBRARY
      INCLUDE_VAR PROTOBUF_INCLUDE_DIR)
  endif()

  if(NOT TARGET protobuf::libprotoc)
    gz_import_target(protobuf
      TARGET_NAME protobuf::libprotoc
      LIB_VAR PROTOBUF_PROTOC_LIBRARY
      INCLUDE_VAR PROTOBUF_INCLUDE_DIR)
  endif()

  if(NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc IMPORTED)
    set_target_properties(protobuf::protoc PROPERTIES
      IMPORTED_LOCATION ${PROTOBUF_PROTOC_EXECUTABLE})
  endif()
  
  # See: https://github.com/osrf/buildfarmer/issues/377
  if(MSVC)
    target_compile_options(protobuf::protoc INTERFACE /wd4251)
    target_compile_options(protobuf::libprotoc INTERFACE /wd4251)
    target_compile_options(protobuf::libprotobuf INTERFACE /wd4251)
  endif()

endif()
