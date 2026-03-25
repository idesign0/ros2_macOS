#
# 2012-01-31, Lars Bilke
# - Enable Code Coverage
#
# 2013-09-17, Joakim Söderberg
# - Added support for Clang.
# - Some additional usage instructions.
#
# 2017-09-13
# - Tweaked instructions for Gazebo libraries
# - Tweaked function name to avoid name collisions
#
# USAGE:
# 1. Add the following line to your CMakeLists.txt:
#      include(GzCodeCoverage)
#
# 2. Set compiler flags to turn off optimization and enable coverage:
#    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
#    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
#
# 3. Use the function GZ_SETUP_TARGET_FOR_COVERAGE to create a custom make target
#    which runs your test executable and produces a lcov code coverage report:
#    Example:
#    GZ_SETUP_TARGET_FOR_COVERAGE(
#        my_coverage_target  # Name for custom target.
#        test_driver         # Name of the test driver executable that runs the tests.
#                            # NOTE! This should always have a ZERO as exit code
#                            # otherwise the coverage generation will not complete.
#        coverage            # Name of output directory.
#        )
#
# 4. Build a Coverge build:
#   cmake -DCMAKE_BUILD_TYPE=Coverage ..
#   make
#   make my_coverage_target
#
#

# Check prereqs
FIND_PROGRAM( GCOV_PATH gcov )
FIND_PROGRAM( LCOV_PATH lcov )
FIND_PROGRAM( GREP_PATH grep )
FIND_PROGRAM( GENHTML_PATH genhtml )
FIND_PROGRAM( GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/tests)

IF(NOT GCOV_PATH)
  MESSAGE(FATAL_ERROR "gcov not found! Aborting...")
ENDIF() # NOT GCOV_PATH

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
  # Clang version 3.0.0 and greater now supports gcov as well.
  MESSAGE(WARNING "Compiler is not GNU gcc! Clang Version 3.0.0 and greater supports gcov as well, but older versions don't.")

  IF(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    MESSAGE(FATAL_ERROR "Compiler is not GNU gcc! Aborting...")
  ENDIF()
ENDIF() # NOT CMAKE_COMPILER_IS_GNUCXX

# Convert to uppercase in order to support arbitrary capitalization
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPERCASE)
IF ( NOT (CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "DEBUG" OR CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "COVERAGE"))
  MESSAGE( WARNING "Code coverage results with an optimized (non-Debug) build may be misleading" )
ENDIF() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"


#################################################
# gz_setup_target_for_coverage(
#     [BRANCH_COVERAGE]
#     [OUTPUT_NAME <output_name>]
#     [TARGET_NAME <target_name>]
#     [TEST_RUNNER <test_runner>])
#
# This function will create custom coverage targets with the specified options.
#
# Coverage is not run for files in the following formats:
#
# *.cxx : We assume these files are created by swig.
# moc_*.cpp and qrc_*.cpp : We assume these files are created by Qt's meta-object compiler.
#
# BRANCH_COVERAGE:  Optional. If provided, branch coverage will be computed
#                   instead of line coverage.
#
# OUTPUT_NAME:  Required.
#               lcov output is generated as _outputname.info
#               HTML report is generated in _outputname/index.html
#
# TARGET_NAME:  The name of new the custom make target.
#
# TEST_RUNNER:  The name of the target which runs the tests.
#               MUST return ZERO always, even on errors.
#               If not, no coverage report will be created!
#
FUNCTION(gz_setup_target_for_coverage)

  #------------------------------------
  # Define the expected arguments
  set(options "BRANCH_COVERAGE")
  set(oneValueArgs "OUTPUT_NAME" "TARGET_NAME" "TEST_RUNNER")
  set(multiValueArgs)

  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(_outputname ${gz_coverage_OUTPUT_NAME})
  set(_targetname ${gz_coverage_TARGET_NAME})
  set(_testrunner ${gz_coverage_TEST_RUNNER})

  if(gz_coverage_BRANCH_COVERAGE)
    set(_branch_flags --rc lcov_branch_coverage=1)
  endif()

  IF(NOT LCOV_PATH)
    MESSAGE(FATAL_ERROR "lcov not found! Aborting...")
  ENDIF() # NOT LCOV_PATH

  IF(NOT GREP_PATH)
    MESSAGE(FATAL_ERROR "grep not found! Run code coverage on linux or mac.")
  ENDIF()

  IF(NOT GENHTML_PATH)
    MESSAGE(FATAL_ERROR "genhtml not found! Aborting...")
  ENDIF() # NOT GENHTML_PATH

  # Read ignore file list
  if (EXISTS "${PROJECT_SOURCE_DIR}/coverage.ignore.in")
    configure_file("${PROJECT_SOURCE_DIR}/coverage.ignore.in"
                    ${PROJECT_BINARY_DIR}/coverage.ignore)
    file (STRINGS "${PROJECT_BINARY_DIR}/coverage.ignore" IGNORE_LIST_RAW)
    string(REGEX REPLACE "([^;]+)" "'${PROJECT_SOURCE_DIR}/\\1'" IGNORE_LIST "${IGNORE_LIST_RAW}")
    message(STATUS "Ignore coverage additions: " ${IGNORE_LIST})
  else()
    set(IGNORE_LIST "")
  endif()

  # Setup target
  ADD_CUSTOM_TARGET(${_targetname}

    COMMAND ${CMAKE_COMMAND} -E remove ${_outputname}.info.cleaned
      ${_outputname}.info
    # Capturing lcov counters and generating report
    COMMAND ${LCOV_PATH} ${_branch_flags} -q --no-checksum
      --directory ${PROJECT_BINARY_DIR} --capture
      --output-file ${_outputname}.info 2>/dev/null
    # Remove negative counts
    COMMAND sed -i '/,-/d' ${_outputname}.info
    COMMAND ${LCOV_PATH} ${_branch_flags} -q
      --remove ${_outputname}.info '*/test/*' '/usr/*' '*_TEST*' '*.cxx' 'moc_*.cpp' 'qrc_*.cpp' '*.pb.*' '*/build/*' '*/install/*' ${IGNORE_LIST} --output-file ${_outputname}.info.cleaned
    COMMAND ${GENHTML_PATH} ${_branch_flags} -q --prefix ${PROJECT_SOURCE_DIR}
    --legend -o ${_outputname} ${_outputname}.info.cleaned
    COMMAND ${LCOV_PATH} --summary ${_outputname}.info.cleaned 2>&1 | grep "lines" | cut -d ' ' -f 4 | cut -d '%' -f 1 > ${_outputname}/lines.txt
    COMMAND ${LCOV_PATH} --summary ${_outputname}.info.cleaned 2>&1 | grep "functions" | cut -d ' ' -f 4 | cut -d '%' -f 1 > ${_outputname}/functions.txt
    COMMAND ${LCOV_PATH} ${_branch_flags}
      --summary ${_outputname}.info.cleaned 2>&1 | grep "branches" | cut -d ' ' -f 4 | cut -d '%' -f 1 > ${_outputname}/branches.txt
    COMMAND ${CMAKE_COMMAND} -E rename ${_outputname}.info.cleaned
      ${_outputname}.info

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Resetting code coverage counters to zero.\n"
      "Processing code coverage counters and generating report."
  )

  # Show info where to find the report
  ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
    COMMAND COMMAND ${LCOV_PATH} -q --zerocounters --directory ${PROJECT_BINARY_DIR};
    COMMENT "Open ./${_outputname}/index.html in your browser to view the coverage report."
  )

ENDFUNCTION() # GZ_SETUP_TARGET_FOR_COVERAGE
