# Copyright 2023 Open Source Robotics Foundation, Inc.
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

##################################################
# A function that converts proto package into a filesystem path
# (e.g. gz.msgs -> gz/msgs)
# Arguments:
#   PROTO_PACKAGE - Input package name to convert to path
#   PROTO_PACKAGE_PATH - Output formatted path
function(_gz_msgs_proto_pkg_to_path PROTO_PACKAGE PROTO_PACKAGE_PATH)
  if (PROTO_PACKAGE)
    string(REPLACE "." "/" PACKAGE_PATH ${PROTO_PACKAGE})
  else()
    set(PACKAGE_PATH ".")
  endif()
  set(${PROTO_PACKAGE_PATH} ${PACKAGE_PATH} PARENT_SCOPE)
endfunction()

##################################################
# A function that converts proto package into a string with underscores
# (e.g. gz.msgs -> gz_msgs)
# Arguments:
#   PROTO_PACKAGE - Input package name to convert to path
#   PROTO_PACKAGE_PATH - Output formatted path
function(_gz_msgs_proto_pkg_to_string PROTO_PACKAGE PROTO_PACKAGE_STRING)
  string(REPLACE "." "_" PACKAGE_STRING ${PROTO_PACKAGE})
  set(${PROTO_PACKAGE_STRING} ${PACKAGE_STRING} PARENT_SCOPE)
endfunction()


##################################################
# A function that converts a proto file and package into a unique string
# (e.g. gz.msgs.foobar -> gz_msgs_foobar)
# Arguments:
#   PROTO_FILE - Input Full path to proto file
#   PROTO_PACKAGE - Input proto package name
#   UNIQUE_NAME - Output unique name
function(_gz_msgs_proto_to_unique PROTO_FILE PROTO_PACKAGE UNIQUE_NAME)
  # Get the filename without extension or directory
  get_filename_component(FIL_WE ${PROTO_FILE} NAME_WE)
  _gz_msgs_proto_pkg_to_string(${PROTO_PACKAGE} PROTO_PACKAGE_STRING)
  set(${UNIQUE_NAME} "${PROTO_PACKAGE_STRING}_${FIL_WE}" PARENT_SCOPE)
endfunction()
