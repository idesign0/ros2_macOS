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
# Find yaml.
#
# Usage of this module as follows:
#
#     find_package(YAML)
#
# Variables defined by this module:
#
#  YAML_TARGET             Imported target for libyaml
#
#  YAML_FOUND              System has YAML libs/headers
#  YAML_INCLUDE_DIRS       The location of YAML headers
#  YAML_LIBRARIES          The YAML libraries

# initialize this variable to the default target name
set(YAML_TARGET YAML::YAML)

find_package(yaml ${YAML_FIND_VERSION} CONFIG QUIET)
if (yaml_FOUND)

  # yaml's cmake script imports its own target, so we'll
  # overwrite the default with the name of theirs. In the
  # future, we should be able to use a target alias instead.
  set(YAML_TARGET yaml)

  set(YAML_FOUND True)
  set(YAML_INCLUDE_DIRS ${yaml_INCLUDE_DIRS})
  set(YAML_LIBRARIES ${yaml_LIBRARIES})

  return()

endif()

if(YAML_FIND_VERSION AND NOT YAML_FIND_VERSION VERSION_EQUAL "0.1")
  message(WARNING "FindYAML only knows how to find version 0.1 "
                  "but you requested version ${YAML_FIND_VERSION}.")
else()
  include(GzPkgConfig)
  gz_pkg_check_modules_quiet(YAML yaml-0.1)

  # If that failed, then fall back to manual detection.
  if(NOT YAML_FOUND)

    if(NOT YAML_FIND_QUIETLY)
      message(STATUS "Attempting manual search for yaml")
    endif()

    find_path(YAML_INCLUDE_DIRS yaml.h ${YAML_INCLUDE_DIRS} ENV CPATH)
    find_library(YAML_LIBRARIES NAMES yaml)
    set(YAML_FOUND true)

    if(NOT YAML_INCLUDE_DIRS)
      if(NOT YAML_FIND_QUIETLY)
        message(STATUS "Looking for yaml headers - not found")
      endif()
      set(YAML_FOUND false)
    endif()

    if(NOT YAML_LIBRARIES)
      if(NOT YAML_FIND_QUIETLY)
        message (STATUS "Looking for yaml library - not found")
      endif()
      set(YAML_FOUND false)
    endif()

    if(YAML_FOUND)
      include(GzImportTarget)
      gz_import_target(YAML)
      message(STATUS "Assuming libyaml is static, defining YAML_DECLARE_STATIC")
      set_target_properties(YAML::YAML PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "YAML_DECLARE_STATIC"
      )
    endif()
  endif()
  
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    YAML
    REQUIRED_VARS YAML_FOUND)
endif()
