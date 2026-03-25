# Copyright (c) 2024 Fraunhofer IOSB and contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


IF(GLIB2_LIBRARIES AND GLIB2_INCLUDE_DIRS)
  # in cache already
  SET(GLIB2_FOUND TRUE)
ELSE()

  #INCLUDE(FindPkgConfig)
  FIND_PACKAGE(PkgConfig QUIET)

  ## Glib
  IF(GLIB2_FIND_REQUIRED)
    SET( _pkgconfig_REQUIRED "REQUIRED" )
  ELSE()
    SET( _pkgconfig_REQUIRED "" )
  ENDIF()

  IF(GLIB2_MIN_VERSION)
    PKG_SEARCH_MODULE( GLIB2 ${_pkgconfig_REQUIRED} glib-2.0>=${GLIB2_MIN_VERSION} )
  ELSE()
    PKG_SEARCH_MODULE( GLIB2 ${_pkgconfig_REQUIRED} glib-2.0 )
  ENDIF()
  IF(PKG_CONFIG_FOUND)
    IF(GLIB2_FOUND)
      SET( GLIB2_CORE_FOUND TRUE )
    ELSE()
      SET( GLIB2_CORE_FOUND FALSE )
    ENDIF()
  ENDIF()

  # Look for glib2 include dir and libraries w/o pkgconfig
  IF(NOT GLIB2_FOUND AND NOT PKG_CONFIG_FOUND)
    FIND_PATH(
      _glibconfig_include_DIR
    NAMES
      glibconfig.h
    PATHS
      /opt/gnome/lib64
      /opt/gnome/lib
      /opt/lib/
      /opt/local/lib
      /sw/lib/
      /usr/lib64
      /usr/lib
      /usr/local/include
      ${CMAKE_LIBRARY_PATH}
    PATH_SUFFIXES
      glib-2.0/include
    )

    FIND_PATH(
      _glib2_include_DIR
    NAMES
      glib.h
    PATHS
      /opt/gnome/include
      /opt/local/include
      /sw/include
      /usr/include
      /usr/local/include
    PATH_SUFFIXES
      glib-2.0
    )

    #MESSAGE(STATUS "Glib headers: ${_glib2_include_DIR}")

    FIND_LIBRARY(
      _glib2_link_DIR
    NAMES
      glib-2.0
      glib
    PATHS
      /opt/gnome/lib
      /opt/local/lib
      /sw/lib
      /usr/lib
      /usr/local/lib
    )
    IF( _glib2_include_DIR AND _glib2_link_DIR )
        SET( _glib2_FOUND TRUE )
    ENDIF()


    IF( _glib2_FOUND )
        SET( GLIB2_INCLUDE_DIRS ${_glib2_include_DIR} ${_glibconfig_include_DIR} )
        SET( GLIB2_LIBRARIES ${_glib2_link_DIR} )
        SET( GLIB2_CORE_FOUND TRUE )
    ELSE()
        SET( GLIB2_CORE_FOUND FALSE )
    ENDIF()

    # Handle dependencies
    # libintl
    IF(NOT LIBINTL_FOUND)
      FIND_PATH(LIBINTL_INCLUDE_DIR
      NAMES
        libintl.h
      PATHS
        /opt/gnome/include
        /opt/local/include
        /sw/include
        /usr/include
        /usr/local/include
      )

      FIND_LIBRARY(LIBINTL_LIBRARY
      NAMES
        intl
      PATHS
        /opt/gnome/lib
        /opt/local/lib
        /sw/lib
        /usr/local/lib
        /usr/lib
      )

      IF(LIBINTL_LIBRARY AND LIBINTL_INCLUDE_DIR)
        SET(LIBINTL_FOUND TRUE)
      ENDIF()
    ENDIF()

    # libiconv
    IF(NOT LIBICONV_FOUND)
      FIND_PATH(LIBICONV_INCLUDE_DIR
      NAMES
        iconv.h
      PATHS
        /opt/gnome/include
        /opt/local/include
        /opt/local/include
        /sw/include
        /sw/include
        /usr/local/include
        /usr/include
      PATH_SUFFIXES
        glib-2.0
      )

      FIND_LIBRARY(LIBICONV_LIBRARY
      NAMES
        iconv
      PATHS
        /opt/gnome/lib
        /opt/local/lib
        /sw/lib
        /usr/lib
        /usr/local/lib
      )

      IF(LIBICONV_LIBRARY AND LIBICONV_INCLUDE_DIR)
        SET(LIBICONV_FOUND TRUE)
      ENDIF()
    ENDIF()

    IF(LIBINTL_FOUND)
      SET(GLIB2_LIBRARIES ${GLIB2_LIBRARIES} ${LIBINTL_LIBRARY})
      SET(GLIB2_INCLUDE_DIRS ${GLIB2_INCLUDE_DIRS} ${LIBINTL_INCLUDE_DIR})
    ENDIF()

    IF(LIBICONV_FOUND)
      SET(GLIB2_LIBRARIES ${GLIB2_LIBRARIES} ${LIBICONV_LIBRARY})
      SET(GLIB2_INCLUDE_DIRS ${GLIB2_INCLUDE_DIRS} ${LIBICONV_INCLUDE_DIR})
    ENDIF()

  ENDIF()
  ##

  IF(GLIB2_CORE_FOUND AND GLIB2_INCLUDE_DIRS AND GLIB2_LIBRARIES)
    SET(GLIB2_FOUND TRUE)
  ENDIF()

  IF(GLIB2_FOUND)
    IF(NOT GLIB2_FIND_QUIETLY)
      MESSAGE(STATUS "Found GLib2: ${GLIB2_LIBRARIES} ${GLIB2_INCLUDE_DIRS}")
    ENDIF()
  ELSE()
    IF(GLIB2_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could not find GLib2")
    ENDIF()
  ENDIF()

  # show the GLIB2_INCLUDE_DIRS and GLIB2_LIBRARIES variables only in the advanced view
  MARK_AS_ADVANCED(GLIB2_INCLUDE_DIRS GLIB2_LIBRARIES)
  MARK_AS_ADVANCED(LIBICONV_INCLUDE_DIR LIBICONV_LIBRARY)
  MARK_AS_ADVANCED(LIBINTL_INCLUDE_DIR LIBINTL_LIBRARY)

ENDIF()

IF(GLIB2_FOUND)
  # Check if system has a newer version of glib
  # which supports g_regex_match_simple
  INCLUDE( CheckIncludeFiles )
  SET( CMAKE_REQUIRED_INCLUDES ${GLIB2_INCLUDE_DIRS} )
  CHECK_INCLUDE_FILES( glib/gregex.h HAVE_GLIB_GREGEX_H )
  # Reset CMAKE_REQUIRED_INCLUDES
  SET( CMAKE_REQUIRED_INCLUDES "" )
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( GLIB2 DEFAULT_MSG
  GLIB2_INCLUDE_DIRS
  GLIB2_LIBRARIES
)
