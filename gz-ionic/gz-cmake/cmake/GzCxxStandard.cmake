# Copyright (C) 2023 Open Source Robotics Foundation
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

#################################################
# Macro to setup supported compiler warnings
# Based on work of Florent Lamiraux, Thomas Moulard, JRL, CNRS/AIST.
# Internal to gz-cmake
macro(_gz_filter_valid_compiler_options var)

  include(CheckCXXCompilerFlag)
  # Store the current setting for CMAKE_REQUIRED_QUIET
  set(original_cmake_required_quiet ${CMAKE_REQUIRED_QUIET})

  # Make these tests quiet so they don't pollute the cmake output
  set(CMAKE_REQUIRED_QUIET true)

  foreach(flag ${ARGN})
    CHECK_CXX_COMPILER_FLAG(${flag} result${flag})
    if(result${flag})
      set(${var} "${${var}} ${flag}")
    endif()
  endforeach()

  # Restore the old setting for CMAKE_REQUIRED_QUIET
  set(CMAKE_REQUIRED_QUIET ${original_cmake_required_quiet})
endmacro()

#################################################
# _gz_check_known_cxx_standards(<11|14|17>)
#
# Creates a fatal error if the variable passed in does not represent a supported
# version of the C++ standard.
#
# NOTE: This function is meant for internal gz-cmake use
#
function(_gz_check_known_cxx_standards standard)

  list(FIND GZ_KNOWN_CXX_STANDARDS ${standard} known)
  if(${known} EQUAL -1)
    message(FATAL_ERROR
      "You have specified an unsupported standard: ${standard}. "
      "Accepted values are: ${GZ_KNOWN_CXX_STANDARDS}.")
  endif()

endfunction()

#################################################
# _gz_handle_cxx_standard(<function_prefix>
#                          <target_name>
#                          <pkgconfig_cflags_variable>)
#
# Handles the C++ standard argument for gz_create_core_library(~) and
# gz_add_component(~).
#
# NOTE: This is only meant for internal gz-cmake use.
#
macro(_gz_handle_cxx_standard prefix target pkgconfig_cflags)

  if(${prefix}_CXX_STANDARD)
    _gz_check_known_cxx_standards(${${prefix}_CXX_STANDARD})
  endif()

  if(${prefix}_PRIVATE_CXX_STANDARD)
    _gz_check_known_cxx_standards(${${prefix}_PRIVATE_CXX_STANDARD})
  endif()

  if(${prefix}_INTERFACE_CXX_STANDARD)
    _gz_check_known_cxx_standards(${${prefix}_INTERFACE_CXX_STANDARD})
  endif()

  if(${prefix}_CXX_STANDARD
      AND (${prefix}_PRIVATE_CXX_STANDARD
           OR ${prefix}_INTERFACE_CXX_STANDARD))
    message(FATAL_ERROR
      "If CXX_STANDARD has been specified, then you are not allowed to specify "
      "PRIVATE_CXX_STANDARD or INTERFACE_CXX_STANDARD. Please choose to either "
      "specify CXX_STANDARD alone, or else specify some combination of "
      "PRIVATE_CXX_STANDARD and INTERFACE_CXX_STANDARD")
  endif()

  if(${prefix}_CXX_STANDARD)
    set(${prefix}_INTERFACE_CXX_STANDARD ${${prefix}_CXX_STANDARD})
    set(${prefix}_PRIVATE_CXX_STANDARD ${${prefix}_CXX_STANDARD})
  endif()

  if(${prefix}_INTERFACE_CXX_STANDARD)
    target_compile_features(${target} INTERFACE ${GZ_CXX_${${prefix}_INTERFACE_CXX_STANDARD}_FEATURES})
    gz_string_append(${pkgconfig_cflags} "-std=c++${${prefix}_INTERFACE_CXX_STANDARD}")
  endif()

  if(${prefix}_PRIVATE_CXX_STANDARD)
    target_compile_features(${target} PRIVATE ${GZ_CXX_${${prefix}_PRIVATE_CXX_STANDARD}_FEATURES})
  endif()

endmacro()
