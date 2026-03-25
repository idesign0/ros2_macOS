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
# gz_create_core_library(SOURCES <sources>
#                         [CXX_STANDARD <11|14|17>]
#                         [PRIVATE_CXX_STANDARD <11|14|17>]
#                         [INTERFACE_CXX_STANDARD <11|14|17>]
#                         [GET_TARGET_NAME <output_var>]
#                         [LEGACY_PROJECT_PREFIX <prefix>])
#
# This function will produce the "core" library for your project. There is no
# need to specify a name for the library, because that will be determined by
# your project information.
#
# SOURCES: Required. Specify the source files that will be used to generate the
#          library.
#
# [GET_TARGET_NAME]: Optional. The variable that follows this argument will be
#                    set to the library target name that gets produced by this
#                    function. The target name will always be
#                    ${PROJECT_LIBRARY_TARGET_NAME}.
#
# [LEGACY_PROJECT_PREFIX]: Optional. The variable that follows this argument will be
#                          used as a prefix for the legacy cmake config variables
#                          <prefix>_LIBRARIES and <prefix>_INCLUDE_DIRS.
#
# If you need a specific C++ standard, you must also specify it in this
# function in order to ensure that your library's target properties get set
# correctly. The following is a breakdown of your choices:
#
# [CXX_STANDARD]: This library must compile using the specified standard, and so
#                 must any libraries which link to it.
#
# [PRIVATE_CXX_STANDARD]: This library must compile using the specified standard,
#                         but libraries which link to it do not need to.
#
# [INTERFACE_CXX_STANDARD]: Any libraries which link to this library must compile
#                           with the specified standard.
#
# Most often, you will want to use CXX_STANDARD, but there may be cases in which
# you want a finer degree of control. If your library must compile with a
# different standard than what is required by dependent libraries, then you can
# specify both PRIVATE_CXX_STANDARD and INTERFACE_CXX_STANDARD without any
# conflict. However, both of those arguments conflict with CXX_STANDARD, so you
# are not allowed to use either of them if you use the CXX_STANDARD argument.
#
function(gz_create_core_library)
  # Define the expected arguments
  set(options INTERFACE)
  set(oneValueArgs INCLUDE_SUBDIR LEGACY_PROJECT_PREFIX CXX_STANDARD PRIVATE_CXX_STANDARD INTERFACE_CXX_STANDARD GET_TARGET_NAME)
  set(multiValueArgs SOURCES)
  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(gz_create_core_library "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(gz_create_core_library_SOURCES)
    set(sources ${gz_create_core_library_SOURCES})
  elseif(NOT gz_create_core_library_INTERFACE)
    message(FATAL_ERROR "You must specify SOURCES for gz_create_core_library(~)!")
  endif()

  if(gz_create_core_library_INTERFACE)
    set(interface_option INTERFACE)
    set(property_type INTERFACE)
  else()
    set(interface_option) # Intentionally blank
    set(property_type PUBLIC)
  endif()

  #------------------------------------
  # Create the target for the core library, and configure it to be installed
  _gz_add_library_or_component(
    LIB_NAME ${PROJECT_LIBRARY_TARGET_NAME}
    INCLUDE_DIR "${PROJECT_INCLUDE_DIR}"
    EXPORT_BASE GZ_${GZ_DESIGNATION_UPPER}
    SOURCES ${sources}
    ${interface_option})

  # These generator expressions are necessary for multi-configuration generators
  # such as MSVC on Windows. They also ensure that our target exports its
  # headers correctly
  target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
    ${property_type}
      # This is the publicly installed headers directory.
      "$<INSTALL_INTERFACE:${GZ_INCLUDE_INSTALL_DIR_FULL}>"
      # This is the in-build version of the core library headers directory.
      # Generated headers for the core library get placed here.
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
      # Generated headers for the core library might also get placed here.
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/core/include>")

  # We explicitly create these directories to avoid false-flag compiler warnings
  file(MAKE_DIRECTORY
    "${PROJECT_BINARY_DIR}/include"
    "${PROJECT_BINARY_DIR}/core/include")

  if(EXISTS "${PROJECT_SOURCE_DIR}/include")
    target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
      ${property_type}
        # This is the build directory version of the headers. When exporting the
        # target, this will not be included, because it is tied to the build
        # interface instead of the install interface.
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")
  endif()

  if(EXISTS "${PROJECT_SOURCE_DIR}/core/include")
    target_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
      ${property_type}
        # This is the include directories for projects that put the core library
        # contents into its own subdirectory.
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/core/include>")
  endif()


  #------------------------------------
  # Adjust variables if a specific C++ standard was requested
  _gz_handle_cxx_standard(gz_create_core_library
    ${PROJECT_LIBRARY_TARGET_NAME} PROJECT_PKGCONFIG_CFLAGS)


  #------------------------------------
  # Handle cmake and pkgconfig packaging
  if(gz_create_core_library_INTERFACE)
    set(project_pkgconfig_core_lib) # Intentionally blank
  else()
    set(project_pkgconfig_core_lib "-l${PROJECT_NAME_LOWER}")
  endif()

  # Export and install the core library's cmake target and package information
  _gz_create_cmake_package(LEGACY_PROJECT_PREFIX ${gz_create_core_library_LEGACY_PROJECT_PREFIX})

  # Generate and install the core library's pkgconfig information
  _gz_create_pkgconfig()


  #------------------------------------
  # Pass back the target name if they ask for it.
  if(gz_create_core_library_GET_TARGET_NAME)
    set(${gz_create_core_library_GET_TARGET_NAME} ${PROJECT_LIBRARY_TARGET_NAME} PARENT_SCOPE)
  endif()

endfunction()
