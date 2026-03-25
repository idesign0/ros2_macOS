# Copyright 2020 Canonical, Ltd.
#
# This program is free software: you can redistribute it and/or modify it under the terms of the
# GNU Limited General Public License version 3, as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranties of MERCHANTABILITY, SATISFACTORY QUALITY,
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Limited General Public License for more details.
#
# You should have received a copy of the GNU Limited General Public License along with this program.
# If not, see <http://www.gnu.org/licenses/>.

find_package(ament_cmake_core QUIET REQUIRED)
ament_register_extension("ament_package" "ament_nodl"
  "nodl_package_hook.cmake")

include("${ament_nodl_DIR}/nodl_export_node_description_file.cmake")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_FLAGS MATCHES "-stdlib=libc\\+\\+")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    set(FILESYSTEM_LIB c++experimental)
  else()
    set(FILESYSTEM_LIB c++fs)
  endif()
else()
  set(FILESYSTEM_LIB stdc++fs)
endif()

if(UNIX AND NOT APPLE)
  # this is needed to use the experimental/filesystem on Linux, but cannot be passed with
  # ament_export_libraries() because it is not absolute and cannot be found with find_library
  list(APPEND nodl_LIBRARIES ${FILESYSTEM_LIB})
endif()
