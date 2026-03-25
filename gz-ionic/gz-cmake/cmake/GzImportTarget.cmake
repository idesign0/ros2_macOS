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
#
########################################
# gz_import_target(<package> [INTERFACE]
#     [TARGET_NAME <target_name>]
#     [LIB_VAR <library_variable>]
#     [INCLUDE_VAR <header_directory_variable>]
#     [CFLAGS_VAR <cflags_varaible>])
#
# This macro will create an imported target based on the variables pertaining
# to <package>, such as <package>_LIBRARIES, <package>_INCLUDE_DIRS, and
# <package>_CFLAGS. Optionally, you can provide TARGET_NAME followed by a
# string, which will then be used as the name of the imported target. If
# TARGET_NAME is not provided, the name of the imported target will default to
# <package>::<package>.
#
# INTERFACE: Optional. Use INTERFACE when the target does not actually provide
#            a library that needs to be linked against (e.g. it is a header-only
#            library, or the target is just used to specify compiler flags).
#
# TARGET_NAME: Optional. Explicitly specify the desired imported target name.
#              Default is <package>::<package>.
#
# LIB_VAR: Optional. Explicitly specify the name of the library variable for
#          this package. Default is <package>_LIBRARIES.
#
# INCLUDE_VAR: Optional. Explicitly specify the name of the include directory
#              variable for this package. Default is <package>_INCLUDE_DIRS.
#
# CFLAGS_VAR: Optional. Explicitly specify the name of the cflags variable for
#             this package. Default is <package>_CFLAGS.
#
macro(gz_import_target package)

  if (NOT gz_import_target_skip_parsing)
    #------------------------------------
    # Define the expected arguments
    set(options "INTERFACE")
    set(oneValueArgs "TARGET_NAME" "LIB_VAR" "INCLUDE_VAR" "CFLAGS_VAR")
    set(multiValueArgs) # We are not using multiValueArgs yet

    #------------------------------------
    # Parse the arguments
    _gz_cmake_parse_arguments(gz_import_target "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  endif()

  #------------------------------------
  # Check if a target name has been provided, otherwise use
  # ${package}::{$package} as the target name.
  if(gz_import_target_TARGET_NAME)
    set(target_name ${gz_import_target_TARGET_NAME})
  else()
    set(target_name ${package}::${package})
  endif()

  if(NOT TARGET ${target_name})

    #------------------------------------
    # Use default versions of these build variables if custom versions were not
    # provided.
    if(NOT gz_import_target_LIB_VAR)
      set(gz_import_target_LIB_VAR ${package}_LIBRARIES)
    endif()

    if(NOT gz_import_target_INCLUDE_VAR)
      set(gz_import_target_INCLUDE_VAR ${package}_INCLUDE_DIRS)
    endif()

    if(NOT gz_import_target_CFLAGS_VAR)
      set(gz_import_target_CFLAGS_VAR ${package}_CFLAGS)
    endif()

    #------------------------------------
    # Link against this "imported" target by saying
    # target_link_libraries(mytarget package::package), instead of linking
    # against the variable package_LIBRARIES with the old-fashioned
    # target_link_libraries(mytarget ${package_LIBRARIES}
    if(NOT gz_import_target_INTERFACE)
      add_library(${target_name} UNKNOWN IMPORTED)
    else()
      add_library(${target_name} INTERFACE IMPORTED)
    endif()

    # Do not bother with the IMPORTED_LOCATION or IMPORTED_IMPLIB variables if it
    # is an INTERFACE target.
    if(NOT gz_import_target_INTERFACE)

      if(${gz_import_target_LIB_VAR})
        _gz_sort_libraries(${target_name} ${${gz_import_target_LIB_VAR}})
      endif()

    endif()

    if(${gz_import_target_LIB_VAR})
      set_target_properties(${target_name} PROPERTIES
        INTERFACE_LINK_LIBRARIES "${${gz_import_target_LIB_VAR}}")
    endif()

    if(${gz_import_target_INCLUDE_VAR})
      # TODO: In a later version of cmake, it should be possible to replace this
      # with
      #
      # target_include_directories(${target_name} INTERFACE ${${gz_import_target_INCLUDE_VAR}})
      #
      # But this will not be possible until we are using whichever version of cmake
      # the PR https://gitlab.kitware.com/cmake/cmake/merge_requests/1264
      # is available for.
      set_property(
        TARGET ${target_name}
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES
          ${${gz_import_target_INCLUDE_VAR}})
    endif()

    if(${gz_import_target_CFLAGS_VAR})
      # TODO: See note above. We should eventually be able to replace this with
      # target_compile_options(${target_name} INTERFACE ${${gz_import_target_CFLAGS_VAR}})
      set_property(
        TARGET ${target_name}
        PROPERTY INTERFACE_COMPILE_OPTIONS
          ${${gz_import_target_CFLAGS_VAR}})
    endif()

    # What about linker flags? Is there no target property for that?

  endif()

endmacro()

# This is an awkward hack to give the package both an IMPORTED_LOCATION and
# a set of INTERFACE_LIBRARIES in the event that PkgConfig returns multiple
# libraries for this package. It seems that IMPORTED_LOCATION cannot support
# specifying multiple libraries, so if we have multiple libraries, we need to
# pass them into INTERFACE_LINK_LIBRARIES. However, if IMPORTED_LOCATION is
# missing from the target, the dependencies do not get configured correctly by
# the generator expressions, and the build system will try to link to a nonsense
# garbage file.
#
# TODO: Figure out if there is a better way to fill in the various library
# properties of an imported target.
function(_gz_sort_libraries target_name first_lib)

  if(MSVC)
    # Note: For MSVC, we only care about the "import library" which is the
    # library ending in *.lib. The linker only needs to be told where the
    # *.lib library is. The dynamic library (*.dll) only needs to be visible
    # to the program at run-time, not at compile or link time. Furthermore,
    # find_library on Windows only looks for *.lib files, so we expect that
    # results of the form package_LIBRARIES will contain *.lib files when
    # running on Windows. IMPORTED_IMPLIB is the target property that
    # indicates the "import library" of an "imported target", so that is
    # the property that will fill in first and foremost.
    #
    # TODO: How does MinGW handle libraries?
    set_target_properties(${target_name} PROPERTIES
      IMPORTED_IMPLIB "${first_lib}")
  endif()
  set_target_properties(${target_name} PROPERTIES
    IMPORTED_LOCATION "${first_lib}")

  foreach(extra_lib ${ARGN})
    set_target_properties(${target_name} PROPERTIES
      INTERFACE_LINK_LIBRARIES "${extra_lib}")
  endforeach()

  get_target_property(ill ${target_name} INTERFACE_LINK_LIBRARIES)
  set_target_properties(${target_name}
    PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES ${ill})
  set_target_properties(${target_name}
    PROPERTIES IMPORTED_LINK_DEPENDENT_LIBRARIES ${ill})

endfunction()
