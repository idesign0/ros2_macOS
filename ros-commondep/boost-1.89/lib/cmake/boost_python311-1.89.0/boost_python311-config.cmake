# Hand-authored wrapper: versioned python311 component backed by the unversioned
# boost_python config (the vendored Boost 1.89 is built for Python 3.11).
# Upstream Boost only ships the unversioned 'boost_python' config, so
# find_package(Boost COMPONENTS python311) fails; this bridges that gap.

# Pin the python variant so only the libboost_python311 variant is selected
# (prevents the 3.13 variant from also matching and triggering SEND_ERROR).
if(NOT DEFINED Boost_PYTHON_VERSION)
  set(Boost_PYTHON_VERSION 3.11)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../boost_python-1.89.0/boost_python-config.cmake")

if(TARGET Boost::python AND NOT TARGET Boost::python311)
  add_library(Boost::python311 INTERFACE IMPORTED)
  set_target_properties(Boost::python311 PROPERTIES
    INTERFACE_LINK_LIBRARIES Boost::python)
endif()

set(boost_python311_FOUND FALSE)
if(TARGET Boost::python311)
  set(boost_python311_FOUND TRUE)
endif()
