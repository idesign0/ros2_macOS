#!/bin/zsh

# ==============================================================================
# CONFIGURATION
# ==============================================================================
DISTRO="kilted" # "kilted" or "humble"

MAIN_REPO_URL="https://github.com/idesign0/ros2_macOS.git"
GZ_REPO_URL="https://github.com/idesign0/gz-macOS.git"
# Get CPU count once to avoid syntax errors in loops
NPROC=$(sysctl -n hw.ncpu)

if [ "$DISTRO" = "kilted" ]; then
    MAIN_BRANCH="kilted"
    GZ_BRANCH="ionic"
else
    MAIN_BRANCH="humble"
    GZ_BRANCH="harmonic"
fi

ROS2_WORKSPACE="ros2-$DISTRO"
GZ_WORKSPACE="gz-$GZ_BRANCH"

# Path setup for Framework Python 3.11
export PATH="/Library/Frameworks/Python.framework/Versions/3.11/bin:$HOME/Library/Python/3.11/bin:$PATH"

# ==============================================================================

set -e

# Architecture check (moved here from after the build)
ARCH=$(uname -m)
if [ "$ARCH" != "arm64" ]; then
  echo "This script is intended for Apple Silicon (arm64)."
  exit 1
fi

ROOT_DIR=$(pwd)

# --- Sudo Check ---
# We ONLY use sudo for the 'ln' command later.
# Try to cache the password; continue without sudo if unavailable.
HAVE_SUDO=0
if sudo -v 2>/dev/null; then
  HAVE_SUDO=1
  echo "Sudo credentials cached."
else
  echo "WARNING: No sudo access. Symlink/chown steps will be skipped."
fi

echo "Starting Full Local Setup for macOS $DISTRO (Apple Silicon)..."

# 1. Clone Repositories
if [ ! -d "$ROS2_WORKSPACE" ]; then
    echo "Cloning main repository..."
    mkdir -p "$ROS2_WORKSPACE"
    git clone -b $MAIN_BRANCH $MAIN_REPO_URL "$ROS2_WORKSPACE/src" --recurse-submodules
fi

if [ ! -d "$GZ_WORKSPACE" ]; then
    echo "Cloning Gazebo repository..."
    mkdir -p "$GZ_WORKSPACE"
    git clone -b $GZ_BRANCH $GZ_REPO_URL "$GZ_WORKSPACE/src" --recurse-submodules
fi

# 2. Sync Submodules
if [ -f "$ROOT_DIR/$ROS2_WORKSPACE/src/scripts/sync-submodules-branches.sh" ]; then
    echo "Syncing submodules..."
    chmod +x "$ROOT_DIR/$ROS2_WORKSPACE/src/scripts/sync-submodules-branches.sh"
    cd "$ROOT_DIR/$ROS2_WORKSPACE/src" && ./scripts/sync-submodules-branches.sh
    cd "$ROOT_DIR"
fi

# 3. Homebrew & Python Dependencies
echo "Installing Homebrew packages (Running as $(whoami))..."
brew install graphviz eigen@3 eigen GeographicLib openssl@3 qt@5 cmake wget python@3.11 glog

BREW_SCRIPT="$ROOT_DIR/$ROS2_WORKSPACE/src/brew-packages/install_brew_packages.sh"
if [ -f "$BREW_SCRIPT" ]; then
    echo "Running brew installation script..."
    chmod +x "$BREW_SCRIPT"
    "$BREW_SCRIPT"
else
    echo "Custom brew script not found at $BREW_SCRIPT, skipping."
fi

echo "Creating Eigen3 symlink..."
if [ ! -e /opt/homebrew/include/eigen3 ]; then
    if [ "$HAVE_SUDO" = "1" ]; then
        sudo ln -sfn /opt/homebrew/opt/eigen@3/include/eigen3 /opt/homebrew/include/eigen3
    else
        ln -sfn /opt/homebrew/opt/eigen@3/include/eigen3 /opt/homebrew/include/eigen3 2>/dev/null || echo "WARNING: Could not create eigen3 symlink (no sudo). Create it manually if needed."
    fi
else
    echo "Eigen3 symlink already exists, skipping."
fi

PYTHON="/Library/Frameworks/Python.framework/Versions/3.11/bin/python3"
echo "Installing Python dependencies..."

if [ ! -x "$PYTHON" ]; then
  echo "Python 3.11 not found at $PYTHON"
  exit 1
fi

"$PYTHON" -m pip install --upgrade pip wheel
"$PYTHON" -m pip install "setuptools>=61.0,<70.0"
"$PYTHON" -m pip install -U \
  --config-settings="--global-option=build_ext" \
  --config-settings="--global-option=-I$(brew --prefix graphviz)/include/" \
  --config-settings="--global-option=-L$(brew --prefix graphviz)/lib/" \
  argcomplete catkin_pkg colcon-common-extensions coverage \
  cryptography empy==3.3.4 flake8 flake8-blind-except==0.1.1 flake8-builtins \
  flake8-class-newline flake8-comprehensions flake8-deprecated \
  flake8-docstrings flake8-import-order flake8-quotes \
  importlib-metadata lark==1.1.1 lxml matplotlib mock mypy==0.931 netifaces \
  nose pep8 psutil pydocstyle pydot pygraphviz pyparsing==2.4.7 \
  pytest-mock rosdep rosdistro vcstool typeguard

# 4. Build Gazebo Workspace
echo "Building Gazebo ($GZ_BRANCH)..."
cd "$ROOT_DIR/$GZ_WORKSPACE"

# NO SUDO HERE - brew unlink must run as user
brew unlink protobuf || true

colcon build --packages-select protobuf --executor parallel \
  --parallel-workers "$NPROC" \
  --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release \
  -DBOOST_ROOT="$(pwd)/src/dependencies/boost-1.89" \
  --merge-install --continue-on-error

colcon build --packages-ignore protobuf --executor parallel \
  --parallel-workers "$NPROC" \
  --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release \
  -DBOOST_ROOT="$(pwd)/src/dependencies/boost-1.89" \
  -DCMAKE_MACOSX_RPATH=FALSE -DCMAKE_INSTALL_NAME_DIR="$(pwd)/install/lib" \
  --merge-install --continue-on-error

# 5. Build Main Workspace
echo "Finalizing Full Workspace Build..."
cd "$ROOT_DIR/$ROS2_WORKSPACE"

echo "Unlinking conflicting packages (Running as $(whoami))..."
# NO SUDO HERE
brew unlink boost boost-python3 xtensor xsimd xtl qt asio orocos-kdl protobuf ceres-solver osqp || true

# Export build environment
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export OPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
export GZ_INSTALL_DIR="$ROOT_DIR/$GZ_WORKSPACE/install"
export CMAKE_PREFIX_PATH="$(brew --prefix qt@5):$GZ_INSTALL_DIR:$CMAKE_PREFIX_PATH"
export DYLD_LIBRARY_PATH="$GZ_INSTALL_DIR/lib:$DYLD_LIBRARY_PATH"
export PATH="$GZ_INSTALL_DIR/bin:$PATH"
export GZ_VERSION=$GZ_BRANCH

colcon build \
  --packages-ignore qt_gui_cpp rqt_gui_cpp \
  --executor parallel \
  --parallel-workers "$NPROC" \
  --cmake-args -DCMAKE_TOOLCHAIN_FILE=$(pwd)/src/cmake/toolchain.cmake \
  --continue-on-error \
  --merge-install

# Fix ownership if someone accidentally used sudo previously
if [ "$HAVE_SUDO" = "1" ]; then
    echo "Ensuring user ownership of current directory..."
    sudo chown -R $(whoami) "$ROOT_DIR/$ROS2_WORKSPACE" "$ROOT_DIR/$GZ_WORKSPACE" 2>/dev/null || true
fi

echo "--------------------------------------------------"
echo "Setup Complete!"
echo "Run the following to use your workspace:"
echo "source $ROOT_DIR/$ROS2_WORKSPACE/install/setup.bash"
echo "--------------------------------------------------"
