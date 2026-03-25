#
# Copyright (C) 2021 Open Source Robotics Foundation
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

# Prefer pkg-config over cmake if possible since version checking is not working
# on urdfdom series from 1.x to 3.0.0 (at least)
include(GzPkgConfig)
if(PKG_CONFIG_FOUND)
  if (GzURDFDOM_FIND_VERSION)
    set(signature "urdfdom >= ${GzURDFDOM_FIND_VERSION}")
  else()
    set(signature "urdfdom")
  endif()
  gz_pkg_check_modules(GzURDFDOM "${signature}")
else()
  message(VERBOSE "Unable to find pkg-config in the system, fallback to use CMake")
endif()

if(NOT GzURDFDOM_FOUND)
  if(GzURDFDOM_FIND_VERSION)
    set(find_version VERSION ${GzURDFDOM_FIND_VERSION})
  else()
    set(find_version "")
  endif()

  # NOTE: urdfdom cmake does not support version checking
  gz_find_package(urdfdom ${find_version} QUIET)
  if (urdfdom_FOUND)
    add_library(GzURDFDOM::GzURDFDOM INTERFACE IMPORTED)
    target_link_libraries(GzURDFDOM::GzURDFDOM
      INTERFACE
        urdfdom::urdfdom_model
        urdfdom::urdfdom_world
        urdfdom::urdfdom_sensor
        urdfdom::urdfdom_model_state
    )
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(GzURDFDOM DEFAULT_MSG)
  endif()
endif()
