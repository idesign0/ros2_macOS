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
# Find curl.
#
# Usage of this module as follows:
#
#     find_package(GzCURL)
#
# Variables defined by this module:
#
#  curl::curl                 Imported target for libcurl
#
#  GzCURL_FOUND              System has CURL libs/headers
#  GzCURL_INCLUDE_DIRS       The location of CURL headers
#  GzCURL_LIBRARIES          The CURL libraries
#  GzCURL_VERSION            The version of CURL found

set(gz_quiet_arg)
if(GzCURL_FIND_QUIETLY)
  set(gz_quiet_arg QUIET)
endif()

find_package(CURL ${GzCURL_FIND_VERSION} ${gz_quiet_arg})

set(GzCURL_FOUND ${CURL_FOUND})

if(${GzCURL_FOUND})

  set(GzCURL_INCLUDE_DIRS ${CURL_INCLUDE_DIRS})
  set(GzCURL_LIBRARIES ${CURL_LIBRARIES})
  set(GzCURL_VERSION ${CURL_VERSION_STRING})

  # Older versions of curl don't create imported targets, so we will create
  # them here if they have not been provided.
  if(TARGET CURL::libcurl AND NOT TARGET curl::curl)
    add_library(curl::curl INTERFACE IMPORTED)
    set_target_properties(curl::curl PROPERTIES
        INTERFACE_LINK_LIBRARIES CURL::libcurl)
  endif()

  include(GzImportTarget)

  if(NOT TARGET curl::curl)
    gz_import_target(curl
      LIB_VAR CURL_LIBRARIES
      INCLUDE_VAR CURL_INCLUDE_DIRS)
  endif()

  include(GzPkgConfig)
  gz_pkg_config_entry(GzCURL "libcurl >= ${GzCURL_FIND_VERSION}")

endif()
