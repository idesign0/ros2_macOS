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
# gz_add_get_install_prefix_impl(GET_INSTALL_PREFIX_FUNCTION <get_install_prefix_function>
#                                GET_INSTALL_PREFIX_HEADER <get_install_prefix_function>
#                                OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE <override_install_prefix_env_variable>)
#
# This macro adds to ${PROJECT_LIBRARY_TARGET_NAME} the implementation of
# of the function passed by GET_INSTALL_PREFIX_FUNCTION and declared in
# GET_INSTALL_PREFIX_HEADER .
#
# The defined functions implements a GET_INSTALL_PREFIX_FUNCTION that returns
# the installation directory of the package (CMAKE_INSTALL_PREFIX at build time)
# as following:
# * if the library is shared and GZ_ENABLE_RELOCATABLE_INSTALL is ON, by extracting the
#   location of the shared library via dladdr, and computing the corresponding
#   install prefix from it
# * if the library is static or GZ_ENABLE_RELOCATABLE_INSTALL is OFF, by using the exact
#   value of CMAKE_INSTALL_PREFIX that was hardcoded in the library at compilation time
#
# As in some cases it is important to have the ability to control and change the value returned by
# the GET_INSTALL_PREFIX_FUNCTION at runtime, in both cases the library returns the value of
# the OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE if the environment variable is defined
#
# To use this macro, please add gz_find_package(DL)
# in the dependencies of your project
#
# Arguments are as follows:
#
# GET_INSTALL_PREFIX_FUNCTION: Required. The name (with namespace) of the function that returns
#          the install directory of the package. Example: gz::${GZ_DESIGNATION}::getInstallPrefix
#
# GET_INSTALL_PREFIX_HEADER: Required. The name (with full relative path) of the include that contains
#          the declaration of the ${GET_INSTALL_PREFIX_FUNCTION} function.
#          Example: gz/${GZ_DESIGNATION}/InstallationDirectories.hh
#
# OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE: Required. The name of the environmental variable that can be
#          used to override the value returned by ${GET_INSTALL_PREFIX_FUNCTION} at runtime.
#          Example: GZ_${GZ_DESIGNATION_UPPER}_INSTALL_PREFIX

macro(gz_add_get_install_prefix_impl)
  set(_options)
  set(_oneValueArgs
    GET_INSTALL_PREFIX_FUNCTION
    GET_INSTALL_PREFIX_HEADER
    OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE
  )
  set(_multiValueArgs )
  cmake_parse_arguments(gz_add_get_install_prefix_impl "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

  if(NOT DEFINED gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_FUNCTION)
    message(FATAL_ERROR
      "gz_add_get_install_prefix_impl: missing parameter GET_INSTALL_PREFIX_FUNCTION")
  endif()

  if(NOT DEFINED gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_HEADER)
    message(FATAL_ERROR
      "gz_add_get_install_prefix_impl: missing parameter GET_INSTALL_PREFIX_HEADER")
  endif()

  if(NOT DEFINED gz_add_get_install_prefix_impl_OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE)
    message(FATAL_ERROR
      "gz_add_get_install_prefix_impl: missing parameter OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE")
  endif()

  if(NOT TARGET ${PROJECT_LIBRARY_TARGET_NAME})
    message(FATAL_ERROR
      "Target ${PROJECT_LIBRARY_TARGET_NAME} required by gz_add_get_install_prefix_impl\n"
      "does not exist.")
  endif()

  get_target_property(target_type ${PROJECT_LIBRARY_TARGET_NAME} TYPE)
  if(NOT (target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY"))
    message(FATAL_ERROR "gz_add_get_install_prefix_impl: library ${_library} is of unsupported type ${target_type}")
  endif()


  set(gz_add_get_install_prefix_impl_GENERATED_CPP
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_LIBRARY_TARGET_NAME}_get_install_prefix_impl.cc)

  # Write cpp for shared or module library type
  option(GZ_ENABLE_RELOCATABLE_INSTALL "If ON, enable the feature of providing a relocatable install prefix in shared library." OFF)

  if ((target_type STREQUAL "MODULE_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY") AND GZ_ENABLE_RELOCATABLE_INSTALL)
    if(NOT TARGET ${DL_TARGET})
      message(FATAL_ERROR
        "gz_add_get_install_prefix_impl called without DL_TARGET defined,\n"
        "please add gz_find_package(DL) if you want to use gz_add_get_install_prefix_impl.")
    endif()

    # We can't query the LOCATION property of the target due to https://cmake.org/cmake/help/v3.25/policy/CMP0026.html
    # We can only access the directory of the library at generation time via $<TARGET_FILE_DIR:tgt>
    file(GENERATE OUTPUT "${gz_add_get_install_prefix_impl_GENERATED_CPP}"
         CONTENT
"// This file is automatically generated by the gz_add_get_install_prefix_impl CMake macro.

#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

#include <dlfcn.h>

#include <${gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_HEADER}>

#ifdef _MSC_VER
  // Disable warnings related to the use of std::getenv
  // See https://stackoverflow.com/questions/66090389/c17-what-new-with-error-c4996-getenv-this-function-or-variable-may-be-un
  #pragma warning(disable: 4996)
#endif

std::string ${gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_FUNCTION}()
{
  if(const char* override_env_var = std::getenv(\"${gz_add_get_install_prefix_impl_OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE}\"))
  {
    return std::string(override_env_var);
  }

  std::error_code fs_error;
  std::filesystem::path library_location;

  // Get location of the library
  Dl_info address_info;
  int res_val = dladdr(reinterpret_cast<void *>(&${gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_FUNCTION}), &address_info);
  if (address_info.dli_fname && res_val > 0)
  {
    library_location = address_info.dli_fname;
  }
  else
  {
    return \"${CMAKE_INSTALL_PREFIX}\";
  }

  const std::filesystem::path library_directory = library_location.parent_path();
  // Given the library_directory, return the install prefix via the relative path
#ifndef _WIN32
  const std::filesystem::path rel_path_from_install_prefix_to_lib = std::string(\"${CMAKE_INSTALL_LIBDIR}\");
#else
  const std::filesystem::path rel_path_from_install_prefix_to_lib = std::string(\"${CMAKE_INSTALL_BINDIR}\");
#endif
  const std::filesystem::path rel_path_from_lib_to_install_prefix =
    std::filesystem::relative(std::filesystem::current_path(), std::filesystem::current_path() / rel_path_from_install_prefix_to_lib, fs_error);

  if (fs_error)
  {
    return \"${CMAKE_INSTALL_PREFIX}\";
  }

  const std::filesystem::path install_prefix = library_directory / rel_path_from_lib_to_install_prefix;
  const std::filesystem::path install_prefix_canonical = std::filesystem::canonical(install_prefix, fs_error);

  if (fs_error)
  {
    return \"${CMAKE_INSTALL_PREFIX}\";
  }

  // Return install prefix
  return install_prefix_canonical.string();
}

#ifdef _MSC_VER
  #pragma warning(pop)
#endif
")
  else()
    # For static library, fallback to just provide return CMAKE_INSTALL_PREFIX
    file(GENERATE OUTPUT "${gz_add_get_install_prefix_impl_GENERATED_CPP}"
         CONTENT
"// This file is automatically generated by the gz_add_get_install_prefix_impl CMake macro.
#include <string>

#include <${gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_HEADER}>

#ifdef _MSC_VER
  #pragma warning(push)
  // Disable warnings related to the use of std::getenv
  // See https://stackoverflow.com/questions/66090389/c17-what-new-with-error-c4996-getenv-this-function-or-variable-may-be-un
  #pragma warning(disable: 4996)
#endif

std::string ${gz_add_get_install_prefix_impl_GET_INSTALL_PREFIX_FUNCTION}()
{
  if(const char* override_env_var = std::getenv(\"${gz_add_get_install_prefix_impl_OVERRIDE_INSTALL_PREFIX_ENV_VARIABLE}\"))
  {
    return std::string(override_env_var);
  }

  return \"${CMAKE_INSTALL_PREFIX}\";
}

#ifdef _MSC_VER
  #pragma warning(pop)
#endif
")
endif()

  # Add cpp to library
  target_sources(${PROJECT_LIBRARY_TARGET_NAME} PRIVATE ${gz_add_get_install_prefix_impl_GENERATED_CPP})

# Only link DL in the case that it is needed.
if ((target_type STREQUAL "MODULE_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY") AND GZ_ENABLE_RELOCATABLE_INSTALL)
  # Link DL_TARGET that provides dladdr
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME} PRIVATE ${DL_TARGET})
endif()

endmacro()
