#===============================================================================
# Copyright (C) 2018 Open Source Robotics Foundation
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
# Find jsoncpp.
#
# Usage of this module as follows:
#
#     find_package(JSONCPP)
#
# Variables defined by this module:
#
#  JSONCPP_FOUND              System has JSONCPP libs/headers
#  JSONCPP_INCLUDE_DIRS       The location of JSONCPP headers
#  JSONCPP_LIBRARIES          The JSONCPP libraries
#  JSONCPP_TARGET             Imported target for libjsoncpp

if(JSONCPP_FIND_VERSION)
  message(WARNING "FindJSONCPP doesn't support the request of specific "
                  "versions. Please do not use VERSION.")
else()
  find_package(jsoncpp CONFIG QUIET)
  set(JSONCPP_TARGET jsoncpp_lib)
  include(GzPkgConfig)

  if(JSONCPP_FOUND)
    gz_pkg_config_entry(JSONCPP jsoncpp)
  else()
    gz_pkg_check_modules_quiet(JSONCPP jsoncpp)
    set(JSONCPP_TARGET JSONCPP::JSONCPP)

    # If that failed, then fall back to manual detection.
    if(NOT JSONCPP_FOUND)

      if(NOT JSONCPP_FIND_QUIETLY)
        message(STATUS "Attempting manual search for jsoncpp")
      endif()

      find_path(JSONCPP_INCLUDE_DIRS json/json.h ${JSONCPP_INCLUDE_DIRS} ENV CPATH)
      find_library(JSONCPP_LIBRARIES NAMES jsoncpp)
      set(JSONCPP_FOUND true)

      if(NOT JSONCPP_INCLUDE_DIRS)
        if(NOT JSONCPP_FIND_QUIETLY)
          message(STATUS "Looking for jsoncpp headers - not found")
        endif()
        set(JSONCPP_FOUND false)
      endif()

      if(NOT JSONCPP_LIBRARIES)
        if(NOT JSONCPP_FIND_QUIETLY)
          message (STATUS "Looking for jsoncpp library - not found")
        endif()
        set(JSONCPP_FOUND false)
      endif()

      if(JSONCPP_FOUND)
        include(GzImportTarget)
        gz_import_target(JSONCPP)
      endif()
    endif()
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
      JSONCPP
      REQUIRED_VARS JSONCPP_FOUND)
  endif()
endif()
