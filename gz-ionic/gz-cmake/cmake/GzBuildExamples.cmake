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
# gz_build_examples(
#     SOURCE_DIR <source_dir>
#     BINARY_DIR <binary_dir>
#
# Build examples for a Gazebo project.
# Requires a CMakeLists.txt file to be in SOURCE_DIR that acts
# as a top level project.
#
# This generates two test targets
# * EXAMPLES_Configure_TEST - Equivalent of calling "cmake .." on
#                             the examples directory
#
# * EXAMPLES_Build_TEST - Equivalent of calling "make" on the
#                         examples directory
#
# These tests are run during "make test" or can be run specifically
# via "ctest -R EXAMPLES_ -V"
#
# Arguments are as follows:
#
# SOURCE_DIR: Required. Path to the examples folder.
#             For example ${CMAKE_CURRENT_SOURCE_DIR}/examples
#
# BINARY_DIR: Required. Path to the output binary folder
#             For example ${CMAKE_CURRENT_BINARY_DIR}/examples
#
macro(gz_build_examples)
  option(BUILD_EXAMPLES "Build examples (requires a system installation of the software first)" OFF)

  if(NOT ${BUILD_EXAMPLES})
    message(STATUS "Building examples disabled")
  else()
    #------------------------------------
    # Define the expected arguments
    set(options)
    set(oneValueArgs SOURCE_DIR BINARY_DIR)

    #------------------------------------
    # Parse the arguments
    _gz_cmake_parse_arguments(gz_build_examples "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(gz_build_examples_CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})

    if (gz_build_examples_CMAKE_PREFIX_PATH)
      # Replace colons from environment variable with semicolon cmake list delimiter
      # Only perform if string has contents, otherwise cmake will complain about REPLACE command
      string(REPLACE ":" ";" gz_build_examples_CMAKE_PREFIX_PATH ${gz_build_examples_CMAKE_PREFIX_PATH})
    endif()

    if (CMAKE_INSTALL_PREFIX)
      list(APPEND gz_build_examples_CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
    endif()

    add_test(
      NAME EXAMPLES_Configure_TEST
      COMMAND ${CMAKE_COMMAND} -G${CMAKE_GENERATOR}
                               --no-warn-unused-cli
                               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                               -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                               -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                               -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                               -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                               "-DCMAKE_PREFIX_PATH=${gz_build_examples_CMAKE_PREFIX_PATH}"
                               -S ${gz_build_examples_SOURCE_DIR}
                               -B ${gz_build_examples_BINARY_DIR}
    )

    add_test(
      NAME EXAMPLES_Build_TEST
      COMMAND ${CMAKE_COMMAND} --build ${gz_build_examples_BINARY_DIR}
                               --config $<CONFIG>
    )
    set_tests_properties(EXAMPLES_Build_TEST
      PROPERTIES DEPENDS "EXAMPLES_Configure_TEST")
  endif()
endmacro()
