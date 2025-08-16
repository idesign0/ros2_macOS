# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_control_toolbox_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED control_toolbox_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(control_toolbox_FOUND FALSE)
  elseif(NOT control_toolbox_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(control_toolbox_FOUND FALSE)
  endif()
  return()
endif()
set(_control_toolbox_CONFIG_INCLUDED TRUE)

# output package information
if(NOT control_toolbox_FIND_QUIETLY)
  message(STATUS "Found control_toolbox: 3.6.1 (${control_toolbox_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'control_toolbox' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${control_toolbox_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(control_toolbox_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "ament_cmake_export_dependencies-extras.cmake;ament_cmake_export_targets-extras.cmake")
foreach(_extra ${_extras})
  include("${control_toolbox_DIR}/${_extra}")
endforeach()
