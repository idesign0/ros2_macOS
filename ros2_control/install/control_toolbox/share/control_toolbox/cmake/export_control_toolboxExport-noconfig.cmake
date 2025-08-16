#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "control_toolbox::control_toolbox" for configuration ""
set_property(TARGET control_toolbox::control_toolbox APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(control_toolbox::control_toolbox PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libcontrol_toolbox.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libcontrol_toolbox.dylib"
  )

list(APPEND _cmake_import_check_targets control_toolbox::control_toolbox )
list(APPEND _cmake_import_check_files_for_control_toolbox::control_toolbox "${_IMPORT_PREFIX}/lib/libcontrol_toolbox.dylib" )

# Import target "control_toolbox::low_pass_filter" for configuration ""
set_property(TARGET control_toolbox::low_pass_filter APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(control_toolbox::low_pass_filter PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/liblow_pass_filter.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/liblow_pass_filter.dylib"
  )

list(APPEND _cmake_import_check_targets control_toolbox::low_pass_filter )
list(APPEND _cmake_import_check_files_for_control_toolbox::low_pass_filter "${_IMPORT_PREFIX}/lib/liblow_pass_filter.dylib" )

# Import target "control_toolbox::rate_limiter" for configuration ""
set_property(TARGET control_toolbox::rate_limiter APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(control_toolbox::rate_limiter PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/librate_limiter.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/librate_limiter.dylib"
  )

list(APPEND _cmake_import_check_targets control_toolbox::rate_limiter )
list(APPEND _cmake_import_check_files_for_control_toolbox::rate_limiter "${_IMPORT_PREFIX}/lib/librate_limiter.dylib" )

# Import target "control_toolbox::exponential_filter" for configuration ""
set_property(TARGET control_toolbox::exponential_filter APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(control_toolbox::exponential_filter PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libexponential_filter.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libexponential_filter.dylib"
  )

list(APPEND _cmake_import_check_targets control_toolbox::exponential_filter )
list(APPEND _cmake_import_check_files_for_control_toolbox::exponential_filter "${_IMPORT_PREFIX}/lib/libexponential_filter.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
