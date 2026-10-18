#!/bin/zsh

set -e

WS="$HOME/ros2-jazzy"
PYTHON="$(which python3.11)"
EIGEN_DIR="$(brew --prefix eigen@3)/share/eigen3/cmake"

export PATH="$(brew --prefix ruby)/bin:$PATH"

export CMAKE_PREFIX_PATH="\
$(brew --prefix eigen@3):\
$(brew --prefix qt@5):\
$(brew --prefix protobuf):\
$CMAKE_PREFIX_PATH"

cd "$WS"

packages=(
  gz_utils_vendor
  gz_plugin_vendor
  gz_msgs_vendor
  gz_transport_vendor
  sdformat_vendor
  gz_fuel_tools_vendor
  gz_gui_vendor
  gz_physics_vendor
  gz_sensors_vendor
  gz_sim_vendor
  gz_tools_vendor
)

for pkg in "${packages[@]}"; do
  rm -rf "$WS/build/$pkg"

  colcon build \
    --merge-install \
    --packages-select "$pkg" \
    --cmake-args \
      -DBUILD_TESTING=OFF \
      -DPython3_EXECUTABLE="$PYTHON" \
      -DEigen3_DIR="$EIGEN_DIR"

  source "$WS/install/setup.zsh"
done

lib_dirs=("$WS/install/lib")

for dir in "$WS"/install/opt/*/lib; do
  lib_dirs+=("$dir")
done

find "$WS/install" -type f -print0 |
while IFS= read -r -d '' target; do
  if file "$target" | grep -q 'Mach-O'; then
    for dir in "${lib_dirs[@]}"; do
      if ! otool -l "$target" 2>/dev/null | grep -Fq "path $dir "; then
        install_name_tool -add_rpath "$dir" "$target" 2>/dev/null || true
      fi
    done
  fi
done

echo "Gazebo para ROS 2 reconstruido correctamente."