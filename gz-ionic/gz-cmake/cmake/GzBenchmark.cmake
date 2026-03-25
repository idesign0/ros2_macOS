#.rst
# GzBenchmark
# ------------
#
# _gz_add_version_info_target()
#
# Adds a target to generate build and system configuration information. This
# function is private to gz-cmake.
#
# gz_add_benchmarks()
#
# Adds a target to execute all available benchmarks and aggregate the results.
#
# USAGE:
# 1. Add the following line to your CMakeLists.txt
#    include(GzBenchmark)
#
# 2. Add the benchmark
#    gz_add_benchmarks(SOURCES ${benchmark_sources_list})
#
# 3. After building the project, use `make run_benchmarks` to execute and
#    aggregate benchmark results to ${CMAKE_BINARY_DIR}/benchmark_results
#
#===============================================================================
# Copyright (C) 2019 Open Source Robotics Foundation
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

function(_gz_add_version_info_target)
  # generate a version_info.json file that can be used to embed project
  # version information
  # While this command may look a bit unweildy, it creates a target
  # that forces the file to be regenerated at build time.
  add_custom_target(version_info_target
    COMMAND ${CMAKE_COMMAND}
      -Dinput_file=${GZ_CMAKE_DIR}/version_info.json.in
      -Doutput_file=${CMAKE_CURRENT_BINARY_DIR}/version_info.json
      -Drepository_root=${CMAKE_CURRENT_SOURCE_DIR}
      # Yes, these variables need to be passed in, because they won't
      # get properly set when invoked as a CMake script.
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DPROJECT_VERSION=${PROJECT_VERSION}
      -DPROJECT_VERSION_FULL=${PROJECT_VERSION_FULL}
      -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
      -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
      -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
      -DPROJECT_NAME=${PROJECT_NAME}
      -P ${GZ_CMAKE_DIR}/GzGenerateVersionInfo.cmake
  )
endfunction()

function(gz_add_benchmarks)
  cmake_parse_arguments(BENCHMARK "" "" "SOURCES" ${ARGN})

  if(NOT BUILD_TESTING)
    return()
  endif()

  find_package(benchmark)
  if(NOT benchmark_FOUND)
    message(WARNING "Unable to find google benchmark (libbenchmark-dev). Disabling benchmarks.")
    return()
  endif()

  gz_build_executables(
    PREFIX "BENCHMARK_"
    SOURCES ${BENCHMARK_SOURCES}
    LIB_DEPS benchmark::benchmark
    EXEC_LIST BENCHMARK_TARGETS
  )

  set(BENCHMARK_TARGETS_LIST "")
  foreach(benchmark ${BENCHMARK_TARGETS})
    list(APPEND BENCHMARK_TARGETS_LIST "$<TARGET_FILE:${benchmark}>")
  endforeach()

  _gz_add_version_info_target()

  file(GENERATE
    OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/benchmark_targets"
    CONTENT
    "${BENCHMARK_TARGETS_LIST}")

  add_custom_target(
    run_benchmarks
    COMMAND python3 ${GZ_CMAKE_BENCHMARK_DIR}/run_benchmarks.py
      --project-name ${PROJECT_NAME}
      --version-file ${CMAKE_CURRENT_BINARY_DIR}/version_info.json
      --benchmark-targets ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/benchmark_targets
      --results-root ${CMAKE_BINARY_DIR}/benchmark_results
  )
  add_dependencies(run_benchmarks ${BENCHMARK_TARGETS} version_info_target)
endfunction()
