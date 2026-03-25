# Copyright 2023 Nick Morales.
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
# Add a launch integration test.
#
# Test should launch from a launch file installed to the package's share space.
# Use the launch_catch_ros2 Python module to create a Python launch file for
# this purpose. See the examples directory for examples.
#
# :param testname: the name of the test
# :type testname: string
# :param LAUNCH_FILE: the launch file used to start the test, required
# :type LAUNCH_FILE: string
# :param TIMEOUT: the test timeout in seconds, default: 60
# :type TIMEOUT: integer
# :param RESULT_FILE: the path of the result file from the test
#   default: ${AMENT_TEST_RESULTS_DIR}/${PROJECT_NAME}/${testname}.xml
# :type RESULT_FILE: string
# :param RESULT_FILE_PARAMETER_NAME: the name of the parameter that sets the
#   location of the result file in the launch file. Should not be modified if using
#   launch_catch_ros2. Default: "result_file"
# :type RESULT_FILE_PARAMETER_NAME: string

function(catch_ros2_add_integration_test testname)
  cmake_parse_arguments(ARG
    ""
    "LAUNCH_FILE;TIMEOUT;RESULT_FILE;RESULT_FILE_PARAMETER_NAME"
    ""
    ${ARGN})
  if(ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "catch_ros2_add_integration_test() called with unused arguments: "
      "${ARG_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT ${ament_cmake_FOUND})
    find_package(ament_cmake REQUIRED)
  endif()

  if(NOT ARG_LAUNCH_FILE)
    message(FATAL_ERROR
    "catch_ros2_add_integration_test() must be invoked with the LAUNCH_FILE argument")
  endif()
  if(NOT ARG_RESULT_FILE)
    set(ARG_RESULT_FILE "${AMENT_TEST_RESULTS_DIR}/${PROJECT_NAME}/${testname}.xml")
  endif()
  if(NOT ARG_TIMEOUT)
    set(ARG_TIMEOUT 60)
  endif()
  if(NOT ARG_TIMEOUT GREATER 0)
    message(FATAL_ERROR "catch_ros2_add_integration_test() the TIMEOUT argument must be a "
      "valid number and greater than zero")
  endif()
  if(NOT ARG_RESULT_FILE_PARAMETER_NAME)
    set(ARG_RESULT_FILE_PARAMETER_NAME "result_file")
  endif()

  ament_add_test(${testname}
    RESULT_FILE ${ARG_RESULT_FILE}
    COMMAND ros2 launch ${PROJECT_NAME} ${ARG_LAUNCH_FILE} ${ARG_RESULT_FILE_PARAMETER_NAME}:=${ARG_RESULT_FILE}
    RUNNER ${catch_ros2_DIR}/../scripts/run_test.py
    TIMEOUT ${ARG_TIMEOUT}
  )
endfunction()