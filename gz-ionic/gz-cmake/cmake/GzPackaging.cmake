#.rst
# GzPackaging
# ----------------
#
# _gz_setup_packages
#
# Sets up package information for a Gazebo library project.
# This function is internal to gz-cmake.
#
# gz_create_packages
#
# Creates a package for a Gazebo library project
#
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

#################################################
# Set up package information
macro(_gz_setup_packages)

  #============================================================================
  # Use GNUInstallDirs to get canonical paths.
  # We use this filesystem style on Windows as well, because (quite frankly)
  # Windows does not seem to have any sensible convention of its own for
  # installing development libraries. (If anyone is aware of a widely accepted
  # convention for where to install development libraries on Windows, please
  # correct this.)
  include(GNUInstallDirs)

  #============================================================================
  #Find available package generators

  # DEB
  if("${CMAKE_SYSTEM}" MATCHES "Linux")
    find_program(DPKG_PROGRAM dpkg)
    if(EXISTS ${DPKG_PROGRAM})
      list(APPEND CPACK_GENERATOR "DEB")
    endif(EXISTS ${DPKG_PROGRAM})

    find_program(RPMBUILD_PROGRAM rpmbuild)
  endif()

  list(APPEND CPACK_SOURCE_GENERATOR "TBZ2")
  list(APPEND CPACK_SOURCE_GENERATOR "ZIP")
  list(APPEND CPACK_SOURCE_IGNORE_FILES "TODO;\.hg/;\.sw.$;/build/;\.hgtags;\.hgignore;appveyor\.yml;\.travis\.yml;codecov\.yml")

  include(InstallRequiredSystemLibraries)

  #execute_process(COMMAND dpkg --print-architecture _NPROCE)
  set(DEBIAN_PACKAGE_DEPENDS "")

  set(RPM_PACKAGE_DEPENDS "")

  set(PROJECT_CPACK_CFG_FILE "${PROJECT_BINARY_DIR}/cpack_options.cmake")

  #============================================================================
  # Set CPack variables
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_FULL}")
  set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

  if(CPACK_GENERATOR)
    message(STATUS "Found CPack generators: ${CPACK_GENERATOR}")

    configure_file("${GZ_CMAKE_DIR}/cpack_options.cmake.in"
      ${PROJECT_CPACK_CFG_FILE} @ONLY)

    set(CPACK_PROJECT_CONFIG_FILE ${PROJECT_CPACK_CFG_FILE})
    include(CPack)
  endif()

  #============================================================================
  # If we're configuring only to package source, stop here
  if(PACKAGE_SOURCE_ONLY)
    message(WARNING "Configuration was done in PACKAGE_SOURCE_ONLY mode."
    "You can build a tarball (make package_source), but nothing else.")
    return()
  endif()

  #============================================================================
  # Developer's option to cache PKG_CONFIG_PATH and
  # LD_LIBRARY_PATH for local installs
  if(PKG_CONFIG_PATH)
    set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH})
  endif()

  if(LD_LIBRARY_PATH)
    set(ENV{LD_LIBRARY_PATH} ${LD_LIBRARY_PATH}:$ENV{LD_LIBRARY_PATH})
  endif()

  #============================================================================
  # Set up installation directories
  set(GZ_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
  set(GZ_INCLUDE_INSTALL_DIR_POSTFIX "gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR}")
  set(GZ_INCLUDE_INSTALL_DIR_FULL    "${GZ_INCLUDE_INSTALL_DIR}/${GZ_INCLUDE_INSTALL_DIR_POSTFIX}")
  set(GZ_DATA_INSTALL_DIR_POSTFIX "gz/${PROJECT_NAME_LOWER}")
  set(GZ_DATA_INSTALL_DIR         "${CMAKE_INSTALL_DATAROOTDIR}/${GZ_DATA_INSTALL_DIR_POSTFIX}")
  set(GZ_LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
  set(GZ_BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})

  #============================================================================
  # Handle the user's RPATH setting
  option(USE_FULL_RPATH "Turn on to enable the full RPATH" OFF)
  if(USE_FULL_RPATH)
    # use, i.e. don't skip the full RPATH for the build tree
    set(CMAKE_SKIP_BUILD_RPATH FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # the RPATH to be used when installing, but only if its not a system directory
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
    if("${isSystemDir}" STREQUAL "-1")
      set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    endif("${isSystemDir}" STREQUAL "-1")
  endif()

  #============================================================================
  # Add uninstall target
  configure_file(
    "${GZ_CMAKE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

endmacro()

macro(gz_create_packages)

  #============================================================================
  # Load platform-specific build hooks if present.
  _gz_load_build_hooks()

  #============================================================================
  # Tell the user what their settings are
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

endmacro()


#################################################
# _gz_create_pkgconfig([COMPONENT <component>])
#
# Provide the name of the target for which we will generate package config info.
# If the target is a component, pass in the COMPONENT argument followed by the
# component's name.
#
# NOTE: This will be called automatically by gz_create_core_library(~) and
#       gz_add_component(~), so users of gz-cmake should not call this
#       function.
#
# NOTE: For gz-cmake developers, the variables needed by gz.pc.in or
#       gz-component.pc.in MUST be set before calling this function.
#
# Create a pkgconfig file for your target, and install it.
function(_gz_create_pkgconfig)

  #------------------------------------
  # Define the expected arguments
  set(options)
  set(oneValueArgs COMPONENT) # Unused
  set(multiValueArgs) # Unused

  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(_gz_create_pkgconfig "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #------------------------------------
  # Choose which input file to use
  if(_gz_create_pkgconfig_COMPONENT)
    set(pkgconfig_input "${GZ_CMAKE_DIR}/pkgconfig/gz-component.pc.in")
    set(target_name ${PROJECT_LIBRARY_TARGET_NAME}-${_gz_create_pkgconfig_COMPONENT})
  else()
    set(pkgconfig_input "${GZ_CMAKE_DIR}/pkgconfig/gz.pc.in")
    set(target_name ${PROJECT_LIBRARY_TARGET_NAME})
  endif()

  set(pkgconfig_output "${CMAKE_BINARY_DIR}/cmake/pkgconfig/${target_name}.pc")
  set(pkgconfig_install_dir "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
  set(pkgconfig_abs_install_dir "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
  file(RELATIVE_PATH
    PC_CONFIG_RELATIVE_PATH_TO_PREFIX
    "${pkgconfig_abs_install_dir}"
    "${CMAKE_INSTALL_PREFIX}"
  )

  include("${GZ_CMAKE_DIR}/JoinPaths.cmake")
  join_paths(PC_LIBDIR "\${prefix}" "${CMAKE_INSTALL_LIBDIR}")
  join_paths(PC_INCLUDEDIR "\${prefix}" "${CMAKE_INSTALL_INCLUDEDIR}" "${GZ_INCLUDE_INSTALL_DIR_POSTFIX}")

  configure_file(${pkgconfig_input} ${pkgconfig_output} @ONLY)

  install(
    FILES ${pkgconfig_output}
    DESTINATION ${pkgconfig_install_dir}
    COMPONENT pkgconfig)

endfunction()


#################################################
# _gz_create_cmake_package([COMPONENT <component>]
#                           [LEGACY_PROJECT_PREFIX <prefix>])
#
# Provide the name of the target that will be installed and exported. If the
# target is a component, pass in the COMPONENT argument followed by the
# component's name.
#
# For packages like sdformat that use inconsistent case in the legacy cmake
# variable names (like SDFormat_LIBRARIES), the LEGACY_PROJECT_PREFIX argument
# can be used to specify the prefix of these variables.
#
# NOTE: This will be called automatically by gz_create_core_library(~) and
#       gz_add_component(~), so users of gz-cmake should not call this
#       function.
#
# NOTE: For gz-cmake developers, some of the variables needed by
#       gz-config.cmake.in or gz-component-config.cmake.in MUST be
#       set before calling this function. The following variables are set
#       automatically by this function:
#       - import_target_name
#       - target_output_filename
#
# Make the cmake config files for this target
function(_gz_create_cmake_package)

  #------------------------------------
  # Define the expected arguments
  set(options ALL)
  set(oneValueArgs COMPONENT LEGACY_PROJECT_PREFIX)
  set(multiValueArgs) # Unused

  #------------------------------------
  # Parse the arguments
  cmake_parse_arguments(_gz_create_cmake_package "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(_gz_create_cmake_package_COMPONENT AND _gz_create_cmake_package_ALL)
    message(FATAL_ERROR
      "_gz_create_cmake_package was called with both ALL and COMPONENT "
      "specified. This is not allowed!")
  endif()

  #------------------------------------
  # Set configuration arguments
  if(_gz_create_cmake_package_COMPONENT)

    set(component ${_gz_create_cmake_package_COMPONENT})
    set(target_name ${PROJECT_LIBRARY_TARGET_NAME}-${component})
    set(gz_config_input "${GZ_CMAKE_DIR}/gz-component-config.cmake.in")
    set(simple_import_name ${component})

  elseif(_gz_create_cmake_package_ALL)

    set(gz_config_input "${GZ_CMAKE_DIR}/gz-all-config.cmake.in")
    set(target_name ${PROJECT_LIBRARY_TARGET_NAME}-all)
    set(all_pkg_name ${PROJECT_LIBRARY_TARGET_NAME}-all)
    set(simple_import_name all)

  else()

    set(target_name ${PROJECT_LIBRARY_TARGET_NAME})
    set(gz_config_input "${GZ_CMAKE_DIR}/gz-config.cmake.in")
    set(simple_import_name core)

  endif()

  if(_gz_create_cmake_package_LEGACY_PROJECT_PREFIX)

    set(LEGACY_PROJECT_PREFIX ${_gz_create_cmake_package_LEGACY_PROJECT_PREFIX})

  else()

    set(LEGACY_PROJECT_PREFIX ${PROJECT_NAME_NO_VERSION_UPPER})

  endif()

  # This gets used by the gz-*.config.cmake.in files
  set(target_output_filename ${target_name}-targets.cmake)
  set(gz_config_output "${PROJECT_BINARY_DIR}/cmake/${target_name}-config.cmake")
  set(gz_version_output "${PROJECT_BINARY_DIR}/cmake/${target_name}-config-version.cmake")
  set(gz_target_ouput "${PROJECT_BINARY_DIR}/cmake/${target_output_filename}")

  # NOTE: Each component needs to go into its own cmake directory in order to be
  # found by cmake's native find_package(~) command.
  set(gz_config_install_dir "${GZ_LIB_INSTALL_DIR}/cmake/${target_name}")
  set(gz_namespace ${PROJECT_LIBRARY_TARGET_NAME}::)

  set(import_target_name ${gz_namespace}${target_name})
  set(simple_import_name ${gz_namespace}${simple_import_name})

  # Configure the package config file. It will be installed to
  # "[lib]/cmake/gz-<project><major_version>/" where [lib] is the library
  # installation directory.
  configure_package_config_file(
    ${gz_config_input}
    ${gz_config_output}
    INSTALL_DESTINATION ${gz_config_install_dir}
    PATH_VARS GZ_LIB_INSTALL_DIR GZ_INCLUDE_INSTALL_DIR_FULL)

  # Use write_basic_package_version_file to generate a ConfigVersion file that
  # allow users of the library to specify the API or version to depend on
  write_basic_package_version_file(
    ${gz_version_output}
    VERSION "${PROJECT_VERSION_FULL_NO_SUFFIX}"
    COMPATIBILITY SameMajorVersion)

  # Install the configuration files to the configuration installation directory
  install(
    FILES
      ${gz_config_output}
      ${gz_version_output}
    DESTINATION ${gz_config_install_dir}
    COMPONENT cmake)

  # Create *-targets.cmake file for build directory
  export(
    EXPORT ${target_name}
    FILE ${gz_target_ouput}
    # We add a namespace that ends with a :: to the name of the exported target.
    # This is so consumers of the project can call
    #     find_package(gz-<project>)
    #     target_link_libraries(consumer_project gz-<project>::gz-<project>)
    # and cmake will understand that the consumer is asking to link the imported
    # target "gz-<project>" to their "consumer_project" rather than asking
    # to link a library named "gz-<project>". In other words, when
    # target_link_libraries is given a name that contains double-colons (::) it
    # will never mistake it for a library name, and it will throw an error if
    # it cannot find a target with the given name.
    #
    # The advantage of linking against a target rather than a library is that
    # you will automatically link against all the dependencies of that target.
    # This also helps us create find-config files that are relocatable.
    NAMESPACE ${gz_namespace})

  # Install *-targets.cmake file
  install(
    EXPORT ${target_name}
    DESTINATION ${gz_config_install_dir}
    FILE ${target_output_filename}
    # See explanation above for NAMESPACE
    NAMESPACE ${gz_namespace})

endfunction()

#################################################
# Make the cmake config files for this project
# Pass an argument to specify the directory where the CMakeLists.txt for the
#   build hooks is located. If no argument is provided, we default to:
#   ${PROJECT_SOURCE_DIR}/packager-hooks
#
# This function is private to gz-cmake.
function(_gz_load_build_hooks)

  if(ARGV0)
    set(hook_dir ${ARGV0})
  else()
    set(hook_dir "${PROJECT_SOURCE_DIR}/cmake/packager-hooks")
  endif()

  if(EXISTS ${hook_dir}/CMakeLists.txt)
    message(STATUS "Loading packager build hooks from ${hook_dir}")
    add_subdirectory(${hook_dir})
  endif()

endfunction()
