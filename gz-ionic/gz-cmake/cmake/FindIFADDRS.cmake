#===============================================================================
# Copyright (C) 2017 Open Source Robotics Foundation
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
########################################
# Find ifaddrs

# If we cannot find the header or the library, we will switch this to false
set(IFADDRS_FOUND true)

# Find ifaddrs.h
find_path(IFADDRS_INCLUDE_DIRS ifaddrs.h)

if(NOT IFADDRS_INCLUDE_DIRS)
  set(IFADDRS_FOUND false)
endif()

if(IFADDRS_FOUND)

  include(GzImportTarget)

  # Since this is a header-only library, we should import it as an INTERFACE
  # target.
  gz_import_target(IFADDRS INTERFACE)

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  IFADDRS
  REQUIRED_VARS IFADDRS_FOUND)
