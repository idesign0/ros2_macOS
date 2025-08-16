#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "filters::mean" for configuration ""
set_property(TARGET filters::mean APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(filters::mean PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmean.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libmean.dylib"
  )

list(APPEND _cmake_import_check_targets filters::mean )
list(APPEND _cmake_import_check_files_for_filters::mean "${_IMPORT_PREFIX}/lib/libmean.dylib" )

# Import target "filters::params" for configuration ""
set_property(TARGET filters::params APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(filters::params PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libparams.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libparams.dylib"
  )

list(APPEND _cmake_import_check_targets filters::params )
list(APPEND _cmake_import_check_files_for_filters::params "${_IMPORT_PREFIX}/lib/libparams.dylib" )

# Import target "filters::increment" for configuration ""
set_property(TARGET filters::increment APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(filters::increment PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libincrement.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libincrement.dylib"
  )

list(APPEND _cmake_import_check_targets filters::increment )
list(APPEND _cmake_import_check_files_for_filters::increment "${_IMPORT_PREFIX}/lib/libincrement.dylib" )

# Import target "filters::median" for configuration ""
set_property(TARGET filters::median APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(filters::median PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmedian.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libmedian.dylib"
  )

list(APPEND _cmake_import_check_targets filters::median )
list(APPEND _cmake_import_check_files_for_filters::median "${_IMPORT_PREFIX}/lib/libmedian.dylib" )

# Import target "filters::transfer_function" for configuration ""
set_property(TARGET filters::transfer_function APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(filters::transfer_function PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libtransfer_function.dylib"
  IMPORTED_SONAME_NOCONFIG "@rpath/libtransfer_function.dylib"
  )

list(APPEND _cmake_import_check_targets filters::transfer_function )
list(APPEND _cmake_import_check_files_for_filters::transfer_function "${_IMPORT_PREFIX}/lib/libtransfer_function.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
