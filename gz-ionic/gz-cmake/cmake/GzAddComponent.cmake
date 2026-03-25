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
# gz_add_component(<component>
#                   SOURCES <sources> | INTERFACE
#                   [DEPENDS_ON_COMPONENTS <components...>]
#                   [INCLUDE_SUBDIR <subdirectory_name>]
#                   [GET_TARGET_NAME <output_var>]
#                   [INDEPENDENT_FROM_PROJECT_LIB]
#                   [PRIVATELY_DEPENDS_ON_PROJECT_LIB]
#                   [INTERFACE_DEPENDS_ON_PROJECT_LIB]
#                   [CXX_STANDARD <11|14|17>]
#                   [PRIVATE_CXX_STANDARD <11|14|17>]
#                   [INTERFACE_CXX_STANDARD <11|14|17>])
#
# This function will produce a "component" library for your project. This is the
# recommended way to produce plugins or library modules.
#
# <component>: Required. Name of the component. The final name of this library
#              and its target will be gz-<project><major_ver>-<component>
#
# SOURCES: Required (unless INTERFACE is specified). Specify the source files
#          that will be used to generate the library.
#
# INTERFACE: Indicate that this is an INTERFACE library which does not require
#            any source files. This is required if SOURCES is not specified.
#
# [DEPENDS_ON_COMPONENTS]: Specify a list of other components of this package
#                          that this component depends on. This argument should
#                          be considered mandatory whenever there are
#                          inter-component dependencies in an Gazebo package.
#
# [INCLUDE_SUBDIR]: Optional. If specified, the public include headers for this
#                   component will go into "gz/<project>/<subdirectory_name>/".
#                   If not specified, they will go into "gz/<project>/<component>/"
#
# [GET_TARGET_NAME]: Optional. The variable that follows this argument will be
#                    set to the library target name that gets produced by this
#                    function. The target name will always be
#                    ${PROJECT_LIBRARY_TARGET_NAME}-<component>.
#
# [INDEPENDENT_FROM_PROJECT_LIB]:
#     Optional. Specify this if you do NOT want this component to automatically
#     be linked to the core library of this project. The default behavior is to
#     be publically linked.
#
# [PRIVATELY_DEPENDS_ON_PROJECT_LIB]:
#     Optional. Specify this if this component privately depends on the core
#     library of this project (i.e. users of this component do not need to
#     interface with the core library). The default behavior is to be publicly
#     linked.
#
# [INTERFACE_DEPENDS_ON_PROJECT_LIB]:
#     Optional. Specify this if the component's interface depends on the core
#     library of this project (i.e. users of this component need to interface
#     with the core library), but the component itself does not need to link to
#     the core library.
#
# See the documentation of gz_create_core_library(~) for more information about
# specifying the C++ standard. If your component publicly depends on the core
# library, then you probably do not need to specify the standard, because it
# will get inherited from the core library.
function(gz_add_component component_name)
  # Define the expected arguments
  set(options INTERFACE INDEPENDENT_FROM_PROJECT_LIB PRIVATELY_DEPENDS_ON_PROJECT_LIB INTERFACE_DEPENDS_ON_PROJECT_LIB)
  set(oneValueArgs INCLUDE_SUBDIR GET_TARGET_NAME)
  set(multiValueArgs SOURCES DEPENDS_ON_COMPONENTS)
  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(gz_add_component "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
  endif()

  if(gz_add_component_SOURCES)
    set(sources ${gz_add_component_SOURCES})
  elseif(NOT gz_add_component_INTERFACE)
    message(FATAL_ERROR "You must specify SOURCES for gz_add_component(~)!")
  endif()

  if(gz_add_component_INCLUDE_SUBDIR)
    set(include_subdir ${gz_add_component_INCLUDE_SUBDIR})
  else()
    set(include_subdir ${component_name})
  endif()

  if(gz_add_component_INTERFACE)
    set(interface_option INTERFACE)
    set(property_type INTERFACE)
  else()
    set(interface_option) # Intentionally blank
    set(property_type PUBLIC)
  endif()

  # Set the name of the component's target
  set(component_target_name ${PROJECT_LIBRARY_TARGET_NAME}-${component_name})

  # Pass the component's target name back to the caller if requested
  if(gz_add_component_GET_TARGET_NAME)
    set(${gz_add_component_GET_TARGET_NAME} ${component_target_name} PARENT_SCOPE)
  endif()

  # Create an upper case version of the component name, to be used as an export
  # base name.
  string(TOUPPER ${component_name} component_name_upper)
  # hyphen is not supported as macro name, replace it by underscore
  string(REPLACE "-" "_" component_name_upper ${component_name_upper})

  #------------------------------------
  # Create the target for this component, and configure it to be installed
  _gz_add_library_or_component(
    LIB_NAME ${component_target_name}
    INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/${include_subdir}"
    EXPORT_BASE GZ_${GZ_DESIGNATION_UPPER}_${component_name_upper}
    SOURCES ${sources}
    ${interface_option})

  if(gz_add_component_INDEPENDENT_FROM_PROJECT_LIB  OR
     gz_add_component_PRIVATELY_DEPENDS_ON_PROJECT_LIB)

    # If we are not linking this component to the core library, then we need to
    # add these include directories to this component library directly. This is
    # not needed if we link to the core library, because that will pull in these
    # include directories automatically.
    target_include_directories(${component_target_name}
      ${property_type}
        # This is the publicly installed gz/math headers directory.
        "$<INSTALL_INTERFACE:${GZ_INCLUDE_INSTALL_DIR_FULL}>"
        # This is the in-build version of the core library's headers directory.
        # Generated headers for this component might get placed here, even if
        # the component is independent of the core library.
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>")

      file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include")

  endif()

  if(EXISTS "${PROJECT_SOURCE_DIR}/${component_name}/include")

    target_include_directories(${component_target_name}
      ${property_type}
        # This is the in-source version of the component-specific headers
        # directory. When exporting the target, this will not be included,
        # because it is tied to the build interface instead of the install
        # interface.
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${component_name}/include>")

  endif()

  target_include_directories(${component_target_name}
    ${property_type}
      # This is the in-build version of the component-specific headers
      # directory. Generated headers for this component might end up here.
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/${component_name}/include>")

  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${component_name}/include")

  #------------------------------------
  # Adjust variables if a specific C++ standard was requested
  _gz_handle_cxx_standard(gz_add_component
    ${component_target_name} ${component_name}_PKGCONFIG_CFLAGS)


  #------------------------------------
  # Adjust the packaging variables based on how this component depends (or not)
  # on the core library.
  if(gz_add_component_PRIVATELY_DEPENDS_ON_PROJECT_LIB)

    target_link_libraries(${component_target_name}
      PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})

  endif()

  if(gz_add_component_INTERFACE_DEPENDS_ON_PROJECT_LIB)

    target_link_libraries(${component_target_name}
      INTERFACE ${PROJECT_LIBRARY_TARGET_NAME})

  endif()

  if(NOT gz_add_component_INDEPENDENT_FROM_PROJECT_LIB AND
     NOT gz_add_component_PRIVATELY_DEPENDS_ON_PROJECT_LIB AND
     NOT gz_add_component_INTERFACE_DEPENDS_ON_PROJECT_LIB)

    target_link_libraries(${component_target_name}
      ${property_type} ${PROJECT_LIBRARY_TARGET_NAME})

  endif()

  if(NOT gz_add_component_INDEPENDENT_FROM_PROJECT_LIB)

    # Add the core library as a cmake dependency for this component
    # NOTE: It seems we need to triple-escape "${gz_package_required}" and
    #       "${gz_package_quiet}" here.
    gz_string_append(${component_name}_CMAKE_DEPENDENCIES
      "if(NOT ${PKG_NAME}_CONFIG_INCLUDED)\n  find_package(${PKG_NAME} ${PROJECT_VERSION_FULL_NO_SUFFIX} EXACT \\\${gz_package_quiet} \\\${gz_package_required})\nendif()" DELIM "\n")

    # Choose what type of pkgconfig entry the core library belongs to
    set(lib_pkgconfig_type ${component_name}_PKGCONFIG_REQUIRES)
    if(gz_add_component_PRIVATELY_DEPENDS_ON_PROJECT_LIB
        AND NOT gz_add_component_INTERFACE_DEPENDS_ON_PROJECT_LIB)
      set(lib_pkgconfig_type ${lib_pkgconfig_type}_PRIVATE)
    endif()

    gz_string_append(${lib_pkgconfig_type} "${PKG_NAME} = ${PROJECT_VERSION_FULL_NO_SUFFIX}")

  endif()

  if(gz_add_component_DEPENDS_ON_COMPONENTS)
    gz_string_append(${component_name}_CMAKE_DEPENDENCIES
      "find_package(${PKG_NAME} ${PROJECT_VERSION_FULL_NO_SUFFIX} EXACT \\\${gz_package_quiet} \\\${gz_package_required} COMPONENTS ${gz_add_component_DEPENDS_ON_COMPONENTS})" DELIM "\n")
  endif()

  #------------------------------------
  # Set variables that are needed by cmake/gz-component-config.cmake.in
  set(component_pkg_name ${component_target_name})
  if(gz_add_component_INTERFACE)
    set(component_pkgconfig_lib)
  else()
    set(component_pkgconfig_lib "-l${component_pkg_name}")
  endif()
  set(component_cmake_dependencies ${${component_name}_CMAKE_DEPENDENCIES})
  # This next set is redundant, but it serves as a reminder that this input
  # variable is used in config files
  set(component_name ${component_name})

  # ... and by cmake/pkgconfig/gz-component.pc.in
  set(component_pkgconfig_requires ${${component_name}_PKGCONFIG_REQUIRES})
  set(component_pkgconfig_requires_private ${${component_name}_PKGCONFIG_REQUIRES_PRIVATE})
  set(component_pkgconfig_lib_deps ${${component_name}_PKGCONFIG_LIBS})
  set(component_pkgconfig_lib_deps_private ${${component_name}_PKGCONFIG_LIBS_PRIVATE})
  set(component_pkgconfig_cflags ${${component_name}_PKGCONFIG_CFLAGS})

  # Export and install the cmake target and package information
  _gz_create_cmake_package(COMPONENT ${component_name})

  # Generate and install the pkgconfig information for this component
  _gz_create_pkgconfig(COMPONENT ${component_name})


  #------------------------------------
  # Add this component to the "all" target
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}-all INTERFACE ${lib_name})
  get_property(all_known_components TARGET ${PROJECT_LIBRARY_TARGET_NAME}-all
    PROPERTY INTERFACE_GZ_ALL_KNOWN_COMPONENTS)
  if(NOT all_known_components)
    set_property(TARGET ${PROJECT_LIBRARY_TARGET_NAME}-all
      PROPERTY INTERFACE_GZ_ALL_KNOWN_COMPONENTS "${component_target_name}")
  else()
    set_property(TARGET ${PROJECT_LIBRARY_TARGET_NAME}-all
      PROPERTY INTERFACE_GZ_ALL_KNOWN_COMPONENTS "${all_known_components};${component_target_name}")
  endif()
endfunction()
