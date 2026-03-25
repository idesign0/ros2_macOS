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
# Find FreeImage

# Grab the version numbers requested by the call to find_package(~)
set(major_version ${FreeImage_FIND_VERSION_MAJOR})
set(minor_version ${FreeImage_FIND_VERSION_MINOR})

# Set the full version number
set(full_version ${major_version}.${minor_version})

if (NOT WIN32)
  include(GzPkgConfig)
  gz_pkg_config_library_entry(FreeImage freeimage)

  # If we don't have PkgConfig, or if PkgConfig failed, then do a manual search
  if(NOT FreeImage_FOUND)

    find_path(FreeImage_INCLUDE_DIRS FreeImage.h)
    if(NOT FreeImage_INCLUDE_DIRS)

      if(NOT FreeImage_FIND_QUIETLY)
        message(STATUS "Looking for FreeImage.h - not found")
        message(STATUS "Missing: Unable to find FreeImage.h")
      endif()

    else(NOT FreeImage_INCLUDE_DIRS)
      # Check the FreeImage header for the right version
      set(testFreeImageSource ${CMAKE_CURRENT_BINARY_DIR}/CMakeTmp/test_freeimage.c)
      set(FreeImage_test_output "")
      set(FreeImage_compile_output "")
      file(WRITE ${testFreeImageSource}
        "#include <FreeImage.h>\nint main () { if (FREEIMAGE_MAJOR_VERSION >= ${major_version} && FREEIMAGE_MINOR_VERSION >= ${minor_version}) return 1; else return 0;} \n")

      try_run(FREEIMAGE_RUNS
              FREEIMAGE_COMPILES
              ${CMAKE_CURRENT_BINARY_DIR}
              ${testFreeImageSource}
              CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${FreeImage_INCLUDE_DIRS}
              RUN_OUTPUT_VARIABLE FreeImage_test_output
              COMPILE_OUTPUT_VARIABLE FreeImage_compile_output)

      if(NOT FREEIMAGE_COMPILES)

        if(NOT FreeImage_FIND_QUIETLY)
          message(STATUS "FreeImage test failed to compile - This may indicate a build system bug")
        endif()

        return()

      endif(NOT FREEIMAGE_COMPILES)

      if(NOT FREEIMAGE_RUNS)
        if(NOT FreeImage_FIND_QUIETLY)
          message(STATUS "Invalid FreeImage Version. Requires ${major_version}.${minor_version}")
        endif()
      endif(NOT FREEIMAGE_RUNS)

    endif(NOT FreeImage_INCLUDE_DIRS)
    mark_as_advanced(FreeImage_INCLUDE_DIRS)

    find_library(FreeImage_LIBRARIES NAMES freeimage FreeImage)
    if(FreeImage_LIBRARIES)
      set(FreeImage_FOUND true)
    else()
      if(NOT FreeImage_FIND_QUIELTY)
        message(STATUS "Missing: Unable to find libfreeimage")
      endif()
    endif(FreeImage_LIBRARIES)
    mark_as_advanced(FreeImage_LIBRARIES)

  endif()

  if(FreeImage_FOUND)
    # Create the imported target for FreeImage if we found it
    include(GzImportTarget)
    gz_import_target(FreeImage)
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    FreeImage
    REQUIRED_VARS FreeImage_FOUND)
else()
  # true by default, change to false when a failure appears
  set(FreeImage_FOUND true)

  # 1. look for FreeImage headers
  find_path(FreeImage_INCLUDE_DIRS FreeImage.h
    hints
      ${CMAKE_FIND_ROOT_PATH}
    paths
      ${CMAKE_FIND_ROOT_PATH}
    doc "FreeImage header include dir"
    path_suffixes
      include
  )

  if (FreeImage_INCLUDE_DIRS)
    if(NOT FreeImage_FIND_QUIETLY)
      message(STATUS "Looking for FreeImage.h FreeImageconfig.h - found")
    endif()
  else()
    if(NOT FreeImage_FIND_QUIETLY)
      message(STATUS "Looking for FreeImage.h FreeImageconfig.h - not found")
    endif()

    set(FreeImage_FOUND false)
  endif()
  mark_as_advanced(FreeImage_INCLUDE_DIRS)

  # 2. look for FreeImage libraries
  find_library(FreeImage_LIBRARIES FreeImage)
  mark_as_advanced(FreeImage_LIBRARIES)

  if (FreeImage_LIBRARIES)
    if(NOT FreeImage_FIND_QUIETLY)
      message(STATUS "Looking for FreeImage library - found")
    endif()
  else()
    if(NOT FreeImage_FIND_QUIETLY)
      message(STATUS "Looking for FreeImage library - not found")
    endif()

    set (FreeImage_FOUND false)
  endif()

  if (FreeImage_FOUND)
    include(GzPkgConfig)
    gz_pkg_config_library_entry(FreeImage "FreeImage")
    include(GzImportTarget)
    gz_import_target(FreeImage)
  endif()

endif()
