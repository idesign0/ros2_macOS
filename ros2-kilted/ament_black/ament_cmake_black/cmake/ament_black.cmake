# Copyright 2014-2015 Picknik Robotics
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

#
# Add a test to check the code for compliance with black.
#
# :param TESTNAME: the name of the test, default: "black" :type TESTNAME: string
# :param ARGN: the files or directories to check :type ARGN: list of strings
#
# @public
#
function(ament_black)
  cmake_parse_arguments(ARG "" "TESTNAME;CONFIG_FILE" "" ${ARGN})
  if(NOT ARG_TESTNAME)
    set(ARG_TESTNAME "black")
  endif()

  find_program(ament_black_BIN NAMES "ament_black")
  if(NOT ament_black_BIN)
    message(
      FATAL_ERROR "ament_black() variable 'ament_black_BIN' must not be empty")
  endif()

  set(result_file
      "${AMENT_TEST_RESULTS_DIR}/${PROJECT_NAME}/${ARG_TESTNAME}.xunit.xml")
  set(cmd "${ament_black_BIN}" "--xunit-file" "${result_file}")
  list(APPEND cmd ${ARG_UNPARSED_ARGUMENTS})
  if(ARG_CONFIG_FILE)
    list(APPEND cmd "--config" "${ARG_CONFIG_FILE}")
  elseif(DEFINED ament_cmake_black_CONFIG_FILE)
    list(APPEND cmd "--config" "${ament_cmake_black_CONFIG_FILE}")
  endif()
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/ament_black")
  ament_add_test(
    "${ARG_TESTNAME}"
    COMMAND
    ${cmd}
    OUTPUT_FILE
    "${CMAKE_BINARY_DIR}/ament_black/${ARG_TESTNAME}.txt"
    RESULT_FILE
    "${result_file}"
    WORKING_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}")
  set_tests_properties("${ARG_TESTNAME}" PROPERTIES LABELS "black;linter")
endfunction()
