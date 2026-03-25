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
cmake_minimum_required(VERSION 3.5)

project(test_apex_test_tools)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

if(BUILD_TESTING)
  # run common linters
  find_package(ament_lint_auto)
  ament_lint_auto_find_test_dependencies()

  # gtest
  set(MEMORY_TEST memory_gtest)

  find_package(ament_cmake_gtest REQUIRED)
  find_package(apex_test_tools REQUIRED)

  apex_test_tools_add_gtest(${MEMORY_TEST}
      test/src/test_memory.cpp)  
endif()

ament_auto_package()
