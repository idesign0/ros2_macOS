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
# Find OGRE2 headers and libraries
#
# Usage of this module as follows:
#
#     gz_find_package(GzOGRE2)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  GZ_OGRE2_PROJECT_NAME    Possible values: OGRE2 (default) or OGRE-Next
#                            Specify the project name used in the packaging.
#                            It will impact directly in the name of the
#                            CMake/pkg-config modules being used.
#
# Variables defined by this module:
#
#  OGRE2_FOUND              System has OGRE libs/headers
#  OGRE2_LIBRARIES          The OGRE libraries
#  OGRE2_INCLUDE_DIRS       The location of OGRE headers
#  OGRE2_VERSION            Full OGRE version in the form of MAJOR.MINOR.PATCH
#  OGRE2_VERSION_MAJOR      OGRE major version
#  OGRE2_VERSION_MINOR      OGRE minor version
#  OGRE2_VERSION_PATCH      OGRE patch version
#  OGRE2_RESOURCE_PATH      Path to ogre plugins directory
#  GzOGRE2::GzOGRE2       Imported target for OGRE2
#
# Supports finding the following OGRE2 components: HlmsPbs, HlmsUnlit, Overlay,
#  PlanarReflections
#
# Example usage:
#
#     gz_find_package(GzOGRE2
#                     VERSION 2.2.0
#                     COMPONENTS HlmsPbs HlmsUnlit Overlay)

# check ogre-next>=3.0.0 was not requested to be found or was not found using
# VERSION_MAJOR parameter and show a warning from MESSAGE_STRING parameter
# for additional details see
# https://github.com/gazebosim/gz-cmake/pull/468#issuecomment-2662882691
macro(check_possible_ogre_next_3_or_later VERSION_MAJOR MESSAGE_STRING)
  if (${VERSION_MAJOR})
    if (${VERSION_MAJOR} VERSION_GREATER "2")
      message(WARNING "OGRE-Next with major version ${${VERSION_MAJOR}} ${MESSAGE_STRING} but OGRE-Next>=3.0.0 is not supported by GzOGRE2. Please, use instead GzOGRENext.")
    endif()
  endif()
endmacro()
# check if ogre-next>=3.0.0 was requested to be found
check_possible_ogre_next_3_or_later(
  GzOGRE2_FIND_VERSION_MAJOR
  "was requested to be found"
)

if (NOT (DEFINED GzOGRE2_FIND_VERSION_MAJOR AND DEFINED
    GzOGRE2_FIND_VERSION_MINOR))
  message(WARNING
    "find_package(GzOGRE2) must be called with a VERSION argument with a minimum of major and minor version")
  set(OGRE2_FOUND false)
  return()
endif()

# Sanity check: exclude OGRE1 project releasing versions in two ways:
#  - Legacy in from using 1.x.y until 1.12.y series
#  - Modern versions using X.Y.Z starting with 13.y.z
# Reduce valid versions to 2.x series
if (${GzOGRE2_FIND_VERSION_MAJOR})
  if (${GzOGRE2_FIND_VERSION_MAJOR} VERSION_LESS "2" OR
      ${GzOGRE2_FIND_VERSION_MAJOR} VERSION_GREATER_EQUAL "3")
    set (OGRE2_FOUND false)
    return()
  endif()
endif()

macro(append_library VAR LIB)
  if(EXISTS "${LIB}")
    list(APPEND ${VAR} ${LIB})
  endif()
endmacro()

# Ubuntu Jammy version 2.2.5+dfsg3-0ubuntu2 is buggy in the pkg-config files
# duplicating usr/usr for some paths in pkg-config
macro(fix_pkgconfig_prefix_jammy_bug FILESYSTEM_PATH OUTPUT_VAR)
  if (EXISTS "${FILESYSTEM_PATH}")
    string(REPLACE "/usr//usr" "/usr"
      ${OUTPUT_VAR}
      ${FILESYSTEM_PATH})
  endif()
endmacro()

# Ubuntu Jammy version 2.2.5+dfsg3-0ubuntu2 is buggy in the pkg-config files
# using a non existing path /usr/lib/${arch}/OGRE/OGRE-Next insted of the path
# /usr/lib/${arch}/OGRE-Next which is the right one
macro(fix_pkgconfig_resource_path_jammy_bug FILESYSTEM_PATH OUTPUT_VAR)
  if (EXISTS "${FILESYSTEM_PATH}")
    string(REPLACE "OGRE/OGRE-Next" "OGRE-Next"
      ${OUTPUT_VAR}
      ${FILESYSTEM_PATH})
  endif()
endmacro()

# filter all ocurrences of LIBRARY_STR with the form of: debug;<path>;optimized;<path>
# based on CMAKE_BUILD_TYPE
macro(select_lib_by_build_type LIBRARY_STR OUTPUT_VAR)
  foreach(library ${LIBRARY_STR})
    if(library STREQUAL optimized)
      set(conf optimized)
    elseif(library STREQUAL debug)
      set(conf debug)
    else()
      if(conf STREQUAL optimized)
        append_library(LIB_RELEASE ${library})
        set(conf)
      elseif(conf STREQUAL debug)
        append_library(LIB_DEBUG ${library})
        set(conf)
      else()
        # assume library without debug/optimized prefix
        append_library(LIB_RELEASE ${library})
        append_library(LIB_DEBUG ${library})
      endif()
    endif()
  endforeach()

  if(LIB_DEBUG AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(${OUTPUT_VAR} "${LIB_DEBUG}")
  elseif(LIB_RELEASE)
    set(${OUTPUT_VAR} "${LIB_RELEASE}")
  endif()
endmacro()

# this macro and the version parsing logic below is taken from the
# FindOGRE.cmake file distributed by ogre
macro(get_preprocessor_entry CONTENTS KEYWORD VARIABLE)
  string(REGEX MATCH
    "# *define +${KEYWORD} +((\"([^\n]*)\")|([^ \n]*))"
    PREPROC_TEMP_VAR
    ${${CONTENTS}}
  )
  if (CMAKE_MATCH_3)
    set(${VARIABLE} ${CMAKE_MATCH_3})
  else ()
    set(${VARIABLE} ${CMAKE_MATCH_4})
  endif ()
endmacro()

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
  set(PKG_CONFIG_PATH_ORIGINAL $ENV{PKG_CONFIG_PATH})
  foreach (GZ_OGRE2_PROJECT_NAME "OGRE2" "OGRE-Next")
    message(STATUS "Looking for OGRE using the name: ${GZ_OGRE2_PROJECT_NAME}")
    if (GZ_OGRE2_PROJECT_NAME STREQUAL "OGRE2")
      set(OGRE2_INSTALL_PATH "OGRE-2.${GzOGRE2_FIND_VERSION_MINOR}")
      # For OGRE 2.3 debs built via OpenRobotics buildfarms, we use OgreNext
      # For OGRE 2.3 macOS homebrew builds retain the OGRE name
      # For OGRE 2.2 and below retain the OGRE name
      if (${GzOGRE2_FIND_VERSION_MINOR} GREATER_EQUAL "3" AND NOT APPLE)
        set(OGRE2LIBNAME "OgreNext")
      else()
        set(OGRE2LIBNAME "Ogre")
      endif()
    else()
      # This matches OGRE2.2 debs built in upstream Ubuntu
      set(OGRE2_INSTALL_PATH "OGRE-Next")
      set(OGRE2LIBNAME "OgreNext")
    endif()

    # Note: OGRE2 installed from debs is named OGRE-2.2 while the version
    # installed from source does not have the 2.2 suffix
    # look for OGRE2 installed from debs
    gz_pkg_check_modules_quiet(${GZ_OGRE2_PROJECT_NAME} ${OGRE2_INSTALL_PATH} NO_CMAKE_ENVIRONMENT_PATH QUIET)

    if (${GZ_OGRE2_PROJECT_NAME}_FOUND)
      set(GZ_PKG_NAME ${OGRE2_INSTALL_PATH})
      set(OGRE2_FOUND ${${GZ_OGRE2_PROJECT_NAME}_FOUND})  # sync possible OGRE-Next to OGRE2
      fix_pkgconfig_prefix_jammy_bug("${${GZ_OGRE2_PROJECT_NAME}_LIBRARY_DIRS}" OGRE2_LIBRARY_DIRS)
      set(OGRE2_LIBRARIES ${${GZ_OGRE2_PROJECT_NAME}_LIBRARIES})  # sync possible Ogre-Next ot OGRE2
    else()
      # look for OGRE2 installed from source
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

      # loop through pkg config paths and find an ogre version that is >= 2.0.0
      foreach(pkg_path ${PKG_CONFIG_PATH_TMP})
        set(ENV{PKG_CONFIG_PATH} ${pkg_path})
        pkg_check_modules(OGRE2 "OGRE" NO_CMAKE_ENVIRONMENT_PATH QUIET)
        if (OGRE2_FOUND)
          if (${OGRE2_VERSION} VERSION_LESS 2.0.0)
            set (OGRE2_FOUND false)
          else ()
            # pkg_check_modules does not provide complete path to libraries
            # So update variable to point to full path
            set(OGRE2_LIBRARY_NAME ${OGRE2_LIBRARIES})
            find_library(OGRE2_LIBRARY NAMES ${OGRE2_LIBRARY_NAME}
                                       HINTS ${OGRE2_LIBRARY_DIRS} NO_DEFAULT_PATH)
            if ("${OGRE2_LIBRARY}" STREQUAL "OGRE2_LIBRARY-NOTFOUND")
              set(OGRE2_FOUND false)
              continue()
            else()
              set(OGRE2_LIBRARIES ${OGRE2_LIBRARY})
            endif()
            set(GZ_PKG_NAME "OGRE")
            break()
          endif()
        endif()
      endforeach()
    endif()

    if (NOT OGRE2_FOUND)
      message(STATUS "  ! ${GZ_OGRE2_PROJECT_NAME} not found")

      # reset pkg config path
      set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH_ORIGINAL})

      continue()
    endif()

    # use pkg-config to find ogre plugin path
    # do it here before resetting the pkg-config paths
    pkg_get_variable(OGRE2_PLUGINDIR ${GZ_PKG_NAME} plugindir)

    if(NOT OGRE2_PLUGINDIR)
      GZ_BUILD_WARNING ("Failed to find OGRE's plugin directory. The build will succeed, but there will likely be run-time errors.")
    else()
      fix_pkgconfig_prefix_jammy_bug("${OGRE2_PLUGINDIR}" OGRE2_PLUGINDIR)
    endif()

    # reset pkg config path
    set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH_ORIGINAL})

    set(OGRE2_INCLUDE_DIRS ${${GZ_OGRE2_PROJECT_NAME}_INCLUDE_DIRS})  # sync possible OGRE-Next to OGRE2

    unset(OGRE2_INCLUDE CACHE)
    unset(OGRE2_INCLUDE)
    # verify ogre header can be found in the include path
    find_path(OGRE2_INCLUDE
      NAMES Ogre.h
      PATHS ${OGRE2_INCLUDE_DIRS}
      NO_DEFAULT_PATH
    )

    if(NOT OGRE2_INCLUDE)
      set(OGRE2_FOUND false)
      continue()
    endif()

    # manually search and append the the RenderSystem/GL3Plus path to
    # OGRE2_INCLUDE_DIRS so OGRE GL headers can be found
    foreach (dir ${OGRE2_INCLUDE_DIRS})
      get_filename_component(dir_name "${dir}" NAME)
      if ("${dir_name}" STREQUAL ${GZ_PKG_NAME})
        set(dir_include "${dir}/RenderSystems/GL3Plus")
      else()
        set(dir_include "${dir}")
      endif()
      list(APPEND OGRE2_INCLUDE_DIRS ${dir_include})
    endforeach()

    file(READ ${OGRE2_INCLUDE}/OgrePrerequisites.h OGRE_TEMP_VERSION_CONTENT)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MAJOR OGRE2_VERSION_MAJOR)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MINOR OGRE2_VERSION_MINOR)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_PATCH OGRE2_VERSION_PATCH)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_NAME OGRE2_VERSION_NAME)
    set(OGRE2_VERSION "${OGRE2_VERSION_MAJOR}.${OGRE2_VERSION_MINOR}.${OGRE2_VERSION_PATCH}")

    set(GzOGRE2_VERSION_EXACT FALSE)
    set(GzOGRE2_VERSION_COMPATIBLE FALSE)

    # check if ogre-next>=3.0.0 was found
    check_possible_ogre_next_3_or_later(
      OGRE2_VERSION_MAJOR
      "was found"
    )

    if (NOT ("${OGRE2_VERSION_MAJOR}" EQUAL "${GzOGRE2_FIND_VERSION_MAJOR}"))
      set(OGRE2_FOUND FALSE)
      continue()
    endif()

    if (NOT ("${OGRE2_VERSION_MINOR}" EQUAL "${GzOGRE2_FIND_VERSION_MINOR}"))
      message(STATUS "  ! ${GZ_OGRE2_PROJECT_NAME} found with incompatible version ${OGRE2_VERSION}")
      set(OGRE2_FOUND FALSE)
      continue()
    endif()

    if ("${OGRE2_VERSION}" VERSION_EQUAL "${GzOGRE2_FIND_VERSION}")
      set(GzOGRE2_VERSION_EXACT TRUE)
      set(GzOGRE2_VERSION_COMPATIBLE TRUE)
    endif()

    if ("${OGRE2_VERSION}" VERSION_GREATER "${GzOGRE2_FIND_VERSION}")
      set(GzOGRE2_VERSION_COMPATIBLE TRUE)
    endif()

    # find ogre components
    include(GzImportTarget)
    foreach(component ${GzOGRE2_FIND_COMPONENTS})
      find_library(OGRE2-${component}
        NAMES
          "${OGRE2LIBNAME}${component}_d.${OGRE2_VERSION}"
          "${OGRE2LIBNAME}${component}_d"
          "${OGRE2LIBNAME}${component}.${OGRE2_VERSION}"
          "${OGRE2LIBNAME}${component}"
        HINTS ${OGRE2_LIBRARY_DIRS})
      if (NOT "${OGRE2-${component}}" STREQUAL "OGRE2-${component}-NOTFOUND")
        message(STATUS "  + component ${component}: found")
        # create a new target for each component
        set(component_TARGET_NAME "GzOGRE2-${component}::GzOGRE2-${component}")
        set(component_INCLUDE_DIRS ${OGRE2_INCLUDE_DIRS})

          foreach (dir ${OGRE2_INCLUDE_DIRS})
            get_filename_component(dir_name "${dir}" NAME)
            if ("${dir_name}" STREQUAL ${GZ_PKG_NAME})
              # 1. append the Hlms/Common include dir if it exists.
              string(FIND ${component} "Hlms" HLMS_POS)
              if(${HLMS_POS} GREATER -1)
                set(dir_include "${dir}/Hlms/Common")
                if (EXISTS ${dir_include})
                  list(APPEND component_INCLUDE_DIRS ${dir_include})
                endif()
              endif()
              # 2. append the PlanarReflections include
              if(${component} STREQUAL "PlanarReflections")
                 list(APPEND component_INCLUDE_DIRS "${dir}/PlanarReflections")
              endif()
            endif()
          endforeach()

        set(component_LIBRARY_DIRS ${OGRE2_LIBRARY_DIRS})
        set(component_LIBRARIES ${OGRE2-${component}})
        gz_import_target(${component} TARGET_NAME ${component_TARGET_NAME}
            LIB_VAR component_LIBRARIES
            INCLUDE_VAR component_INCLUDE_DIRS)

        # Forward the link directories to be used by RPath
        set_property(
          TARGET ${component_TARGET_NAME}
          PROPERTY INTERFACE_LINK_DIRECTORIES
          ${OGRE2_LIBRARY_DIRS}
        )
        # add it to the list of ogre libraries
        list(APPEND OGRE2_LIBRARIES ${component_TARGET_NAME})

      elseif(GzOGRE2_FIND_REQUIRED_${component})
        message(STATUS "  ! component ${component}: not found!")
        set(OGRE2_FOUND false)
      endif()
    endforeach()

    # OGRE was found using the current value in the loop. No need to iterate
    # more times.
    if (OGRE2_FOUND)
      break()
    else()
      message(STATUS "  ! ${GZ_OGRE2_PROJECT_NAME} not found")
    endif()
  endforeach()

  if (NOT OGRE2_FOUND)
    return()
  endif()

  if ("${OGRE2_PLUGINDIR}" STREQUAL "")
    # set path to find ogre plugins
    # keep variable naming consistent with ogre 1
    # TODO currently using harded paths based on dir structure in ubuntu
    foreach(resource_path ${OGRE2_LIBRARY_DIRS})
      foreach (ogre2_plugin_dir_name "OGRE" "OGRE-Next")
        if (EXISTS "${resource_path}/${ogre2_plugin_dir_name}")
          list(APPEND OGRE2_RESOURCE_PATH "${resource_path}/${ogre2_plugin_dir_name}")
        endif()
      endforeach()
    endforeach()
  else()
    set(OGRE2_RESOURCE_PATH ${OGRE2_PLUGINDIR})
    # Seems that OGRE2_PLUGINDIR can end in a newline, which will cause problems
    # when we pass it to the compiler later.
    string(REPLACE "\n" "" OGRE2_RESOURCE_PATH ${OGRE2_RESOURCE_PATH})
  endif()
  fix_pkgconfig_resource_path_jammy_bug("${OGRE2_RESOURCE_PATH}" OGRE2_RESOURCE_PATH)

  # We need to manually specify the pkgconfig entry (and type of entry),
  # because gz_pkg_check_modules does not work for it.
  include(GzPkgConfig)
  gz_pkg_config_library_entry(GzOGRE2 OgreMain)
else() #PkgConfig_FOUND

  set(OGRE2_FOUND TRUE)
  set(OGRE_LIBRARIES "")
  set(OGRE2_VERSION "")
  set(OGRE2_VERSION_MAJOR "")
  set(OGRE2_VERSION_MINOR "")
  set(OGRE2_RESOURCE_PATH "")

  set(OGRE2_SEARCH_VER "OGRE-${GzOGRE2_FIND_VERSION_MAJOR}.${GzOGRE2_FIND_VERSION_MINOR}")
  set(OGRE2_PATHS "")
  set(OGRE2_INC_PATHS "")
  foreach(_rootPath ${VCPKG_CMAKE_FIND_ROOT_PATH})
      list(APPEND OGRE2_PATHS "${_rootPath}/lib/${OGRE2_SEARCH_VER}/")
      list(APPEND OGRE2_PATHS "${_rootPath}/lib/${OGRE2_SEARCH_VER}/manual-link/")
      list(APPEND OGRE2_INC_PATHS "${_rootPath}/include/${OGRE2_SEARCH_VER}")
  endforeach()

  find_library(OGRE2_LIBRARY
    NAMES "OgreMain"
    HINTS ${OGRE2_PATHS}
    NO_DEFAULT_PATH)

  find_path(OGRE2_INCLUDE
    NAMES "Ogre.h"
    HINTS ${OGRE2_INC_PATHS})

  if("${OGRE2_LIBRARY}" STREQUAL "OGRE2_LIBRARY-NOTFOUND")
    set(OGRE2_FOUND false)
  else()
    set(OGRE2_LIBRARIES ${OGRE2_LIBRARY})
  endif()

  if(NOT OGRE2_INCLUDE)
    set(OGRE2_FOUND false)
  endif()

  if (OGRE2_FOUND)
    set(OGRE2_INCLUDE_DIRS ${OGRE2_INCLUDE})
    set(OGRE2_LIBRARY_DIRS ${OGRE2_PATHS})

    file(READ ${OGRE2_INCLUDE}/OgrePrerequisites.h OGRE_TEMP_VERSION_CONTENT)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MAJOR OGRE2_VERSION_MAJOR)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MINOR OGRE2_VERSION_MINOR)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_PATCH OGRE2_VERSION_PATCH)
    get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_NAME OGRE2_VERSION_NAME)
    set(OGRE2_VERSION "${OGRE2_VERSION_MAJOR}.${OGRE2_VERSION_MINOR}.${OGRE2_VERSION_PATCH}")
    set(OGRE_TEMP_VERSION_CONTENT "")

    macro(ogre_find_component COMPONENT HEADER PATH_HINTS)
      set(PREFIX OGRE2_${COMPONENT})
      find_path(${PREFIX}_INCLUDE_DIR
          NAMES ${HEADER}
          HINTS ${OGRE2_INCLUDE_DIRS}
          PATH_SUFFIXES
              ${PATH_HINTS} ${COMPONENT} ${OGRE2_SEARCH_VER}/${COMPONENT})

      find_library(${PREFIX}_LIBRARY
          NAMES
              "Ogre${COMPONENT}"
              "Ogre${COMPONENT}_d"
          HINTS
              ${OGRE2_LIBRARY_DIRS}
          NO_DEFAULT_PATH)

      if (NOT ${PREFIX}_FOUND)
        if (${PREFIX}_INCLUDE_DIR AND ${PREFIX}_LIBRARY)
          set(${PREFIX}_FOUND TRUE)
          set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIR})
          set(${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARY})
          message(STATUS "Found ${PREFIX}: ${${PREFIX}_LIBRARIES}")
        endif()
      endif()
  endmacro()

  macro(ogre_find_plugin PLUGIN HEADER)
    set(PREFIX OGRE2_${PLUGIN})
    string(REPLACE "RenderSystem_" "" PLUGIN_TEMP ${PLUGIN})
    string(REPLACE "Plugin_" "" PLUGIN_NAME ${PLUGIN_TEMP})
      # header files for plugins are not usually needed, but find them anyway if they are present
    set(OGRE2_PLUGIN_PATH_SUFFIXES
      PlugIns
      PlugIns/${PLUGIN_NAME}
      Plugins
      Plugins/${PLUGIN_NAME}
      ${PLUGIN}
      RenderSystems
      RenderSystems/${PLUGIN_NAME}
      ${ARGN})
    find_path(
      ${PREFIX}_INCLUDE_DIR
      NAMES
        ${HEADER}
      HINTS
        ${OGRE2_INCLUDE_DIRS} ${OGRE_PREFIX_SOURCE}
      PATH_SUFFIXES
        ${OGRE2_PLUGIN_PATH_SUFFIXES})
    find_library(${PREFIX}_LIBRARY
      NAMES ${PLUGIN}
      HINTS  ${OGRE2_LIBRARY_DIRS}
      PATH_SUFFIXES "" opt "${OGRE2_SEARCH_VER}" "${OGRE2_SEARCH_VER}/opt")

    if (NOT ${PREFIX}_FOUND)
      if (${PREFIX}_INCLUDE_DIR AND ${PREFIX}_LIBRARY)
          set(${PREFIX}_FOUND TRUE)
          set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIR})
          set(${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARY})
          message(STATUS "Found ${PREFIX}: ${${PREFIX}_LIBRARIES}")
      endif()
    endif()
  endmacro()

  ogre_find_component(Overlay OgreOverlaySystem.h "Overlay")
  ogre_find_component(HlmsPbs OgreHlmsPbs.h Hlms/Pbs/)
  ogre_find_component(HlmsUnlit OgreHlmsUnlit.h Hlms/Unlit)

  ogre_find_plugin(Plugin_ParticleFX OgreParticleFXPrerequisites.h PlugIns/ParticleFX/include)
  ogre_find_plugin(RenderSystem_GL3Plus OgreGL3PlusRenderSystem.h RenderSystems/GL3Plus/include)
  ogre_find_plugin(RenderSystem_Direct3D11 OgreD3D11RenderSystem.h RenderSystems/Direct3D11/include)

  foreach(component ${GzOGRE2_FIND_COMPONENTS})
    set(PREFIX OGRE2_${component})
    if(${PREFIX}_FOUND)
      set(component_TARGET_NAME "GzOGRE2-${component}::GzOGRE2-${component}")
      set(component_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIRS})
      # append the Hlms/Common include dir if it exists.
      string(FIND ${component} "Hlms" HLMS_POS)
      if(${HLMS_POS} GREATER -1)
        foreach (dir ${OGRE2_INCLUDE_DIRS})
          get_filename_component(dir_name "${dir}" NAME)
          if ("${dir_name}" STREQUAL "OGRE-${OGRE2_VERSION_MAJOR}.${OGRE2_VERSION_MINOR}")
            set(dir_include "${dir}/Hlms/Common")
            if (EXISTS ${dir_include})
              list(APPEND component_INCLUDE_DIRS ${dir_include})
            endif()
          endif()
        endforeach()
      endif()

      set(component_LIBRARIES ${${PREFIX}_LIBRARIES})

      gz_import_target(${component}
        TARGET_NAME ${component_TARGET_NAME}
        LIB_VAR component_LIBRARIES
        INCLUDE_VAR component_INCLUDE_DIRS
      )
      list(APPEND OGRE2_LIBRARIES ${component_TARGET_NAME})
    endif()
  endforeach()

  set(OGRE2_PLUGINS_VCPKG Plugin_ParticleFX RenderSystem_GL3Plus RenderSystem_Direct3D11)
  foreach(PLUGIN ${OGRE2_PLUGINS_VCPKG})
    if(OGRE2_${PLUGIN}_FOUND)
      list(APPEND OGRE2_INCLUDE_DIRS ${OGRE2_${PLUGIN}_INCLUDE_DIRS})
    endif()
  endforeach()
  endif()
endif()

set(GzOGRE2_FOUND false)
# create OGRE2 target
if (OGRE2_FOUND)
  set(GzOGRE2_FOUND true)

  gz_import_target(GzOGRE2
    TARGET_NAME GzOGRE2::GzOGRE2
    LIB_VAR OGRE2_LIBRARIES
    INCLUDE_VAR OGRE2_INCLUDE_DIRS)

  # Forward the link directories to be used by RPath
  set_property(
    TARGET GzOGRE2::GzOGRE2
    PROPERTY INTERFACE_LINK_DIRECTORIES
    ${OGRE2_LIBRARY_DIRS}
  )
else()
  # Unset variables so that we don't leak incorrect versions
  set(OGRE2_VERSION "")
  set(OGRE2_VERSION_MAJOR "")
  set(OGRE2_VERSION_MINOR "")
  set(OGRE2_VERSION_PATCH "")
  set(OGRE2_LIBRARIES "")
  set(OGRE2_INCLUDE_DIRS "")
endif()
