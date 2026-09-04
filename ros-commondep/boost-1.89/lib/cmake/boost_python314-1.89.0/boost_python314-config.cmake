# ci-boost-python314: versioned python314 component backed by the BREW boost-python3
# (libboost_python314, built vs Python 3.14). Brew eigenpy/pinocchio link libboost_python314,
# so its Boost.Python ABI must be the 3.14 one — the vendored Boost 1.89 only ships python311/313,
# and eigenpy's SEARCH_FOR_BOOST_PYTHON does find_package(Boost COMPONENTS python314) -> fails.
# Point python314 at the brew boost-python3 dylib so the ABI matches brew eigenpy.
if(NOT TARGET Boost::python314)
  add_library(Boost::python314 INTERFACE IMPORTED)
  set_target_properties(Boost::python314 PROPERTIES
    INTERFACE_LINK_LIBRARIES "/opt/homebrew/opt/boost-python3/lib/libboost_python314.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/opt/boost/include")
endif()

set(boost_python314_FOUND FALSE)
if(TARGET Boost::python314)
  set(boost_python314_FOUND TRUE)
endif()
