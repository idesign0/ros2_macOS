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
# Find EIGEN3
#
# Usage of this module is as follows:
#
#     find_package(EIGEN3 [VERSION <major>[.<minor>[.<patch>]]])
#
# Variables defined by this module:
#
# Eigen3::Eigen            Imported target for eigen3
# EIGEN3_FOUND             System has eigen3 library and headers

find_package(Eigen3 ${EIGEN3_FIND_VERSION} CONFIG)

if(EIGEN3_FOUND)

  # Attempt to create an imported target in case we're using an old version
  # of Eigen. (This function skip creating the imported target if it already
  # exists).
  gz_import_target(EIGEN3 INTERFACE
    TARGET_NAME Eigen3::Eigen)

  if(EIGEN3_FIND_VERSION)
    gz_pkg_config_entry(EIGEN3 "eigen3 >= ${EIGEN3_FIND_VERSION}")
  else()
    gz_pkg_config_entry(EIGEN3 "eigen3")
  endif()

  return()

endif()

if(EIGEN3_FIND_VERSION)
  gz_pkg_check_modules_quiet(EIGEN3 "eigen3 >= ${EIGEN3_FIND_VERSION}"
    INTERFACE
    TARGET_NAME Eigen3::Eigen)
else()
  gz_pkg_check_modules_quiet(EIGEN3 "eigen3"
    INTERFACE
    TARGET_NAME Eigen3::Eigen)
endif()

if(MSVC)

  set(EIGEN3_FOUND TRUE)

  find_path(EIGEN3_INCLUDE_DIRS signature_of_eigen3_matrix_library)
  mark_as_advanced(EIGEN3_INCLUDE_DIRS)
  if(NOT EIGEN3_INCLUDE_DIRS)
    set(EIGEN3_FOUND FALSE)
    if(NOT EIGEN3_FIND_QUIETLY)
      message(STATUS "Looking for eigen headers (signature_of_eigen3_matrix_library) - not found")
    endif()
  endif()

  gz_import_target(EIGEN3 INTERFACE
    TARGET_NAME Eigen3::Eigen)

endif()
