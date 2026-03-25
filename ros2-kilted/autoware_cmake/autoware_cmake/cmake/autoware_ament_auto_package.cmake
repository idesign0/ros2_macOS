# Copyright 2025 The Autoware Contributors
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

macro(autoware_ament_auto_package)
  # cSpell:ignore ARGN
  cmake_parse_arguments(_ARG_AUTOWARE_AMENT_AUTO_PACKAGE
    "INSTALL_TO_PATH"
    ""
    "INSTALL_TO_SHARE"
    ${ARGN})

  # Export all found build dependencies which are also run dependencies
  set(_run_depends
    ${${PROJECT_NAME}_BUILD_EXPORT_DEPENDS}
    ${${PROJECT_NAME}_BUILDTOOL_EXPORT_DEPENDS}
    ${${PROJECT_NAME}_EXEC_DEPENDS})
  foreach(_dep
      ${${PROJECT_NAME}_FOUND_BUILD_DEPENDS}
      ${${PROJECT_NAME}_FOUND_BUILDTOOL_DEPENDS})
    if(_dep IN_LIST _run_depends)
      ament_export_dependencies("${_dep}")
    endif()
  endforeach()

  # Export and install include directory maintaining Autoware structure
  # Always use "include" as destination (not "include/${PROJECT_NAME}")
  # to maintain Autoware's naming convention across all ROS 2 versions
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
    ament_export_include_directories("include")
    install(DIRECTORY include/ DESTINATION include
      FILES_MATCHING
      PATTERN "*.h"
      PATTERN "*.hpp"
      PATTERN "*.hh"
      PATTERN "*.hxx"
    )
  endif()

  # Export and install all libraries
  if(NOT ${PROJECT_NAME}_LIBRARIES STREQUAL "")
    ament_export_libraries(${${PROJECT_NAME}_LIBRARIES})
    install(
      TARGETS ${${PROJECT_NAME}_LIBRARIES}
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
    )
  endif()

  # Install executables
  if(NOT ${PROJECT_NAME}_EXECUTABLES STREQUAL "")
    if(_ARG_AUTOWARE_AMENT_AUTO_PACKAGE_INSTALL_TO_PATH)
      set(_executable_destination "bin")
    else()
      set(_executable_destination "lib/${PROJECT_NAME}")
    endif()
    install(
      TARGETS ${${PROJECT_NAME}_EXECUTABLES}
      DESTINATION ${_executable_destination}
    )
  endif()

  # Install additional directories to share
  foreach(_dir ${_ARG_AUTOWARE_AMENT_AUTO_PACKAGE_INSTALL_TO_SHARE})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}")
      install(DIRECTORY "${_dir}/" DESTINATION "share/${PROJECT_NAME}/${_dir}")
    endif()
  endforeach()

  # Call ament_package with any unparsed arguments
  set(_unparsed_args ${_ARG_AUTOWARE_AMENT_AUTO_PACKAGE_UNPARSED_ARGUMENTS})
  ament_package(${_unparsed_args})
endmacro()
