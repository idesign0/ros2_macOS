# Hand-authored wrapper: versioned numpy311 component backed by the unversioned
# boost_numpy config (the vendored Boost 1.89 is built for Python 3.11).
# Upstream Boost only ships the unversioned 'boost_numpy' config, so
# find_package(Boost COMPONENTS numpy311) fails; this bridges that gap.

# Pin the python variant so only the libboost_numpy311 variant is selected
# (prevents the 3.13 variant from also matching and triggering SEND_ERROR).
if(NOT DEFINED Boost_PYTHON_VERSION)
  set(Boost_PYTHON_VERSION 3.11)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../boost_numpy-1.89.0/boost_numpy-config.cmake")

if(TARGET Boost::numpy AND NOT TARGET Boost::numpy311)
  add_library(Boost::numpy311 INTERFACE IMPORTED)
  set_target_properties(Boost::numpy311 PROPERTIES
    INTERFACE_LINK_LIBRARIES Boost::numpy)
endif()

set(boost_numpy311_FOUND FALSE)
if(TARGET Boost::numpy311)
  set(boost_numpy311_FOUND TRUE)
endif()
