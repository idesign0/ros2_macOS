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

if(NODL_EXPORT_NODE_DESCRIPTION_FILE_CMAKE_GUARD)
  return()
endif()
set(NODL_EXPORT_NODE_DESCRIPTION_FILE_CMAKE_GUARD TRUE)

#
# Export a node interface description file provided by the current package.
#
# In this macro "Export" implies both installing the file to
# the current package's share folder as well as installing the
# required ament index resources needed for the interface description to be
# discovered later.
#
# The relative_filename should be relative to the CMake file that calls this
# macro.
#
# The relative path to this file will be preserved when being installed.
# For example, if that argument is "nodl/my_desc.xml", then it will be
# installed to "<prefix>/share/<package>/nodl/my_desc.xml" or if the
# argument is just "my_desc.xml" it would be installed to
# "<prefix>/share/<package>/my_desc.xml".
#
# This macro would be used like this:
#
#   nodl_export_node_description_file("my_desc.xml")
#
# This macro assumes the package name is PROJECT_NAME.
#
# :param relative_filename: relative path to the node description file
# :type relative_filename: string
#
# @public
#
macro(nodl_export_node_description_file relative_filename)
  set(abs_filename "${CMAKE_CURRENT_SOURCE_DIR}/${relative_filename}")
  if(NOT EXISTS "${abs_filename}")
    message(FATAL_ERROR "Given node description file '${abs_filename}' does not exist")
  endif()

  set(relative_dir "")
  get_filename_component(relative_dir "${relative_filename}" DIRECTORY)
  install(FILES ${relative_filename} DESTINATION share/${PROJECT_NAME}/${relative_dir})

  # this accumulated value is written to the ament index resource file in the
  # ament_package() call via the node hook
  set(__NODL_CATEGORY_CONTENT__
    "${__NODL_CATEGORY_CONTENT__}share/${PROJECT_NAME}/${relative_filename}\n")
endmacro()
