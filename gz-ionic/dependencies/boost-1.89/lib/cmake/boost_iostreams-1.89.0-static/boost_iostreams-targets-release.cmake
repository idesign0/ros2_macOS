#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Boost::iostreams" for configuration "Release"
set_property(TARGET Boost::iostreams APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Boost::iostreams PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libboost_iostreams.a"
  )

list(APPEND _cmake_import_check_targets Boost::iostreams )
list(APPEND _cmake_import_check_files_for_Boost::iostreams "${_IMPORT_PREFIX}/lib/libboost_iostreams.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
