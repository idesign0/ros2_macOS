# Copyright 2020 Open Source Robotics Foundation, Inc.
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

message(WARNING "GzPython is deprecated, use find_package(Python3) instead.")

set(GZ_PYTHON_VERSION "" CACHE STRING
  "Specify specific Python3 version to use ('major.minor' or 'versionMin...[<]versionMax')")

find_package(Python3 ${GZ_PYTHON_VERSION} QUIET)

# Tick-tock PYTHON_EXECUTABLE until Python3_EXECUTABLE is released
# TODO(jrivero) gz-cmake4: start the deprecation cycle of PYTHON_EXECUTABLE
if(Python3_EXECUTABLE AND NOT PYTHON_EXECUTABLE)
  set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
endif()
