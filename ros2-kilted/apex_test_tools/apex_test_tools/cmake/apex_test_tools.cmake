# Copyright 2018-2020 Apex.AI, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# target is a target

macro(_setup_memory_tools_once)
  if(NOT _setup_memory_tools_once_flag)
    set(_setup_memory_tools_once_flag TRUE)
    find_package(osrf_testing_tools_cpp REQUIRED)
    get_target_property(__extra_memory_tools_env_vars
      osrf_testing_tools_cpp::memory_tools LIBRARY_PRELOAD_ENVIRONMENT_VARIABLE)
  endif()
endmacro()

macro(apex_test_tools_add_gtest target)
  _setup_memory_tools_once()
  ament_add_gtest(${target}
    ${ARGN}
    ENV ${__extra_memory_tools_env_vars}
  )

  target_link_libraries(${target} osrf_testing_tools_cpp::memory_tools)
  ament_target_dependencies(${target} apex_test_tools)
endmacro()
