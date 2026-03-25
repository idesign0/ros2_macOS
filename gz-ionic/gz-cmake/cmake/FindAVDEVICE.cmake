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
# Find AV device.
set(av_major ${AVDEVICE_FIND_VERSION_MAJOR})
set(av_minor ${AVDEVICE_FIND_VERSION_MINOR})
set(av_patch ${AVDEVICE_FIND_VERSION_PATCH})

include(GzPkgConfig)
gz_pkg_check_modules_quiet(AVDEVICE "libavdevice >= ${av_major}.${av_minor}.${av_patch}")

if(NOT AVDEVICE_FOUND)
  include(GzManualSearch)
  gz_manual_search(AVDEVICE
                   HEADER_NAMES "libavdevice/avdevice.h"
                   LIBRARY_NAMES "avdevice")

  # Version check
  if(AVDEVICE_FOUND)
    file(READ "${AVDEVICE_INCLUDE_DIRS}/libavdevice/version.h" ver_file)

    # ffmpeg 5.1 splitted version information in two files
    # https://github.com/FFmpeg/FFmpeg/commit/884c5976592c2d8084e8c9951c94ddf04019d81d
    if(EXISTS "${AVDEVICE_INCLUDE_DIRS}/libavdevice/version_major.h")
      file(READ "${AVDEVICE_INCLUDE_DIRS}/libavdevice/version_major.h" ver_major_file)
      string(CONCAT ver_file ${ver_file} ${ver_major_file})
    endif()

    string(REGEX MATCH "LIBAVDEVICE_VERSION_MAJOR[ \t\r\n]+([0-9]*)" _ ${ver_file})
    set(ver_major ${CMAKE_MATCH_1})

    string(REGEX MATCH "LIBAVDEVICE_VERSION_MINOR[ \t\r\n]+([0-9]*)" _ ${ver_file})
    set(ver_minor ${CMAKE_MATCH_1})

    string(REGEX MATCH "LIBAVDEVICE_VERSION_MICRO[ \t\r\n]+([0-9]*)" _ ${ver_file})
    set(ver_patch ${CMAKE_MATCH_1})

    set(AVDEVICE_VERSION "${ver_major}.${ver_minor}.${ver_patch}")

    if(AVDEVICE_VERSION VERSION_LESS AVDEVICE_FIND_VERSION)
      set(AVDEVICE_FOUND FALSE)
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  AVDEVICE
  REQUIRED_VARS AVDEVICE_FOUND)
