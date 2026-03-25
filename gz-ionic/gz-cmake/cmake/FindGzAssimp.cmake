#===============================================================================
# Copyright (C) 2022 Open Source Robotics Foundation
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
# Find Assimp
# Provides a GzAssimp alias to avoid conflicts with other packages
# that might provide a FindAssimp function, such as dartsim

# Set CMP0012 policy to NEW to avoid OLD policy warning in assimp 5.0.1
set(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
find_package(assimp CONFIG QUIET)

set(GzAssimp_FOUND ${assimp_FOUND})
if (MSVC)
  set(GzAssimp_LIBRARIES assimp::assimp)
else()
  set(GzAssimp_LIBRARIES ${ASSIMP_LIBRARIES})
endif()
set(GzAssimp_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIRS})
set(GzAssimp_VERSION ${assimp_VERSION})

include(GzPkgConfig)
gz_pkg_config_entry(GzAssimp "assimp")

# Clear cached variables so downstream packages will trigger a find
unset(assimp_FOUND CACHE)
unset(ASSIMP_INCLUDE_DIRS CACHE)
unset(ASSIMP_LIBRARIES CACHE)
unset(ASSIMP_VERSION CACHE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GzAssimp
  REQUIRED_VARS GzAssimp_FOUND
  VERSION_VAR GzAssimp_VERSION)
