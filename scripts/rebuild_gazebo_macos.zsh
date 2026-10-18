#!/bin/zsh

set -e

SCRIPT_DIR="${0:A:h}"
SRC="${SCRIPT_DIR:h}"
WS="${SRC:h}"
PATCH_DIR="$SRC/patches/gazebo-macos"

PYTHON="$(which python3.11)"
EIGEN_DIR="$(brew --prefix eigen@3)/share/eigen3/cmake"

export PATH="$(brew --prefix ruby)/bin:$PATH"

export CMAKE_PREFIX_PATH="\
$(brew --prefix eigen@3):\
$(brew --prefix qt@5):\
$(brew --prefix protobuf):\
${CMAKE_PREFIX_PATH:-}"

apply_patch()
{
  local repo="$1"
  local patch="$2"

  if git -C "$repo" apply --reverse --check "$patch" 2>/dev/null; then
    echo "Patch already applied: $(basename "$patch")"
  elif git -C "$repo" apply --check "$patch"; then
    git -C "$repo" apply "$patch"
    echo "Applied: $(basename "$patch")"
  else
    echo "Unable to apply: $patch"
    return 1
  fi
}

apply_patch \
  "$SRC/gazebo-release/gz_sim_vendor" \
  "$PATCH_DIR/gz_sim_vendor.patch"

apply_patch \
  "$SRC/gazebo-release/gz_gui_vendor" \
  "$PATCH_DIR/gz_gui_vendor.patch"

apply_patch \
  "$SRC/gazebo-release/gz_msgs_vendor" \
  "$PATCH_DIR/gz_msgs_vendor.patch"

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
  echo "Building $pkg"

  rm -rf "$WS/build/$pkg"
  rm -rf "$WS/install/opt/$pkg"

  colcon build \
    --merge-install \
    --packages-select "$pkg" \
    --cmake-args \
      -DBUILD_TESTING=OFF \
      -DPython3_EXECUTABLE="$PYTHON" \
      -DEigen3_DIR="$EIGEN_DIR"

  source "$WS/install/local_setup.zsh"
done

echo "Replacing obsolete gz-harmonic references"

find "$WS/install" -type f -print0 |
while IFS= read -r -d '' target; do
  file "$target" | grep -q 'Mach-O' || continue

  otool -L "$target" 2>/dev/null |
  awk '/\/gz-harmonic\/install\// {print $1}' |
  while read -r old_dependency; do
    library_name="$(basename "$old_dependency")"

    replacement="$(find "$WS/install" \
      -name "$library_name" \
      -print -quit 2>/dev/null)"

    if [[ -n "$replacement" ]]; then
      install_name_tool \
        -change "$old_dependency" \
        "@rpath/$library_name" \
        "$target"
    else
      echo "Dependency not found: $library_name"
      exit 1
    fi
  done
done

echo "Adding runtime library paths"

lib_dirs=("$WS/install/lib")

for dir in "$WS"/install/opt/*/lib; do
  lib_dirs+=("$dir")
done

find "$WS/install" -type f -print0 |
while IFS= read -r -d '' target; do
  file "$target" | grep -q 'Mach-O' || continue

  for dir in "${lib_dirs[@]}"; do
    if ! otool -l "$target" 2>/dev/null |
         grep -Fq "path $dir "; then
      install_name_tool \
        -add_rpath "$dir" \
        "$target" 2>/dev/null || true
    fi
  done
done

ENV_SETUP="$WS/install/gazebo_macos_setup.zsh"

cat > "$ENV_SETUP" <<'EOF'
for bin_dir in "$HOME"/ros2-jazzy/install/opt/*/bin; do
  export PATH="$bin_dir:$PATH"
done

gz_config_paths=()

for gz_dir in "$HOME"/ros2-jazzy/install/opt/*/share/gz; do
  gz_config_paths+=("$gz_dir")
done

export GZ_CONFIG_PATH="${(j/:/)gz_config_paths}"
export PATH="$(brew --prefix ruby)/bin:$PATH"
EOF

SETUP="$WS/install/setup.zsh"
HOOK="source \"$ENV_SETUP\""

if ! grep -Fq "$HOOK" "$SETUP"; then
  print -- "$HOOK" >> "$SETUP"
fi

echo "Gazebo for ROS 2 rebuilt successfully."