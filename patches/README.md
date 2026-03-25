# ROS 2 Kilted — macOS Apple Silicon Patches

These patches enable building the full ROS 2 Kilted workspace on
**macOS Apple Silicon (arm64)** with Homebrew Eigen 5, Apple Clang 16,
and Gazebo Ionic.

## Quick start

```bash
cd /path/to/ros2-kilted   # workspace root
./patches/apply-patches.sh
```

## Patch inventory

| Patch file | Package | Category | License | Upstream repo |
|---|---|---|---|---|
| `toolchain-macos-eigen5-asio.patch` | *(workspace toolchain)* | Toolchain | Apache-2.0 | — |
| `sophus-eigen5-version-range.patch` | Sophus | Eigen 5 | MIT | [strasdat/Sophus](https://github.com/strasdat/Sophus) |
| `rko_lio-eigen5-version-range.patch` | rko_lio | Eigen 5 | MIT | [ROKEY-SPARK/rko_lio](https://github.com/ROKEY-SPARK/rko_lio) |
| `robotraconteur-eigen5-version-range.patch` | robotraconteur_companion | Eigen 5 | Apache-2.0 | [robotraconteur/robotraconteur_companion](https://github.com/robotraconteur/robotraconteur_companion) |
| `gtsam-system-eigen5.patch` | GTSAM | Eigen 5 | BSD-3-Clause | [borglab/gtsam](https://github.com/borglab/gtsam) |
| `grid_map-eigen5-compat.patch` | grid_map | Eigen 5 | BSD-3-Clause | [ANYbotics/grid_map](https://github.com/ANYbotics/grid_map) |
| `aruco_ros-eigen5-cpp17.patch` | aruco_ros | Eigen 5 / C++17 | MIT | [pal-robotics/aruco_ros](https://github.com/pal-robotics/aruco_ros) |
| `apriltag_ros-eigen5-macos.patch` | apriltag_ros | Eigen 5 | MIT | [christianrauch/apriltag_ros](https://github.com/christianrauch/apriltag_ros) |
| `eiquadprog-macos-cassert.patch` | eiquadprog | Apple Clang cassert | LGPL-3.0 | [stack-of-tasks/eiquadprog](https://github.com/stack-of-tasks/eiquadprog) |
| `steering_functions-macos-cassert.patch` | steering_functions | Apple Clang cassert | Apache-2.0 | [hbanzhaf/steering_functions](https://github.com/hbanzhaf/steering_functions) |
| `rmf_traffic-macos-cassert-eigen5.patch` | rmf_traffic | Apple Clang cassert / Eigen 5 | Apache-2.0 | [open-rmf/rmf_traffic](https://github.com/open-rmf/rmf_traffic) |
| `libnabo-macos-cpp17-cassert.patch` | libnabo | Apple Clang cassert / C++17 | BSD-3-Clause | [norlab-ulaval/libnabo](https://github.com/norlab-ulaval/libnabo) |
| `cartographer-macos-eigen5.patch` | Cartographer | Linker / Eigen 5 | Apache-2.0 | [cartographer-project/cartographer](https://github.com/cartographer-project/cartographer) |
| `etsi_its_messages-geographiclib-export.patch` | etsi_its_messages | Linker | MIT | [ika-rwth-aachen/etsi_its_messages](https://github.com/ika-rwth-aachen/etsi_its_messages) |
| `moveit-trac_ik-target-fix.patch` | MoveIt 2 | Linker | BSD-3-Clause | [moveit/moveit2](https://github.com/moveit/moveit2) |
| `ublox-asio-iocontext.patch` | ublox | Linker / ASIO | BSD | [KumarRobotics/ublox](https://github.com/KumarRobotics/ublox) |
| `rtabmap-g2o-csparse-guard.patch` | RTAB-Map | Linker | BSD-3-Clause | [introlab/rtabmap](https://github.com/introlab/rtabmap) |
| `stomp-remove-boilerplate-dep.patch` | STOMP | CMake | Apache-2.0 | [ros-industrial/stomp](https://github.com/ros-industrial/stomp) |
| `pybind11_json_vendor-cmake-policy.patch` | pybind11_json_vendor | CMake policy | Apache-2.0 | [ros2/pybind11_json_vendor](https://github.com/ros2/pybind11_json_vendor) |
| `rsl-tl-expected-fix.patch` | RSL | CMake / vendor | BSD-3-Clause | [PickNikRobotics/RSL](https://github.com/PickNikRobotics/RSL) |
| `gazebo-release-dartsim-eigen5-cassert.patch` | gz_dartsim_vendor | Eigen 5 / cassert | Apache-2.0 | [gazebo-release/gz_dartsim_vendor](https://github.com/gazebo-release/gz_dartsim_vendor) |
| `ouster-ros-spdlog-sophus.patch` | ouster-ros | spdlog / Sophus | BSD-3-Clause | [ouster-lidar/ouster-ros](https://github.com/ouster-lidar/ouster-ros) |

## License

Each patch is a derivative work of its upstream package and is distributed
under the **same license as the original package** (see table above).

All upstream licenses permit modification and redistribution. The LGPL-3.0
patch (`eiquadprog`) is the most restrictive — it requires that derivative
works also be made available under LGPL-3.0 or a compatible license. This
patch modifies only build system files and adds a standard C++ header include,
so it qualifies as a minor/trivial modification.

The `toolchain-macos-eigen5-asio.patch` and the `apply-patches.sh` script
are original works licensed under **Apache-2.0**, consistent with the
ROS 2 project.

## Authors

- Patches created by **Frederik Schulze** ([@frederikschulze1701-blip](https://github.com/frederikschulze1701-blip))
- Original packages by their respective maintainers (see upstream links above)

## Compatibility

- macOS 14+ (Sonoma) / macOS 15+ (Sequoia) on Apple Silicon (arm64)
- Homebrew Eigen 5.0.x
- Apple Clang 16+
- ROS 2 Kilted Kaiju (2025)
- Gazebo Ionic
