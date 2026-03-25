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

#--------------------------------------
# Find OGRE headers and libraries
#
# Usage of this module as follows:
#
#     gz_find_package(GzOGRE)
#
# Variables defined by this module:
#
#  OGRE_FOUND              System has OGRE libs/headers
#  OGRE_LIBRARIES          The OGRE libraries
#  OGRE_INCLUDE_DIRS       The location of OGRE headers
#  OGRE_VERSION            Full OGRE version in the form of MAJOR.MINOR.PATCH
#  OGRE_VERSION_MAJOR      OGRE major version
#  OGRE_VERSION_MINOR      OGRE minor version
#  OGRE_VERSION_PATCH      OGRE patch version
#  OGRE_RESOURCE_PATH      Path to ogre plugins directory
#  GzOGRE::GzOGRE        Imported target for OGRE
#
# On Windows, we assume that all the OGRE* defines are passed in manually
# to CMake.
#
# Supports finding the following OGRE components: RTShaderSystem, Terrain, Overlay
#
# Example usage:
#
#     gz_find_package(GzOGRE
#                      VERSION 1.8.0
#                      COMPONENTS RTShaderSystem Terrain Overlay)

# Grab the version numbers requested by the call to find_package(~)
set(major_version ${GzOGRE_FIND_VERSION_MAJOR})
set(minor_version ${GzOGRE_FIND_VERSION_MINOR})

# Set the full version number
set(full_version ${major_version}.${minor_version})

# Copied from OGREConfig.cmake
# Internal to gz-cmake.
macro(_gz_ogre_declare_plugin TYPE COMPONENT)
    set(OGRE_${TYPE}_${COMPONENT}_FOUND TRUE)
    set(OGRE_${TYPE}_${COMPONENT}_LIBRARIES ${TYPE}_${COMPONENT})
    list(APPEND OGRE_LIBRARIES ${TYPE}_${COMPONENT})
endmacro()

if (NOT WIN32)
  # pkg-config platforms
  set(PKG_CONFIG_PATH_ORIGINAL $ENV{PKG_CONFIG_PATH})
  set(PKG_CONFIG_PATH_TMP ${PKG_CONFIG_PATH_ORIGINAL})

  execute_process(COMMAND pkg-config --variable pc_path pkg-config
                  OUTPUT_VARIABLE _pkgconfig_invoke_result
                  RESULT_VARIABLE _pkgconfig_failed)
  if(_pkgconfig_failed)
    GZ_BUILD_WARNING ("Failed to get pkg-config search paths")
  elseif (NOT _pkgconfig_invoke_result STREQUAL "")
    set (PKG_CONFIG_PATH_TMP "${PKG_CONFIG_PATH_TMP}:${_pkgconfig_invoke_result}")
  endif()

  # check and see if there are any paths at all
  if ("${PKG_CONFIG_PATH_TMP}" STREQUAL "")
    message("No valid pkg-config search paths found")
    return()
  endif()

  string(REPLACE ":" ";" PKG_CONFIG_PATH_TMP ${PKG_CONFIG_PATH_TMP})

  # loop through pkg config paths and find an ogre version that is < 2.0.0
  foreach(pkg_path ${PKG_CONFIG_PATH_TMP})
    if (NOT EXISTS ${pkg_path})
      continue()
    endif()
    set(OGRE_FOUND false)
    set(OGRE_INCLUDE_DIRS "")
    set(OGRE_LIBRARY_DIRS "")
    set(OGRE_LIBRARIES "")
    set(ENV{PKG_CONFIG_PATH} ${pkg_path})
    gz_pkg_check_modules_quiet(OGRE "OGRE >= ${full_version}"
                                NO_CMAKE_ENVIRONMENT_PATH
                                QUIET)
    if (OGRE_FOUND)
      if (NOT ${OGRE_VERSION} VERSION_LESS 2.0.0)
        set (OGRE_FOUND false)
      else ()
        # set library dirs if the value is empty
        if (NOT OGRE_LIBRARY_DIRS)
          pkg_get_variable(OGRE_LIBRARY_DIRS OGRE libdir)
          if(NOT OGRE_LIBRARY_DIRS)
            GZ_BUILD_WARNING ("Failed to find OGRE's library directory.  The build will succeed, but there will likely be run-time errors.")
          else()
            # strip line break
            string(REGEX REPLACE "\n$" "" OGRE_LIBRARY_DIRS "${OGRE_LIBRARY_DIRS}")

            string(FIND "${OGRE_LIBRARIES}" "${OGRE_LIBRARY_DIRS}" substr_found)
            # in some cases the value of OGRE_LIBRARIES is "OgreMain;pthread"
            # Convert this to full path to library
            if (substr_found EQUAL -1)
              foreach(OGRE_LIBRARY_NAME ${OGRE_LIBRARIES})
                find_library(OGRE_LIBRARY NAMES ${OGRE_LIBRARY_NAME}
                             HINTS ${OGRE_LIBRARY_DIRS} NO_DEFAULT_PATH)
                list (APPEND TMP_OGRE_LIBRARIES "${OGRE_LIBRARY}")
              endforeach()
              set(OGRE_LIBRARIES ${TMP_OGRE_LIBRARIES})
            endif()
          endif()
        endif()
        break()
      endif()
    endif()
  endforeach()

  if (OGRE_FOUND)
    # set OGRE major, minor, and patch version number
    string (REGEX REPLACE "^([0-9]+).*" "\\1"
      OGRE_VERSION_MAJOR "${OGRE_VERSION}")
    string (REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1"
      OGRE_VERSION_MINOR "${OGRE_VERSION}")
    string (REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1"
      OGRE_VERSION_PATCH ${OGRE_VERSION})

    # find ogre components
    foreach(component ${GzOGRE_FIND_COMPONENTS})
      gz_pkg_check_modules_quiet(GzOGRE-${component} "OGRE-${component} >= ${full_version}" NO_CMAKE_ENVIRONMENT_PATH)
      if(GzOGRE-${component}_FOUND)
        list(APPEND OGRE_LIBRARIES GzOGRE-${component}::GzOGRE-${component})
      elseif(GzOGRE_FIND_REQUIRED_${component})
        set(OGRE_FOUND false)
      endif()
    endforeach()

    pkg_get_variable(OGRE_PLUGINDIR OGRE plugindir)
    if(NOT OGRE_PLUGINDIR)
      GZ_BUILD_WARNING ("Failed to find OGRE's plugin directory.  The build will succeed, but there will likely be run-time errors.")
    else()
      # Seems that OGRE_PLUGINDIR can end in a newline, which will cause problems
      # when we pass it to the compiler later.
      string(REPLACE "\n" "" OGRE_PLUGINDIR ${OGRE_PLUGINDIR})
    endif()

    gz_pkg_config_library_entry(GzOGRE OgreMain)

    set(OGRE_RESOURCE_PATH ${OGRE_PLUGINDIR})
  endif()

  #reset pkg config path
  set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH_ORIGINAL})

else()
  find_package(OGRE ${full_version}
  COMPONENTS ${GzOGRE_FIND_COMPONENTS})
  if(OGRE_FOUND)
    # OGREConfig.cmake from vcpkg disable the link against plugin libs
    # when compiling the shared version of it. Here we copied the code
    # to use it.
    foreach(ogre_component ${GzOGRE_FIND_COMPONENTS})
      if(ogre_component MATCHES "Plugin_" OR ogre_component MATCHES "RenderSystem_")
        string(LENGTH "${ogre_component}" len)
        string(FIND "${ogre_component}" "_" split_pos)
        math(EXPR split_pos2 "${split_pos}+1")
        string(SUBSTRING "${ogre_component}" "0" "${split_pos}" component_type)
        string(SUBSTRING "${ogre_component}" "${split_pos2}" "${len}" component_name)

        _gz_ogre_declare_plugin("${component_type}" "${component_name}")
      endif()
    endforeach()

    # need to return only libraries defined by components and give them the
    # full path using OGRE_LIBRARY_DIRS
    # Note: the OGREConfig.cmake installed by vcpkg generates variables that
    # contain unwanted substrings so the string regex replace is added to
    # fix the ogre dir path and lib vars.
    # TODO(anyone) check if this is an OGRE vcpkg config issue.
    string(REGEX REPLACE "\\$.*>" "" OGRE_LIBRARY_DIRS ${OGRE_LIBRARY_DIRS})
    set(ogre_all_libs)
    foreach(ogre_lib ${OGRE_LIBRARIES})
      # dirty hack to be able to know which ogre libraries are defined by the
      # absolute paths. These variables are sometimes multi-configuration vars
      # (i.e: C:/vcpkg/installed/x64-windows$<$<CONFIG:Debug>:/debug>/..)
      # IS_ABSOLUTE cmake function seems not to be working fine with them.
      string(SUBSTRING "${OGRE_LIBRARY_DIRS}" 0 5 PATH_PREFIX)
      string(SUBSTRING "${ogre_lib}" 0 5 ogrelib_PATH_PREFIX)
      if(ogrelib_PATH_PREFIX STREQUAL PATH_PREFIX)
        set(lib_fullpath "${ogre_lib}")
      else()
        string(REGEX REPLACE "\\$.*>" "" ogre_lib ${ogre_lib})
        # Be sure that all Ogre* libraries are using absolute paths
        set(prefix "")
	# vcpkg uses special directory (lib/manual-link/) to place libraries
	# with main sysmbol like OgreMain.
	if(ogre_lib MATCHES "OgreMain" AND NOT IS_ABSOLUTE "${ogre_lib}" AND EXISTS "${OGRE_LIBRARY_DIRS}/manual-link/")
          set(prefix "${OGRE_LIBRARY_DIRS}/manual-link/")
	elseif(ogre_lib MATCHES "Ogre" AND NOT IS_ABSOLUTE "${ogre_lib}")
          set(prefix "${OGRE_LIBRARY_DIRS}/")
        endif()
        if(ogre_lib MATCHES "Plugin_" OR ogre_lib MATCHES "RenderSystem_")
          if(NOT IS_ABSOLUTE "${ogre_lib}")
            set(prefix "${OGRE_LIBRARY_DIRS}/OGRE/")
          endif()
        endif()
        # Some Ogre libraries are not using the .lib extension
        set(postfix "")
        if(NOT ogre_lib MATCHES ".lib$")
          # Do not consider imported targets as libraries
          if(NOT ogre_lib MATCHES "::")
            set(postfix ".lib")
          endif()
        endif()
        set(lib_fullpath "${prefix}${ogre_lib}${postfix}")
      endif()
      list(APPEND ogre_all_libs ${lib_fullpath})
    endforeach()

    set(OGRE_LIBRARIES ${ogre_all_libs})
    set(OGRE_RESOURCE_PATH ${OGRE_CONFIG_DIR})
  endif()
endif()

set(GzOGRE_FOUND false)
if(OGRE_FOUND)
  set(GzOGRE_FOUND true)

  # manually search and append the the RenderSystem/GL path to
  # OGRE_INCLUDE_DIRS so OGRE GL headers can be found
  foreach(dir ${OGRE_INCLUDE_DIRS})
    get_filename_component(dir_name "${dir}" NAME)
    if("${dir_name}" STREQUAL "OGRE")
      if(${OGRE_VERSION} VERSION_LESS 1.11.0)
        set(dir_include "${dir}/RenderSystems/GL")
      else()
        set(dir_include "${dir}/RenderSystems/GL" "${dir}/Paging")
      endif()
    else()
      set(dir_include "${dir}")
    endif()
    list(APPEND OGRE_INCLUDE_DIRS ${dir_include})
  endforeach()

  include(GzImportTarget)
  gz_import_target(GzOGRE
    TARGET_NAME GzOGRE::GzOGRE
    LIB_VAR OGRE_LIBRARIES
    INCLUDE_VAR OGRE_INCLUDE_DIRS)
endif()
