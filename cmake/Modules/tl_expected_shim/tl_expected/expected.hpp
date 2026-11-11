// Shim header: several ROS-ecosystem sources (autoware, point_cloud_transport,
// draco_point_cloud_transport, feetech_ros2_driver, ...) `#include
// <tl_expected/expected.hpp>` (underscore dir), matching the layout of
// PickNickRobotics/cpp_polyfills' vendored `tl_expected` ament package. On
// distros that resolve `tl_expected` via Homebrew's `tl-expected` formula
// instead (see Findtl_expected.cmake), the real header installs at
// `tl/expected.hpp` (upstream TartanLlama/expected's own layout) -> redirect.
#pragma once
#include <tl/expected.hpp>
