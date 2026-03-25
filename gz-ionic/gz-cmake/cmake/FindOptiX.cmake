#===============================================================================
# Copyright (C) 2018 Open Source Robotics Foundation
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
########################################

# Modified from NVIDIA's FindOptiX.cmake distributed in its OptiX SDK
# Usage of this module as follows:
#
#     gz_find_package(OptiX)
#
# Variable used by this module, which needs to be set before calling
# find_package:
#
#  OPTIX_INSTALL_DIR         Set this variable to the root installation of
#                            OptiX SDK
#
# Variables defined by this module:
#
#  optix::optix              Imported target for optix
#  optix::optixu             Imported target for optixu
#  optix::optix_prime        Imported target for optix_prime
#
#  OptiX_FOUND               System has OptiX libs/headers
#  OptiX_LIBRARIES           The OptiX libraries
#  OptiX_INCLUDE_DIRS        The location of OptiX headers
#

########################################
#
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Locate the OptiX distribution.  Search in env path first, then look in the system.

set(OptiX_INSTALL_DIR "$ENV{OPTIX_INSTALL_DIR}" CACHE PATH "Path to OptiX installed location.")

# The distribution contains both 32 and 64 bit libraries.  Adjust the library
# search path based on the bit-ness of the build.  (i.e. 64: bin64, lib64; 32:
# bin, lib).  Note that on Mac, the OptiX library is a universal binary, so we
# only need to look in lib and not lib64 for 64 bit builds.
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT APPLE)
  set(bit_dest "64")
else()
  set(bit_dest "")
endif()

# gz-cmake modification: added "gz_" prefix to macro name
macro(gz_OPTIX_find_api_library name version)
  find_library(${name}_LIBRARY
    NAMES ${name}.${version} ${name}
    PATHS "${OptiX_INSTALL_DIR}/lib${bit_dest}"
    NO_DEFAULT_PATH
    )
  find_library(${name}_LIBRARY
    NAMES ${name}.${version} ${name}
    )
  if(WIN32)
    find_file(${name}_DLL
      NAMES ${name}.${version}.dll
      PATHS "${OptiX_INSTALL_DIR}/bin${bit_dest}"
      NO_DEFAULT_PATH
      )
    find_file(${name}_DLL
      NAMES ${name}.${version}.dll
      )
  endif()
endmacro()

gz_OPTIX_find_api_library(optix 1)
gz_OPTIX_find_api_library(optixu 1)
gz_OPTIX_find_api_library(optix_prime 1)

# Include
find_path(OptiX_INCLUDE
  NAMES optix.h
  PATHS "${OptiX_INSTALL_DIR}/include"
  NO_DEFAULT_PATH
  )
find_path(OptiX_INCLUDE
  NAMES optix.h
  )

# Check to make sure we found what we were looking for
function(OptiX_report_error error_message required)
  if(OptiX_FIND_REQUIRED AND required)
    message(FATAL_ERROR "${error_message}")
  else()
    if(NOT OptiX_FIND_QUIETLY)
      message(STATUS "${error_message}")
    endif(NOT OptiX_FIND_QUIETLY)
  endif()
endfunction()

if(NOT optix_LIBRARY)
  OptiX_report_error("optix library not found.  Please locate before proceeding." TRUE)
endif()
if(NOT OptiX_INCLUDE)
  OptiX_report_error("OptiX headers (optix.h and friends) not found.  Please locate before proceeding." TRUE)
endif()
if(NOT optix_prime_LIBRARY)
  OptiX_report_error("optix Prime library not found.  Please locate before proceeding." FALSE)
endif()

# Macro for setting up dummy targets
function(OptiX_add_imported_library name lib_location dll_lib dependent_libs)
  set(CMAKE_IMPORT_FILE_VERSION 1)

  # Create imported target
  # gz-cmake modification: changed to use ${target_name} instead of ${name}
  set(target_name optix::${name})
  add_library(${target_name} SHARED IMPORTED)

  # Import target "optix" for configuration "Debug"
  if(WIN32)
    set_target_properties(${target_name} PROPERTIES
      IMPORTED_IMPLIB "${lib_location}"
      #IMPORTED_LINK_INTERFACE_LIBRARIES "glu32;opengl32"
      IMPORTED_LOCATION "${dll_lib}"
      IMPORTED_LINK_INTERFACE_LIBRARIES "${dependent_libs}"
      )
  elseif(UNIX)
    set_target_properties(${target_name} PROPERTIES
      #IMPORTED_LINK_INTERFACE_LIBRARIES "glu32;opengl32"
      IMPORTED_LOCATION "${lib_location}"
      # We don't have versioned filenames for now, and it may not even matter.
      #IMPORTED_SONAME "${optix_soname}"
      IMPORTED_LINK_INTERFACE_LIBRARIES "${dependent_libs}"
      )
  else()
    # Unknown system, but at least try and provide the minimum required
    # information.
    set_target_properties(${target_name} PROPERTIES
      IMPORTED_LOCATION "${lib_location}"
      IMPORTED_LINK_INTERFACE_LIBRARIES "${dependent_libs}"
      )
  endif()

  # Commands beyond this point should not need to know the version.
  set(CMAKE_IMPORT_FILE_VERSION)
endfunction()

# Sets up a dummy target
OptiX_add_imported_library(optix "${optix_LIBRARY}" "${optix_DLL}" "${OPENGL_LIBRARIES}")
OptiX_add_imported_library(optixu   "${optixu_LIBRARY}"   "${optixu_DLL}"   "")
OptiX_add_imported_library(optix_prime "${optix_prime_LIBRARY}"  "${optix_prime_DLL}"  "")

# gz-cmake modification: added "gz_" prefix to macro name
macro(gz_OptiX_check_same_path libA libB)
  if(_optix_path_to_${libA})
    if(NOT _optix_path_to_${libA} STREQUAL _optix_path_to_${libB})
      # ${libA} and ${libB} are in different paths.  Make sure there isn't a ${libA} next
      # to the ${libB}.
      get_filename_component(_optix_name_of_${libA} "${${libA}_LIBRARY}" NAME)
      if(EXISTS "${_optix_path_to_${libB}}/${_optix_name_of_${libA}}")
        message(WARNING " ${libA} library found next to ${libB} library that is not being used.  Due to the way we are using rpath, the copy of ${libA} next to ${libB} will be used during loading instead of the one you intended.  Consider putting the libraries in the same directory or moving ${_optix_path_to_${libB}}/${_optix_name_of_${libA} out of the way.")
      endif()
    endif()
    set( _${libA}_rpath "-Wl,-rpath,${_optix_path_to_${libA}}" )
  endif()
endmacro()

# Since liboptix.1.dylib is built with an install name of @rpath, we need to
# compile our samples with the rpath set to where optix exists.
if(APPLE)
  get_filename_component(_optix_path_to_optix "${optix_LIBRARY}" PATH)
  if(_optix_path_to_optix)
    set( _optix_rpath "-Wl,-rpath,${_optix_path_to_optix}" )
  endif()
  get_filename_component(_optix_path_to_optixu "${optixu_LIBRARY}" PATH)
  gz_OptiX_check_same_path(optixu optix)
  get_filename_component(_optix_path_to_optix_prime "${optix_prime_LIBRARY}" PATH)
  gz_OptiX_check_same_path(optix_prime optix)
  gz_OptiX_check_same_path(optix_prime optixu)

  set( optix_rpath ${_optix_rpath} ${_optixu_rpath} ${_optix_prime_rpath} )
  list(LENGTH optix_rpath optix_rpath_LENGTH)
  if (optix_rpath_LENGTH GREATER 0)
    list(REMOVE_DUPLICATES optix_rpath)
  endif()
endif()

########################################

if (OptiX_INCLUDE AND optix_LIBRARY)
  set (OptiX_FOUND TRUE)
else()
  set (OptiX_FOUND FALSE)
endif()

if (OptiX_FOUND)
  set(OptiX_INCLUDE_DIRS ${OptiX_INCLUDE})
  set(OptiX_LIBRARIES ${optix_LIBRARY} ${optixu_LIBRARY} ${optix_prime_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OptiX
  REQUIRED_VARS OptiX_FOUND)

include(GzPkgConfig)
gz_pkg_config_library_entry(OptiX OptiX)

