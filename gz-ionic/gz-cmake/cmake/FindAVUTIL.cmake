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
# Find avutil
include(GzPkgConfig)
gz_pkg_check_modules_quiet(AVUTIL libavutil)

if(NOT AVUTIL_FOUND)
  include(GzManualSearch)
  gz_manual_search(AVUTIL
                   HEADER_NAMES "libavutil/avutil.h"
                   LIBRARY_NAMES "avutil")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  AVUTIL
  REQUIRED_VARS AVUTIL_FOUND)
