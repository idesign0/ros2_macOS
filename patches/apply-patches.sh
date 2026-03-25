#!/bin/bash
# apply-patches.sh — Apply all macOS / Eigen 5 compatibility patches
# Run from the ros2-kilted/ workspace root after cloning/syncing submodules.
#
# Usage:  ./patches/apply-patches.sh
#
# These patches enable building the full ROS 2 Kilted workspace on
# macOS Apple Silicon (arm64) with Homebrew Eigen 5, Apple Clang 16,
# and Gazebo Ionic.
#
# License: Apache-2.0 (same as ROS 2)

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$(dirname "$SCRIPT_DIR")/src"

apply_patch() {
  local pkg_dir="$1"
  local patch_file="$2"
  local full_dir="$SRC_DIR/$pkg_dir"

  if [ ! -d "$full_dir" ]; then
    echo "[SKIP] $pkg_dir — directory not found"
    return 0
  fi

  if [ ! -f "$SCRIPT_DIR/$patch_file" ]; then
    echo "[SKIP] $patch_file — patch not found"
    return 0
  fi

  echo "[PATCH] $pkg_dir ← $patch_file"
  cd "$full_dir"
  git apply --whitespace=nowarn "$SCRIPT_DIR/$patch_file" 2>/dev/null || \
    echo "  [WARN] Patch may already be applied or conflicts — skipping"
  cd "$SRC_DIR"
}

echo "=== Applying macOS / Eigen 5 patches ==="
echo "Workspace: $(dirname "$SRC_DIR")"
echo ""

# --- Toolchain ---
apply_patch "."                          "toolchain-macos-eigen5-asio.patch"

# --- Eigen 5 version range fixes ---
apply_patch "sophus"                     "sophus-eigen5-version-range.patch"
apply_patch "rko_lio"                    "rko_lio-eigen5-version-range.patch"
apply_patch "robotraconteur_companion"   "robotraconteur-eigen5-version-range.patch"
apply_patch "gtsam"                      "gtsam-system-eigen5.patch"
apply_patch "grid_map"                   "grid_map-eigen5-compat.patch"
apply_patch "aruco_ros"                  "aruco_ros-eigen5-cpp17.patch"
apply_patch "apriltag_ros"               "apriltag_ros-eigen5-macos.patch"

# --- Apple Clang missing <cassert> ---
apply_patch "eiquadprog"                 "eiquadprog-macos-cassert.patch"
apply_patch "steering_functions"         "steering_functions-macos-cassert.patch"
apply_patch "rmf_traffic"                "rmf_traffic-macos-cassert-eigen5.patch"
apply_patch "libnabo"                    "libnabo-macos-cpp17-cassert.patch"

# --- Linker / dependency fixes ---
apply_patch "cartographer"               "cartographer-macos-eigen5.patch"
apply_patch "etsi_its_messages"          "etsi_its_messages-geographiclib-export.patch"
apply_patch "moveit"                     "moveit-trac_ik-target-fix.patch"
apply_patch "ublox"                      "ublox-asio-iocontext.patch"
apply_patch "rtabmap"                    "rtabmap-g2o-csparse-guard.patch"
apply_patch "stomp"                      "stomp-remove-boilerplate-dep.patch"

# --- Vendor / CMake policy fixes ---
apply_patch "pybind11_json_vendor"       "pybind11_json_vendor-cmake-policy.patch"
apply_patch "rsl"                        "rsl-tl-expected-fix.patch"
apply_patch "gazebo-release"             "gazebo-release-dartsim-eigen5-cassert.patch"
apply_patch "ouster-ros"                 "ouster-ros-spdlog-sophus.patch"

echo ""
echo "=== Done — $(ls "$SCRIPT_DIR"/*.patch | wc -l | tr -d ' ') patches processed ==="
