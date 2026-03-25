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

include(GzAddComponent)
include(GzBuildExamples)
include(GzBuildExecutables)
include(GzBuildTests)
include(GzCmakeLogging)
include(GzCreateCoreLibrary)
include(GzCxxStandard)
include(GzFindPackage)
include(GzGetLibSourcesAndUnitTests)
include(GzGetSources)
include(GzInstallAllHeaders)
include(GzRelocatableBinaries)
include(GzStringAppend)

#################################################
# assert that a file exists
# From ament/ament_cmake_core/core/assert_file_exists.cmake (Apache 2.0)
function(_gz_assert_file_exists filename error_message)
  if(NOT IS_ABSOLUTE "${filename}")
    set(filename "${CMAKE_CURRENT_LIST_DIR}/${filename}")
  endif()
  if(NOT EXISTS "${filename}")
    message(FATAL_ERROR "${error_message}")
  endif()
endfunction()

#################################################
# From ament/ament_cmake_core/core/stamp.cmake (Apache 2.0)
#   :param path:  file name
#
#   Uses ``configure_file`` to generate a file ``filepath.stamp`` hidden
#   somewhere in the build tree.  This will cause cmake to rebuild its
#   cache when ``filepath`` is modified.
#
function(_gz_stamp path)
  get_filename_component(filename "${path}" NAME)
  configure_file(
    "${path}"
    "${CMAKE_CURRENT_BINARY_DIR}/gz-cmake/stamps/${filename}.stamp"
    COPYONLY
  )
endfunction()

#################################################
# From ament/ament_cmake_core/core/string_ends_with.cmake (Apache 2.0)
# Check if a string ends with a specific suffix.
#
# :param str: the string
# :type str: string
# :param suffix: the suffix
# :type suffix: string
# :param var: the output variable name
# :type var: bool
#
function(_gz_string_ends_with str suffix var)
  string(LENGTH "${str}" str_length)
  string(LENGTH "${suffix}" suffix_length)
  set(value FALSE)
  if(NOT ${str_length} LESS ${suffix_length})
    math(EXPR str_offset "${str_length} - ${suffix_length}")
    string(SUBSTRING "${str}" ${str_offset} ${suffix_length} str_suffix)
    if(str_suffix STREQUAL suffix)
      set(value TRUE)
    endif()
  endif()
  set(${var} ${value} PARENT_SCOPE)
endfunction()

#################################################
# Macro to turn a list into a string
# Internal to gz-cmake.
macro(_gz_list_to_string _output _input_list)

  set(${_output})
  foreach(_item ${${_input_list}})
    # Append each item, and forward any extra options to gz_string_append, such
    # as DELIM or PARENT_SCOPE
    gz_string_append(${_output} "${_item}" ${ARGN})
  endforeach(_item)

endmacro()

#################################################
# Creates the `all` target. This function is private to gz-cmake.
function(_gz_create_all_target)

  add_library(${PROJECT_LIBRARY_TARGET_NAME}-all INTERFACE)

  install(
    TARGETS ${PROJECT_LIBRARY_TARGET_NAME}-all
    EXPORT ${PROJECT_LIBRARY_TARGET_NAME}-all
    LIBRARY DESTINATION ${GZ_LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${GZ_LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${GZ_BIN_INSTALL_DIR}
    COMPONENT libraries)

endfunction()

#################################################
# Exports the `all` target. This function is private to gz-cmake.
function(_gz_export_target_all)

  # find_all_pkg_components is used as a variable in gz-all-config.cmake.in
  set(find_all_pkg_components "")
  get_property(all_known_components TARGET ${PROJECT_LIBRARY_TARGET_NAME}-all
    PROPERTY INTERFACE_GZ_ALL_KNOWN_COMPONENTS)

  if(all_known_components)
    foreach(component ${all_known_components})
      gz_string_append(find_all_pkg_components "find_dependency(${component} ${PROJECT_VERSION_FULL_NO_SUFFIX} EXACT)" DELIM "\n")
    endforeach()
  endif()

  _gz_create_cmake_package(ALL)

endfunction()

#################################################
# Used internally by _gz_add_library_or_component to report argument errors
macro(_gz_add_library_or_component_arg_error missing_arg)

  message(FATAL_ERROR "gz-cmake developer error: Must specify "
                      "${missing_arg} to _gz_add_library_or_component!")

endmacro()

#################################################
# This is only meant for internal use by gz-cmake. If you are a consumer
# of gz-cmake, please use gz_create_core_library(~) or
# gz_add_component(~) instead of this.
#
# _gz_add_library_or_component(LIB_NAME <lib_name>
#                               INCLUDE_DIR <dir_name>
#                               EXPORT_BASE <export_base>
#                               SOURCES <sources>)
#
macro(_gz_add_library_or_component)

  # NOTE: The following local variables are used in the Export.hh.in file, so if
  # you change their names here, you must also change their names there:
  # - include_dir
  # - export_base
  # - lib_name
  #
  # - _gz_export_base

  #------------------------------------
  # Define the expected arguments
  set(options INTERFACE)
  set(oneValueArgs LIB_NAME INCLUDE_DIR EXPORT_BASE)
  set(multiValueArgs SOURCES)

  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(_gz_add_library "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(_gz_add_library_LIB_NAME)
    set(lib_name ${_gz_add_library_LIB_NAME})
  else()
    _gz_add_library_or_component_arg_error(LIB_NAME)
  endif()

  if(NOT _gz_add_library_INTERFACE)
    if(_gz_add_library_SOURCES)
      set(sources ${_gz_add_library_SOURCES})
    else()
      _gz_add_library_or_component_arg_error(SOURCES)
    endif()
  else()
    set(sources)
  endif()

  if(_gz_add_library_INCLUDE_DIR)
    set(include_dir ${_gz_add_library_INCLUDE_DIR})
  else()
    _gz_add_library_or_component_arg_error(INCLUDE_DIR)
  endif()

  if(_gz_add_library_EXPORT_BASE)
    set(export_base ${_gz_add_library_EXPORT_BASE})
  else()
    _gz_add_library_or_component_arg_error(EXPORT_BASE)
  endif()

  # check that export_base has no invalid symbols
  string(REPLACE "-" "_" export_base_replaced ${export_base})
  if(NOT ${export_base} STREQUAL ${export_base_replaced})
      message(FATAL_ERROR
        "export_base has a hyphen which is not"
        "supported by macros used for visibility")
  endif()

  #------------------------------------
  # Create the library target

  message(STATUS "Configuring library: ${lib_name}")

  if(_gz_add_library_INTERFACE)
    add_library(${lib_name} INTERFACE)
  else()
    add_library(${lib_name} ${sources})
  endif()

  if(NOT _gz_add_library_INTERFACE)

    #------------------------------------
    # Generate export macro headers
    # Note: INTERFACE libraries do not need the export header
    set(binary_include_dir
      "${CMAKE_BINARY_DIR}/include/${include_dir}")

    set(implementation_file_name "${binary_include_dir}/detail/Export.hh")

    include(GenerateExportHeader)
    # This macro will generate a header called detail/Export.hh which implements
    # some C-macros that are useful for exporting our libraries. The
    # implementation header does not provide any commentary or explanation for its
    # macros, so we tuck it away in the detail/ subdirectory, and then provide a
    # public-facing header that provides commentary for the macros.
    generate_export_header(${lib_name}
      BASE_NAME ${export_base}
      EXPORT_FILE_NAME ${implementation_file_name}
      EXPORT_MACRO_NAME DETAIL_${export_base}_VISIBLE
      NO_EXPORT_MACRO_NAME DETAIL_${export_base}_HIDDEN
      DEPRECATED_MACRO_NAME GZ_DEPRECATED_ALL_VERSIONS)

    set(install_include_dir
      "${GZ_INCLUDE_INSTALL_DIR_FULL}/${include_dir}")

    # Configure the installation of the automatically generated file.
    install(
      FILES "${implementation_file_name}"
      DESTINATION "${install_include_dir}/detail"
      COMPONENT headers)

    # Configure the public-facing header for exporting and deprecating. This
    # header provides commentary for the macros so that developers can know their
    # purpose.
    configure_file(
      "${GZ_CMAKE_DIR}/Export.hh.in"
      "${binary_include_dir}/Export.hh")

    # Configure the installation of the public-facing header.
    install(
      FILES "${binary_include_dir}/Export.hh"
      DESTINATION "${install_include_dir}"
      COMPONENT headers)

    set_target_properties(
      ${lib_name}
      PROPERTIES
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION_FULL})

  endif()

  #------------------------------------
  # Configure the installation of the target

  install(
    TARGETS ${lib_name}
    EXPORT ${lib_name}
    LIBRARY DESTINATION ${GZ_LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${GZ_LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${GZ_BIN_INSTALL_DIR}
    COMPONENT libraries)

endmacro()

#################################################
# _gz_cmake_parse_arguments(<prefix> <options> <oneValueArgs> <multiValueArgs> [ARGN])
#
# Set <prefix> to match the prefix that is given to cmake_parse_arguments(~).
# This should also match the name of the function or macro that called it.
#
# NOTE: This should only be used by functions inside of gz-cmake specifically.
# Other Gazebo projects should not use this macro.
#
macro(_gz_cmake_parse_arguments prefix options oneValueArgs multiValueArgs)

  cmake_parse_arguments(${prefix} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(${prefix}_UNPARSED_ARGUMENTS)

    # The user passed in some arguments that we don't currently recognize. We'll
    # emit a warning so they can check whether they're using the correct version
    # of gz-cmake.
    message(AUTHOR_WARNING
      "\nThe build script has specified some unrecognized arguments for ${prefix}(~):\n"
      "${${prefix}_UNPARSED_ARGUMENTS}\n"
      "Either the script has a typo, or it is using an unexpected version of gz-cmake. "
      "The version of gz-cmake currently being used is ${gz-cmake${GZ_CMAKE_VERSION_MAJOR}_VERSION}\n")

  endif()

endmacro()
