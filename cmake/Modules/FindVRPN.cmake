# FindVRPN.cmake — vrpn_mocap does find_package(VRPN REQUIRED); brew vrpn ships no
# CMake config and vrpn_mocap bundles no module. Provide VRPN_INCLUDE_DIRS/LIBRARIES
# (vrpn also needs quat). macOS: headers at include root, libs libvrpn + libquat.
find_path(VRPN_INCLUDE_DIRS NAMES vrpn_Tracker.h)
find_library(VRPN_vrpn_LIBRARY NAMES vrpn)
find_library(VRPN_quat_LIBRARY NAMES quat)
set(VRPN_LIBRARIES ${VRPN_vrpn_LIBRARY})
if(VRPN_quat_LIBRARY)
  list(APPEND VRPN_LIBRARIES ${VRPN_quat_LIBRARY})
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRPN DEFAULT_MSG VRPN_vrpn_LIBRARY VRPN_INCLUDE_DIRS)
mark_as_advanced(VRPN_INCLUDE_DIRS VRPN_vrpn_LIBRARY VRPN_quat_LIBRARY)
