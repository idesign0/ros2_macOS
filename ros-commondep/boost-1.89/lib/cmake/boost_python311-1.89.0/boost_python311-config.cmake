# Alias config: redirect boost_python311 to boost_python
# The Boost 1.89 installation names the component "python" (generic)
# but cv_bridge and others request "python311" (version-specific).

get_filename_component(_BOOST_CMAKEDIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

set(_bpy311_quiet)
if(boost_python311_FIND_QUIETLY)
  set(_bpy311_quiet QUIET)
endif()

find_package(boost_python ${boost_python311_FIND_VERSION} EXACT CONFIG
  ${_bpy311_quiet}
  HINTS ${_BOOST_CMAKEDIR}
)
unset(_bpy311_quiet)

if(TARGET Boost::python AND NOT TARGET Boost::python311)
  add_library(Boost::python311 INTERFACE IMPORTED)
  set_target_properties(Boost::python311 PROPERTIES
    INTERFACE_LINK_LIBRARIES Boost::python
  )
endif()

set(boost_python311_FOUND ${boost_python_FOUND})
