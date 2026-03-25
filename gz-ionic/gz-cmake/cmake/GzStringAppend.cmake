# Copyright (C) 2023 Open Source Robotics Foundation
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

#################################################
# gz_string_append(<output_var> <value_to_append> [DELIM <delimiter>])
#
# <output_var>: The name of the string variable that should be appended to
#
# <value_to_append>: The value that should be appended to the string
#
# [DELIM]: Specify a delimiter to separate the contents with. Default value is a
#          space
#
# Macro to append a value to a string
macro(gz_string_append output_var val)
  # Define the expected arguments
  # NOTE: options cannot be set to PARENT_SCOPE alone, so we put it explicitly
  # into cmake_parse_arguments(~). We use a semicolon to concatenate it with
  # this options variable, so all other options should be specified here.
  set(options)
  set(oneValueArgs DELIM)
  set(multiValueArgs)
  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_string_append "PARENT_SCOPE;${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(gz_string_append_DELIM)
    set(delim "${gz_string_append_DELIM}")
  else()
    set(delim " ")
  endif()

  if( (NOT ${output_var}) OR (${output_var} STREQUAL "") )
    # If ${output_var} is blank, just set it to equal ${val}
    set(${output_var} "${val}")
  else()
    # If ${output_var} already has a value in it, append ${val} with the
    # delimiter in-between.
    set(${output_var} "${${output_var}}${delim}${val}")
  endif()

  if(gz_string_append_PARENT_SCOPE)
    set(${output_var} "${${output_var}}" PARENT_SCOPE)
  endif()

endmacro()
