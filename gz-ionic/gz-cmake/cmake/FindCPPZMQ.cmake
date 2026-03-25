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
#===============================================================================
# Find cppzmq
#
# Usage of this module as follows:
#
#     find_package(CPPZMQ)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  CPPZMQ_HEADER_PATH        Set this variable to the location of the zmp.hpp
#                            header file if the module has problems finding it.
#  ZeroMQ_INCLUDE_DIRS       Set this variable to the include directories of
#                            ZeroMQ if the module has problems finding
#                            the proper installation path.
#
# Variables defined by this module:
#
#  CPPZMQ_FOUND              System has CPPZMQ header
#  CPPZMQ_INCLUDE_DIRS       The location of zmq.hpp header

# If we cannot find the header, we will switch this to false
set(CPPZMQ_FOUND true)

# Search for the header
find_path(CPPZMQ_INCLUDE_DIRS zmq.hpp
          PATHS
            ${ZeroMQ_INCLUDE_DIRS}
            ${CPPZMQ_HEADER_PATH})
mark_as_advanced(CPPZMQ_INCLUDE_DIRS)

if(NOT CPPZMQ_INCLUDE_DIRS)
  set(CPPZMQ_FOUND false)
endif()

if(CPPZMQ_FOUND)

  include(GzImportTarget)

  # Since this is a header-only library, we should import it as an INTERFACE
  # target.
  gz_import_target(CPPZMQ INTERFACE)

  # Now, to use the CPPZMQ headers, you should call
  # target_link_libraries(<tgt> CPPZMQ::CPPZMQ) instead of using
  # target_include_directories(~)

endif()
