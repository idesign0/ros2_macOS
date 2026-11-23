# ci-boost-python314: versioned numpy314 component backed by the BREW boost-python3
# (libboost_numpy314). Matches brew eigenpy/pinocchio's Python 3.14 Boost.Python ABI.
if(NOT TARGET Boost::numpy314)
  add_library(Boost::numpy314 INTERFACE IMPORTED)
  set_target_properties(Boost::numpy314 PROPERTIES
    INTERFACE_LINK_LIBRARIES "/opt/homebrew/opt/boost-python3/lib/libboost_numpy314.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/opt/boost/include")
endif()

set(boost_numpy314_FOUND FALSE)
if(TARGET Boost::numpy314)
  set(boost_numpy314_FOUND TRUE)
endif()
