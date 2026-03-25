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
# gz_target_interface_include_directories(<target> [include_targets])
#
# Add the INTERFACE_INCLUDE_DIRECTORIES of [include_targets] to the public
# INCLUDE_DIRECTORIES of <target>. This allows us to propagate the include
# directories of <target> along to any other libraries that depend on it.
#
# You MUST pass in targets to include, not directory names. We must not use
# explicit directory names here if we want our package to be relocatable.
function(gz_target_interface_include_directories name)

  foreach(include_target ${ARGN})
    target_include_directories(
      ${name} PUBLIC
      $<TARGET_PROPERTY:${include_target},INTERFACE_INCLUDE_DIRECTORIES>)
  endforeach()

endfunction()

#################################################
macro(gz_install_includes _subdir)
  install(FILES ${ARGN}
    DESTINATION ${GZ_INCLUDE_INSTALL_DIR}/${_subdir} COMPONENT headers)
endmacro()



#################################################
# gz_install_all_headers(
#   [EXCLUDE_FILES <excluded_headers>]
#   [EXCLUDE_DIRS  <dirs>]
#   [GENERATED_HEADERS <headers>]
#   [COMPONENT] <component>)
#
# From the current directory, install all header files, including files from all
# subdirectories (recursively). You can optionally specify directories or files
# to exclude from installation (the names must be provided relative to the current
# source directory).
#
# This will accept all files ending in *.h and *.hh. You may append an
# additional suffix (like .old or .backup) to prevent a file from being included.
#
# GENERATED_HEADERS should be generated headers which should be included by
# ${GZ_DESIGNATION}.hh. This will only add them to the header, it will not
# generate or install them.
#
# This will also run configure_file on gz_auto_headers.hh.in and config.hh.in
# and install them. This will NOT install any other files or directories that
# appear in the ${CMAKE_CURRENT_BINARY_DIR}.
#
# If the COMPONENT option is specified, this will skip over configuring a
# config.hh file since it would be redundant with the core library.
#
function(gz_install_all_headers)
  # Define the expected arguments
  set(options)
  set(oneValueArgs COMPONENT)
  set(multiValueArgs EXCLUDE_FILES EXCLUDE_DIRS GENERATED_HEADERS)
  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_install_all_headers "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #------------------------------------
  # Build the list of directories
  file(GLOB_RECURSE all_files LIST_DIRECTORIES TRUE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")
  list(SORT all_files)

  set(directories)
  foreach(f ${all_files})
    # Check if this file is a directory
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${f})

      # Check if it is in the list of excluded directories
      list(FIND gz_install_all_headers_EXCLUDE_DIRS ${f} f_index)

      set(append_file TRUE)
      foreach(subdir ${gz_install_all_headers_EXCLUDE_DIRS})

        # Check if ${f} contains ${subdir} as a substring
        string(FIND ${f} ${subdir} pos)

        # If ${subdir} is a substring of ${f} at the very first position, then
        # we should not include anything from this directory. This makes sure
        # that if a user specifies "EXCLUDE_DIRS foo" we will also exclude
        # the directories "foo/bar/..." and so on. We will not, however, exclude
        # a directory named "bar/foo/".
        if(${pos} EQUAL 0)
          set(append_file FALSE)
          break()
        endif()

      endforeach()

      if(append_file)
        list(APPEND directories ${f})
      endif()

    endif()
  endforeach()

  # Append the current directory to the list
  list(APPEND directories ".")

  #------------------------------------
  # Install all the non-excluded header directories along with all of their
  # non-excluded headers
  foreach(dir ${directories})

    # GLOB all the header files in dir
    file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${dir}/*.h" "${dir}/*.hh" "${dir}/*.hpp")
    list(SORT headers)

    # Remove the excluded headers
    if(headers)
      foreach(exclude ${gz_install_all_headers_EXCLUDE_FILES})
        list(REMOVE_ITEM headers ${exclude})
      endforeach()
    endif()

    # Add each header, prefixed by its directory, to the auto headers variable
    foreach(header ${headers})
      set(gz_headers "${gz_headers}#include <${PROJECT_INCLUDE_DIR}/${header}>\n")
    endforeach()

    if("." STREQUAL ${dir})
      set(destination "${GZ_INCLUDE_INSTALL_DIR_FULL}/${PROJECT_INCLUDE_DIR}")
    else()
      set(destination "${GZ_INCLUDE_INSTALL_DIR_FULL}/${PROJECT_INCLUDE_DIR}/${dir}")
    endif()

    install(
      FILES ${headers}
      DESTINATION ${destination}
      COMPONENT headers)

  endforeach()

  # Add generated headers to the list of includes
  foreach(header ${gz_install_all_headers_GENERATED_HEADERS})
      set(gz_headers "${gz_headers}#include <${PROJECT_INCLUDE_DIR}/${header}>\n")
  endforeach()

  if(gz_install_all_headers_COMPONENT)

    set(component_name ${gz_install_all_headers_COMPONENT})

    # Define the install directory for the "config" header
    # The "meta" header will be installed one folder above this
    set(config_header_install_dir ${GZ_INCLUDE_INSTALL_DIR_FULL}/${PROJECT_INCLUDE_DIR}/${component_name})

    # Define the input/output of the configuration for the component "meta" header
    set(meta_header_in ${GZ_CMAKE_DIR}/gz_auto_headers.hh.in)
    set(meta_header_out ${CMAKE_CURRENT_BINARY_DIR}/${component_name}.hh)

  else()

    # Define the install directory for the "config" header
    # The core "meta" header will be installed one folder above this
    set(config_header_install_dir ${GZ_INCLUDE_INSTALL_DIR_FULL}/${PROJECT_INCLUDE_DIR})

    # Define the input/output of the configuration for the core "meta" header
    set(meta_header_in ${GZ_CMAKE_DIR}/gz_auto_headers.hh.in)
    set(meta_header_out ${CMAKE_CURRENT_BINARY_DIR}/../${GZ_DESIGNATION}.hh)

  endif()

  # Generate the install directory for the "meta" header one folder above the "config" header
  cmake_path(SET meta_header_install_dir NORMALIZE ${config_header_install_dir}/..)

  # Generate the "meta" header that includes all of the headers
  configure_file(${meta_header_in} ${meta_header_out})

  # Install the "meta" header
  install(
    FILES ${meta_header_out}
    DESTINATION ${meta_header_install_dir}
    COMPONENT headers)

  # Define the input/output of the configuration for the "config" header
  set(config_header_in ${CMAKE_CURRENT_SOURCE_DIR}/config.hh.in)
  set(config_header_out ${CMAKE_CURRENT_BINARY_DIR}/config.hh)

  if(NOT gz_install_all_headers_COMPONENT)

    # Produce an error if the config file is missing
    #
    # TODO: Maybe we should have a generic config.hh.in file that we fall back
    # on if the project does not have one for itself?
    if(NOT EXISTS ${config_header_in})
      message(FATAL_ERROR
        "Developer error: You are missing the file [${config_header_in}]! "
        "Did you forget to move it from your project's cmake directory while "
        "migrating to the use of gz-cmake?")
    endif()

    # Generate the "config" header that describes our project configuration
    configure_file(${config_header_in} ${config_header_out})

    # Install the "config" header
    install(
      FILES ${config_header_out}
      DESTINATION ${config_header_install_dir}
      COMPONENT headers)

  endif()

endfunction()
