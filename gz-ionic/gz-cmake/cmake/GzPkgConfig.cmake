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
# An alternative to pkg_check_modules that creates an "imported target" which
# helps us to make relocatable packages.
#
# NOTE: This macro assumes that pkg-config is the only means by which you will
#       be searching for the package. If you intend to continue searching in the
#       event that pkg-config fails (or is unavailable), then you should instead
#       call gz_pkg_check_modules_quiet(~).
#
# NOTE: If you need to specify a version comparison for pkg-config, then your
#       second argument must be wrapped in quotes. E.g. if you want to find
#       version greater than or equal to 3.2.1 of a package called SomePackage
#       which is known to pkg-config as libsomepackage, then you should call
#       gz_pkg_check_modules as follows:
#
#       gz_pkg_check_modules(SomePackage "libsomepackage >= 3.2.1")
#
#       The quotes and spaces in the second argument are all very important in
#       order to ensure that our auto-generated *.pc file gets filled in
#       correctly. If you do not have any version requirements, then you can
#       simply leave all of that out:
#
#       gz_pkg_check_modules(SomePackage libsomepackage)
#
#       Without the version comparison, the quotes and spacing are irrelevant.
#       This usage note applies to gz_pkg_check_modules_quiet(~) as well.
#
macro(gz_pkg_check_modules package signature)

  gz_pkg_check_modules_quiet(${package} "${signature}" ${ARGN})

  if(NOT PKG_CONFIG_FOUND)
    message(WARNING "The package [${package}] requires pkg-config in order to be found. "
                   "Please install pkg-config so we can search for that package.")
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    ${package}
    REQUIRED_VARS ${package}_FOUND)

endmacro()

# This is an alternative to gz_pkg_check_modules(~) which you can use if you
# have an alternative way to look for the package if pkg-config is not available
# or cannot find the requested package. This will still setup the pkg-config
# variables for you, whether or not pkg-config is available.
#
# For usage instructions, see gz_pkg_check_modules(~) above.
macro(gz_pkg_check_modules_quiet package signature)
  # Define the expected arguments
  set(options INTERFACE NO_CMAKE_ENVIRONMENT_PATH QUIET)
  set(oneValueArgs "TARGET_NAME")
  set(multiValueArgs)
  #------------------------------------
  # Parse the arguments
  _gz_cmake_parse_arguments(gz_pkg_check_modules "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(gz_pkg_check_modules_INTERFACE)
    set(_gz_pkg_check_modules_interface_option INTERFACE)
  else()
    set(_gz_pkg_check_modules_interface_option) # Intentionally blank
  endif()

  if(NOT gz_pkg_check_modules_TARGET_NAME)
    set(gz_pkg_check_modules_TARGET_NAME "${package}::${package}")
  endif()

  find_package(PkgConfig QUIET)

  gz_pkg_config_entry(${package} "${signature}")

  if(PKG_CONFIG_FOUND)

    if(${gz_pkg_check_modules_NO_CMAKE_ENVIRONMENT_PATH})
      set(gz_pkg_check_modules_no_cmake_environment_path_arg
          NO_CMAKE_ENVIRONMENT_PATH)
    else()
      set(gz_pkg_check_modules_no_cmake_environment_path_arg)
    endif()

    if(${gz_pkg_check_modules_QUIET} OR ${package}_FIND_QUIETLY)
      set(gz_pkg_check_modules_quiet_arg QUIET)
    else()
      set(gz_pkg_check_modules_quiet_arg)
    endif()

    # unset LIBRARIES variable in case it contains full paths already
    # due to previous calls of this function searching for the same package.
    unset(${package}_LIBRARIES)

    pkg_check_modules(${package}
                      ${gz_pkg_check_modules_quiet_arg}
                      ${gz_pkg_check_modules_no_cmake_environment_path_arg}
                      ${signature})

    # TODO: When we require cmake-3.6+, we should remove this procedure and just
    #       use the plain pkg_check_modules, which provides an option called
    #       IMPORTED_TARGET that will create the imported targets the way we do
    #       here.
    if(${package}_FOUND)

      # Because of some idiosyncrasies of pkg-config, pkg_check_modules does not
      # put /usr/include in the <prefix>_INCLUDE_DIRS variable. E.g. try running
      # $ pkg-config --cflags-only-I tinyxml2
      # and you'll find that it comes out blank. This blank value gets cached
      # into the <prefix>_INCLUDE_DIRS variable even though it's a bad value. If
      # other packages then try to call find_path(<prefix>_INCLUDE_DIRS ...) in
      # their own find-module or config-files, the find_path will quit early
      # because a CACHE entry exists for <prefix>_INCLUDE_DIRS. However, that
      # CACHE entry is blank, and so it will typically be interpreted as a
      # failed attempt to find the path. So if this <prefix>_INCLUDE_DIRS
      # variable is blank, then we'll unset it from the CACHE to avoid
      # conflicts and confusion.
      #
      # TODO(MXG): Consider giving a different prefix (e.g. GZ_PC_${package})
      # to pkg_check_modules(~) so that the cached variables don't collide. That
      # would also help with the next TODO below.
      if(NOT ${package}_INCLUDE_DIRS)
        unset(${package}_INCLUDE_DIRS CACHE)
      endif()

      # pkg_check_modules will put ${package}_FOUND into the CACHE, which would
      # prevent our FindXXX.cmake script from being entered the next time cmake
      # is run by a dependent project. This is a problem for us because we
      # produce an imported target which gets wiped out and needs to recreated
      # between runs.
      #
      # TODO: Investigate if there is a more conventional solution to this
      # problem. Perhaps the cmake-3.6 version of pkg_check_modules has a
      # better solution.
      unset(${package}_FOUND CACHE)
      set(${package}_FOUND TRUE)

      # For some reason, pkg_check_modules does not provide complete paths to the
      # libraries it returns, even though find_package is conventionally supposed
      # to provide complete library paths. Having only the library name is harmful
      # to the gz_create_imported_target macro, so we will change the variable to
      # give it complete paths.
      #
      # TODO: How would we deal with multiple modules that are in different
      # directories? How does cmake-3.6+ handle that situation?
      _gz_pkgconfig_find_libraries(
        ${package}_LIBRARIES
        ${package}
        "${${package}_LIBRARIES}"
        "${${package}_LIBRARY_DIRS}")

      include(GzImportTarget)
      gz_import_target(${package} ${_gz_pkg_check_modules_interface_option}
        TARGET_NAME ${gz_pkg_check_modules_TARGET_NAME})

    endif()

  endif()

endmacro()

# This creates variables which inform gz_find_package(~) that your package
# should be found as a module by pkg-config. In most cases, this will be called
# implicitly by gz_pkg_check_modules[_quiet], but if a package provides both a
# cmake config-file (*-config.cmake) and a pkg-config file (*.pc), then you can
# use the cmake config-file to retrieve the package information, and then use
# this macro to generate the relevant pkg-config information.
macro(gz_pkg_config_entry package string)
  # The input string may contain an operator without a version,
  # e.g "protobuf >= ". But this is not valid pkg-config syntax. This regex
  # search/replace will remove the operator if the version is empty.
  string(REGEX REPLACE " *[<>=]+ *$" "" entry ${string})
  set(${package}_PKGCONFIG_ENTRY "${entry}")
  set(${package}_PKGCONFIG_TYPE PKGCONFIG_REQUIRES)

endmacro()

# This creates variables which inform gz_find_package(~) that your package must
# be found as a plain library by pkg-config. This should be used in any
# find-module that handles a library package which does not install a pkg-config
# <package>.pc file.
macro(gz_pkg_config_library_entry package lib_name)

  set(${package}_PKGCONFIG_ENTRY "-l${lib_name}")
  set(${package}_PKGCONFIG_TYPE PKGCONFIG_LIBS)

endmacro()

# Based on discussion here: https://cmake.org/Bug/view.php?id=15804
# and a patch written by Sam Thursfield
function(_gz_pkgconfig_find_libraries output_var package library_names library_dirs)

  foreach(libname ${library_names})

    # As recommended in cmake's find_library documenation, we can call
    # find_library multiple times with the NO_* option to override search order.
    # Give priority to path specified by user by telling cmake not to look
    # in default paths. If the first call succeeds, the second call will not
    # search again
    find_library(
      ${package}_LIBRARY_${libname}
      ${libname}
      PATHS ${library_dirs} NO_DEFAULT_PATH)

    find_library(
      ${package}_LIBRARY_${libname}
      ${libname}
      PATHS ${library_dirs})
    mark_as_advanced(${package}_LIBRARY_${libname})

    list(APPEND library_paths "${${package}_LIBRARY_${libname}}")

  endforeach()

  set(${output_var} ${library_paths} PARENT_SCOPE)

endfunction()
