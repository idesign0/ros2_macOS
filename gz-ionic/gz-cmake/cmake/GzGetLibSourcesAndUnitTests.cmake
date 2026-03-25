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
# gz_get_libsources_and_unittests(<lib_srcs> <tests>)
#
# Grab all the files ending in "*.cc" from either the "src/" subdirectory or the
# current subdirectory if "src/" does not exist. They will be collated into
# library source files <lib_sources_var> and unittest source files <tests_var>.
#
# These output variables can be consumed directly by gz_create_core_library(~),
# gz_add_component(~), gz_build_tests(~), and gz_build_executables(~).
function(gz_get_libsources_and_unittests lib_sources_var tests_var)

  # Glob all the source files
  if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/src)

    # Prefer files in the src/ subdirectory
    file(GLOB source_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "src/*.cc")
    file(GLOB test_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "src/*_TEST.cc")

  else()

    # If src/ doesn't exist, then use the current directory
    file(GLOB source_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.cc")
    file(GLOB test_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*_TEST.cc")

  endif()

  # Sort the files alphabetically
  if(source_files)
    list(SORT source_files)
  endif()

  if(test_files)
    list(SORT test_files)
  endif()

  # Initialize the test list
  set(tests)

  # Remove the unit tests from the list of source files
  foreach(test_file ${test_files})

    # Remove from the source_files list.
    list(REMOVE_ITEM source_files ${test_file})

    # Append to the list of tests.
    list(APPEND tests ${test_file})

  endforeach()

  # Return the lists that have been created.
  set(${lib_sources_var} ${source_files} PARENT_SCOPE)
  set(${tests_var} ${tests} PARENT_SCOPE)

endfunction()
