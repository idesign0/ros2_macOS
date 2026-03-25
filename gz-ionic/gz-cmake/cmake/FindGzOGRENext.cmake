#===============================================================================
# Copyright (C) 2025 Open Source Robotics Foundation
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
# Find OGRE-Next headers and libraries
#
# Usage of this module as follows:
#
#     gz_find_package(GzOGRENext)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  GZ_OGRE_NEXT_PROJECT_NAME  Possible values: OGRE (default) or OGRE-Next
#                             Specify the project name used in the packaging.
#                             It will impact directly in the name of the
#                             CMake/pkg-config modules being used.
#
# Variables defined by this module:
#
#  OGRE_NEXT_FOUND          System has OGRE libs/headers
#  OGRE_NEXT_LIBRARIES      The OGRE libraries
#  OGRE_NEXT_INCLUDE_DIRS   The location of OGRE headers
#  OGRE_NEXT_VERSION        Full OGRE version in the form of MAJOR.MINOR.PATCH
#  OGRE_NEXT_VERSION_MAJOR  OGRE major version
#  OGRE_NEXT_VERSION_MINOR  OGRE minor version
#  OGRE_NEXT_VERSION_PATCH  OGRE patch version
#  OGRE_NEXT_RESOURCE_PATH  Path to ogre plugins directory
#  GzOGRENext::GzOGRENext   Imported target for OGRE-Next
#
# Supports finding the following OGRE-Next components: HlmsPbs, HlmsUnlit,
# Overlay, PlanarReflections, Atmosphere (only for version>=3.0.0)
#
# Example usage:
#
#     gz_find_package(GzOGRENext
#                     VERSION 3.0.0
#                     COMPONENTS HlmsPbs HlmsUnlit Overlay)


# check possible ogre2 was not requested to be found or was not found using
# VERSION_MAJOR parameter and show an error from MESSAGE_STRING parameter and
# set OGRE2_TRIGGERED parameter to true if VERSION_MAJOR==2
# for additional details see
# https://github.com/gazebosim/gz-cmake/pull/468#issuecomment-2662882691
macro(check_possible_ogre2 VERSION_MAJOR MESSAGE_STRING OGRE2_TRIGGERED)
  # set the default value of OGRE2_TRIGGERED parameter
  set(OGRE2_TRIGGERED false)
  if (DEFINED ${VERSION_MAJOR})
    if (${VERSION_MAJOR} VERSION_EQUAL "2")
      message(SEND_ERROR "OGRE-Next with major version ${${VERSION_MAJOR}} ${MESSAGE_STRING} but OGRE-Next<3.0.0 is not supported yet by GzOGRENext. Please, use instead GzOGRE2.")
      message(WARNING
          "Keep in mind FindGzOGRE2.cmake is planned to be removed and FindGzOGRENext.cmake is planned to support OGRE-Next>=2.0.0 soon")
      set(OGRE_NEXT_FOUND false)
      # unset variables so that we don't leak incorrect versions
      set(OGRE_NEXT_VERSION "")
      set(OGRE_NEXT_VERSION_MAJOR "")
      set(OGRE_NEXT_VERSION_MINOR "")
      set(OGRE_NEXT_VERSION_PATCH "")
      set(OGRE_NEXT_LIBRARIES "")
      set(OGRE_NEXT_INCLUDE_DIRS "")
      set(OGRE2_TRIGGERED true)
    endif()
  endif()
endmacro()
# check if ogre-next 2.x.x was requested to be found
check_possible_ogre2(
  GzOGRENext_FIND_VERSION_MAJOR
  "was requested to be found"
  OGRE2_TRIGGERED
)
if (${OGRE2_TRIGGERED})
  return()
endif()

# Sanity check: exclude OGRE1 project releasing versions in two ways:
#  - Legacy: from 1.x.y until 1.12.y series
#  - Modern: starting with 13.y.z
if (DEFINED GzOGRENext_FIND_VERSION_MAJOR)
  if (${GzOGRENext_FIND_VERSION_MAJOR} VERSION_LESS "2" OR
      ${GzOGRENext_FIND_VERSION_MAJOR} VERSION_GREATER_EQUAL "13")
    set(OGRE_NEXT_FOUND false)
    message(STATUS
        "The specified major version is not supported. Probably, OGRE-Next of that version does not exist and you are looking for OGRE.")
    return()
  endif()
endif()

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

# check that PkgConfig is present because finding OGRE-Next is supported
# (in this package) only via PkgConfig yet.
# later in the code, gz_pkg_check_modules_quiet will try to find OGRE-Next
# using PkgConfig.
find_package(PkgConfig REQUIRED)

foreach (GZ_OGRE_NEXT_PROJECT_NAME "OGRE-Next" "OGRE")
  message(STATUS
      "Looking for OGRE-Next using the name: ${GZ_OGRE_NEXT_PROJECT_NAME}")
  if (GZ_OGRE_NEXT_PROJECT_NAME STREQUAL "OGRE")
    set(OGRE_NEXT_INSTALL_PATH "OGRE")
    set(OGRE_NEXT_LIBNAME "Ogre")
  else()
    set(OGRE_NEXT_INSTALL_PATH "OGRE-Next")
    set(OGRE_NEXT_LIBNAME "OgreNext")
  endif()

  # look for OGRE-Next
  gz_pkg_check_modules_quiet(
      ${GZ_OGRE_NEXT_PROJECT_NAME}
      ${OGRE_NEXT_INSTALL_PATH}
      NO_CMAKE_ENVIRONMENT_PATH QUIET)

  if (${GZ_OGRE_NEXT_PROJECT_NAME}_FOUND)
    set(GZ_PKG_NAME ${OGRE_NEXT_INSTALL_PATH})
    set(OGRE_NEXT_FOUND ${${GZ_OGRE_NEXT_PROJECT_NAME}_FOUND})
    set(OGRE_NEXT_LIBRARY_DIRS ${${GZ_OGRE_NEXT_PROJECT_NAME}_LIBRARY_DIRS})
    set(OGRE_NEXT_LIBRARIES ${${GZ_OGRE_NEXT_PROJECT_NAME}_LIBRARIES})
  else()
    set(OGRE_NEXT_FOUND FALSE)
    message(STATUS "  ! ${GZ_OGRE_NEXT_PROJECT_NAME}: not found")
    continue()
  endif()

  # use pkg-config to find ogre plugin path
  # do it here before resetting the pkg-config paths
  pkg_get_variable(OGRE_NEXT_PLUGINDIR ${GZ_PKG_NAME} plugindir)

  if (NOT OGRE_NEXT_PLUGINDIR)
    GZ_BUILD_WARNING ("Failed to find OGRE-Next's plugin directory. The build will succeed, but there will likely be run-time errors.")
  endif()

  set(OGRE_NEXT_INCLUDE_DIRS ${${GZ_OGRE_NEXT_PROJECT_NAME}_INCLUDE_DIRS})

  unset(OGRE_NEXT_INCLUDE CACHE)
  unset(OGRE_NEXT_INCLUDE)
  # verify ogre header can be found in the include path
  find_path(OGRE_NEXT_INCLUDE
    NAMES Ogre.h
    PATHS ${OGRE_NEXT_INCLUDE_DIRS}
    NO_DEFAULT_PATH
  )

  if (NOT OGRE_NEXT_INCLUDE)
    set(OGRE_NEXT_FOUND false)
    continue()
  endif()

  # manually search and append the the RenderSystem/GL3Plus path to
  # OGRE_NEXT_INCLUDE_DIRS so OGRE GL headers can be found
  foreach (dir ${OGRE_NEXT_INCLUDE_DIRS})
    get_filename_component(dir_name "${dir}" NAME)
    if ("${dir_name}" STREQUAL ${GZ_PKG_NAME})
      set(dir_include "${dir}/RenderSystems/GL3Plus")
    else()
      set(dir_include "${dir}")
    endif()
    list(APPEND OGRE_NEXT_INCLUDE_DIRS ${dir_include})
  endforeach()

  file(READ
      ${OGRE_NEXT_INCLUDE}/OgrePrerequisites.h OGRE_TEMP_VERSION_CONTENT)
  get_preprocessor_entry(
      OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MAJOR OGRE_NEXT_VERSION_MAJOR)
  if (NOT OGRE_NEXT_VERSION_MAJOR)
    message(STATUS
        "  ! Couldn't determine the major version. Probably, OGRE (not OGRE-Next) was found")
    message(STATUS "  ! ${GZ_OGRE_NEXT_PROJECT_NAME}: not found")
    set(OGRE_NEXT_FOUND FALSE)
    continue()
  endif()
  get_preprocessor_entry(
      OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MINOR OGRE_NEXT_VERSION_MINOR)
  get_preprocessor_entry(
      OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_PATCH OGRE_NEXT_VERSION_PATCH)
  get_preprocessor_entry(
      OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_NAME OGRE_NEXT_VERSION_NAME)
  set(OGRE_NEXT_VERSION
      "${OGRE_NEXT_VERSION_MAJOR}.${OGRE_NEXT_VERSION_MINOR}.${OGRE_NEXT_VERSION_PATCH}")

  # check if ogre-next 2.x.x was found
  check_possible_ogre2(
    OGRE_NEXT_VERSION_MAJOR
    "was found"
    OGRE2_TRIGGERED
  )
  if (${OGRE2_TRIGGERED})
    continue()
  endif()

  # Sanity check: exclude OGRE1 project releasing versions in two ways:
  #  - Legacy: from 1.x.y until 1.12.y series
  #  - Modern: starting with 13.y.z
  if (${OGRE_NEXT_VERSION_MAJOR} VERSION_LESS "2" OR
      ${OGRE_NEXT_VERSION_MAJOR} VERSION_GREATER_EQUAL "13")
    message(STATUS
        "  ! The major version of the found package is not supported. Probably, OGRE-Next of that version does not exist and instead OGRE was found.")
    message(STATUS "  ! ${GZ_OGRE_NEXT_PROJECT_NAME}: not found")
    set (OGRE_NEXT_FOUND false)
    continue()
  endif()

  set(GzOGRENext_VERSION_EXACT FALSE)
  set(GzOGRENext_VERSION_COMPATIBLE FALSE)

  if (GzOGRENext_FIND_VERSION_MAJOR AND
      NOT ("${OGRE_NEXT_VERSION_MAJOR}" EQUAL
      "${GzOGRENext_FIND_VERSION_MAJOR}"))
    message(STATUS "  ! ${GZ_OGRE_NEXT_PROJECT_NAME} found with incompatible major version ${OGRE_NEXT_VERSION}")
    set(OGRE_NEXT_FOUND FALSE)
    continue()
  endif()

  if (GzOGRENext_FIND_VERSION_MINOR AND
      NOT ("${OGRE_NEXT_VERSION_MINOR}" EQUAL
      "${GzOGRENext_FIND_VERSION_MINOR}"))
    message(STATUS "  ! ${GZ_OGRE_NEXT_PROJECT_NAME} found with incompatible minor version ${OGRE_NEXT_VERSION}")
    set(OGRE_NEXT_FOUND FALSE)
    continue()
  endif()

  if ("${OGRE_NEXT_VERSION}" VERSION_EQUAL "${GzOGRENext_FIND_VERSION}")
    set(GzOGRENext_VERSION_EXACT TRUE)
    set(GzOGRENext_VERSION_COMPATIBLE TRUE)
  endif()

  if ("${OGRE_NEXT_VERSION}" VERSION_GREATER "${GzOGRENext_FIND_VERSION}")
    set(GzOGRENext_VERSION_COMPATIBLE TRUE)
  endif()

  # find ogre components
  include(GzImportTarget)
  foreach(component ${GzOGRENext_FIND_COMPONENTS})
    find_library(OGRE_NEXT-${component}
      NAMES
        "${OGRE_NEXT_LIBNAME}${component}_d.${OGRE_NEXT_VERSION}"
        "${OGRE_NEXT_LIBNAME}${component}_d"
        "${OGRE_NEXT_LIBNAME}${component}.${OGRE_NEXT_VERSION}"
        "${OGRE_NEXT_LIBNAME}${component}"
      HINTS ${OGRE_NEXT_LIBRARY_DIRS})
    if (NOT "${OGRE_NEXT-${component}}" STREQUAL
        "OGRE_NEXT-${component}-NOTFOUND")
      message(STATUS "  + component ${component}: found")
      # create a new target for each component
      set(component_TARGET_NAME
          "GzOGRENext-${component}::GzOGRENext-${component}")
      set(component_INCLUDE_DIRS ${OGRE_NEXT_INCLUDE_DIRS})

        foreach (dir ${OGRE_NEXT_INCLUDE_DIRS})
          get_filename_component(dir_name "${dir}" NAME)
          if ("${dir_name}" STREQUAL ${GZ_PKG_NAME})
            # 1. append the Hlms/Common include dir if it exists.
            string(FIND ${component} "Hlms" HLMS_POS)
            if (${HLMS_POS} GREATER -1)
              set(dir_include "${dir}/Hlms/Common")
              if (EXISTS ${dir_include})
                list(APPEND component_INCLUDE_DIRS ${dir_include})
              endif()
            endif()
            # 2. append the PlanarReflections include
            if (${component} STREQUAL "PlanarReflections")
              list(APPEND component_INCLUDE_DIRS "${dir}/PlanarReflections")
            endif()
            # 3. append the Atmosphere include
            if (${component} STREQUAL "Atmosphere" AND
                ${OGRE_NEXT_VERSION_MAJOR} GREATER_EQUAL 3)
              list(APPEND component_INCLUDE_DIRS "${dir}/Atmosphere")
            endif()
          endif()
        endforeach()

      set(component_LIBRARY_DIRS ${OGRE_NEXT_LIBRARY_DIRS})
      set(component_LIBRARIES ${OGRE_NEXT-${component}})
      gz_import_target(${component}
        TARGET_NAME ${component_TARGET_NAME}
        LIB_VAR component_LIBRARIES
        INCLUDE_VAR component_INCLUDE_DIRS)

      # Forward the link directories to be used by RPath
      set_property(
        TARGET ${component_TARGET_NAME}
        PROPERTY INTERFACE_LINK_DIRECTORIES
        ${OGRE_NEXT_LIBRARY_DIRS}
      )
      # add it to the list of ogre libraries
      list(APPEND OGRE_NEXT_LIBRARIES ${component_TARGET_NAME})

    elseif(GzOGRENext_FIND_REQUIRED_${component})
      message(STATUS "  ! component ${component}: not found!")
      set(OGRE_NEXT_FOUND false)
    endif()
  endforeach()

  # OGRE was found using the current value in the loop. No need to iterate
  # more times.
  if (OGRE_NEXT_FOUND)
    break()
  else()
    message(STATUS "  ! ${GZ_OGRE_NEXT_PROJECT_NAME}: not found")
  endif()
endforeach()

if (NOT OGRE_NEXT_FOUND)
  return()
endif()

if ("${OGRE_NEXT_PLUGINDIR}" STREQUAL "")
  # set path to find ogre plugins
  # keep variable naming consistent with ogre 1
  # TODO currently using harded paths based on dir structure in ubuntu
  foreach(resource_path ${OGRE_NEXT_LIBRARY_DIRS})
    foreach (ogre_next_plugin_dir_name "OGRE" "OGRE-Next")
      if (EXISTS "${resource_path}/${ogre_next_plugin_dir_name}")
        list(APPEND OGRE_NEXT_RESOURCE_PATH "${resource_path}/${ogre_next_plugin_dir_name}")
      endif()
    endforeach()
  endforeach()
else()
  set(OGRE_NEXT_RESOURCE_PATH ${OGRE_NEXT_PLUGINDIR})
  # Seems that OGRE_NEXT_PLUGINDIR can end in a newline, which
  # will cause problems when we pass it to the compiler later.
  string(REPLACE "\n" "" OGRE_NEXT_RESOURCE_PATH ${OGRE_NEXT_RESOURCE_PATH})
endif()

# We need to manually specify the pkgconfig entry (and type of entry),
# because gz_pkg_check_modules does not work for it.
include(GzPkgConfig)
gz_pkg_config_library_entry(GzOGRENext OgreMain)

set(GzOGRENext_FOUND false)
# create OGRENext target
if (OGRE_NEXT_FOUND)
  set(GzOGRENext_FOUND true)

  gz_import_target(GzOGRENext
    TARGET_NAME GzOGRENext::GzOGRENext
    LIB_VAR OGRE_NEXT_LIBRARIES
    INCLUDE_VAR OGRE_NEXT_INCLUDE_DIRS)

  # Forward the link directories to be used by RPath
  set_property(
    TARGET GzOGRENext::GzOGRENext
    PROPERTY INTERFACE_LINK_DIRECTORIES
    ${OGRE_NEXT_LIBRARY_DIRS}
  )
else()
  # Unset variables so that we don't leak incorrect versions
  set(OGRE_NEXT_VERSION "")
  set(OGRE_NEXT_VERSION_MAJOR "")
  set(OGRE_NEXT_VERSION_MINOR "")
  set(OGRE_NEXT_VERSION_PATCH "")
  set(OGRE_NEXT_LIBRARIES "")
  set(OGRE_NEXT_INCLUDE_DIRS "")
endif()
