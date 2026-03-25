#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Boost::stacktrace_addr2line" for configuration "Release"
set_property(TARGET Boost::stacktrace_addr2line APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Boost::stacktrace_addr2line PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libboost_stacktrace_addr2line.a"
  )

list(APPEND _cmake_import_check_targets Boost::stacktrace_addr2line )
list(APPEND _cmake_import_check_files_for_Boost::stacktrace_addr2line "${_IMPORT_PREFIX}/lib/libboost_stacktrace_addr2line.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
