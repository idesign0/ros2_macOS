# ROS 2 Kilted + MoveIt2 + Gazebo Ionic Setup on macOS (Apple Silicon)

This repository provides a streamlined setup for running **ROS 2 Humble** and **Gazebo Sim Ionic** on **macOS with Apple Silicon (M1/M2/M3)**. ROS 2 is built from source with macOS patches, and Gazebo is built from source.

---

## ✅ What's Included

This repository provides a full build of **ROS 2 Kilted** from source for **ARM64 macOS**, including core packages and key frameworks such as:

- **ament** build system  
- **backward_ros** for stack tracing  
- **eProsima** middleware components  
- **gazebo-release** simulation packages  
- **gzsim** components, including `ros2_gz_bridge`, `gz_ros2_control`, and related packages  
- **osrf** organization packages  
- **ros-perception**, including:
  - PCL perception tools
- **ros-planning**, including:
  - `navigation_msgs`, `moveit2`, `SLAM_toolbox`, `spatio_temporal_voxel_layer`
- **ros-teleop** tools  
- **ros-tooling** utilities  
- **ros-visualization** tools  
- Core **ros** and **ros2** packages  
- **ros2_control** framework  
- **rviz** visualization tool  
- **moveit2** (dedicated README inside `moveit/` for testing and debugging)  
- **nav2** (dedicated README inside `ros-planning/` for testing and debugging)  
- **SLAM_toolbox** (`ros-planning/`)  

Additionally, this setup includes:  
- **Gazebo Ionic** built from source.  
- macOS-specific fixes and configurations  
- A clean, tested installation process and environment 

---

## 📦 ROS 2 macOS Prerequisites

This guide will walk you through:

- Setting up Homebrew
- Installing Python 3.11 and core dependencies
- Installing required build tools (`colcon`, `vcstool`, etc.)
- Creating the ROS 2 workspace structure

### 1️⃣ Setup ROS 2 Workspace and Clone Repository

First, create your ROS 2 workspace and clone this repository into the `src` folder:

```bash
mkdir -p ~/kilted-ros2/src
cd ~/kilted-ros2/src
git clone -b kilted https://github.com/idesign0/ros2_macOS.git .
```
After cloning, run:
```bash
git submodule sync --recursive
git submodule update --init --recursive
```
> ⚠️ **Note:**
> if you are using VSCode keep Git: Repository Scan Max Dept to -1 so all submodules get fetch and you can see them in Source control.
### 2️⃣ Install Homebrew Packages  

If you don’t already have Homebrew installed (needed to install more dependencies), follow the instructions at:  
https://brew.sh/

Optional: After installing, check your system health with:

```bash
brew doctor
```

and now, Go back to the root of your workspace and run the Homebrew packages installation script:

```bash
cd ~/kilted-ros2
./src/brew-packages/install_brew_packages.sh
```
This script installs essential tools and libraries needed for building ROS 2 Kilted on macOS ARM64.

You can verify or manually install additional required packages with this command (these should already be installed by the script, but it’s good to double-check):

```bash
brew install asio assimp bison bullet cmake console_bridge cppcheck \
cunit eigen freetype graphviz opencv openssl orocos-kdl pcre poco \
pyqt@5 python qt@5 sip spdlog tinyxml2
```

unlink some brew packages:
```bash
brew unlink boost boost-pytnon3 xtensor xdm xtl qt osqp orocos-kdl asio
```

#### Eigen3 Symlinks (Critical):

Currently, Homebrew installs Eigen into versioned folders (like `eigen(5)` or `eigen@3`). However, the majority of ROS 2 and MoveIt packages are hard-coded to look for a directory named exactly `eigen3`. 

To ensure a smooth build and avoid "Eigen/Core not found" errors, you **must** create a symbolic link to map the Homebrew installation to the path expected by the compiler.

```bash
# Link versioned eigen@3 to the 'eigen3' path expected by ROS packages
sudo ln -sfn /opt/homebrew/opt/eigen@3/include/eigen3 /opt/homebrew/include/eigen3
```

### 3️⃣ Official ROS 2 macOS Prerequisites

  This section covers the essential setup steps to prepare your macOS environment for building ROS 2 Kilted.
  
  #### 1️⃣ Setup Some Environment Variables
  
  To ensure ROS 2 and its dependencies work correctly on your macOS system, add the following environment variables and aliases to your shell configuration (`~/.zshrc` or `~/.bash_profile`):
  
  ```bash
# --- Load completion system early ---
autoload -Uz compinit
compinit

# Minimum required CMake policy version and C++ standard
export CMAKE_POLICY_VERSION_MINIMUM=3.5

# Qt 5 paths (Homebrew)
export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:$(brew --prefix qt@5)"
export PATH="$PATH:$(brew --prefix qt@5)/bin"

# Python 3.11 framework path
export PATH="/Library/Frameworks/Python.framework/Versions/3.11/bin:$HOME/Library/Python/3.11/bin:$PATH"

# Python pip aliases for convenience
alias pip3.10="python3.10 -m pip"
alias pip3.11="python3.11 -m pip"

# OpenSSL root directory (Homebrew)
export OPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl@3

# Gazebo ionic variables
export GZ_VERSION=ionic
export GZ_SIM_SYSTEM_PLUGIN_PATH=~/kilted-ros2/install/lib/
export GZ_BUILD_FROM_SOURCE=1
export GZ_RELAX_VERSION_MATCH=1

# sourcing
#source ~/gz-ionic/install/setup.zsh
#source ~/kilted-ros2/install/setup.zsh
#source ~/ros2_ws/install/setup.zsh

# Set the desired RMW implementation for the ROS 2 build environment.
# Alternative common option: export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI='<CycloneDDS><Domain><Discovery><ParticipantIndex>none</ParticipantIndex></Discovery></Domain></CycloneDDS>'

# Enable Python argcomplete for colcon
eval "$(register-python-argcomplete colcon)"
export PATH="$HOME/bin:$PATH"
```
> ***Recommendation:***  
  Select the DDS implementation based on the primary workload:
> - Use **CycloneDDS** when running **Navigation2** (Legacy Support)
> - Use **Fast DDS** when working with **MoveIt 2** and **Gazebo ROS**
  #### 2️⃣ Install Additional Python Packages
  
  Use `python3 -m pip` (instead of just `pip`) to avoid confusion between Python 2 and Python 3 installations.
  
  Please make sure you are usign Python@3.11:
  
  ```bash
which python3
>> /Library/Frameworks/Python.framework/Versions/3.11/bin/python3

which pip
>> /Library/Frameworks/Python.framework/Versions/3.11/bin/pip
  ```
  First, upgrade `pip`:
  
  ```bash
  python3 -m pip install --upgrade pip
  ```
  Then, install the required Python packages with the appropriate build flags and paths:
  ```bash
  python3 -m pip install -U \
    --config-settings="--global-option=build_ext" \
    --config-settings="--global-option=-I$(brew --prefix graphviz)/include/" \
    --config-settings="--global-option=-L$(brew --prefix graphviz)/lib/" \
    argcomplete catkin_pkg colcon-common-extensions coverage \
    cryptography empy==3.3.4 flake8 flake8-blind-except==0.1.1 flake8-builtins \
    flake8-class-newline flake8-comprehensions flake8-deprecated \
    flake8-docstrings flake8-import-order flake8-quotes \
    importlib-metadata lark==1.1.1 lxml matplotlib mock mypy==0.931 netifaces \
    nose pep8 psutil pydocstyle pydot pygraphviz pyparsing==2.4.7 \
    pytest-mock rosdep rosdistro setuptools==59.6.0 vcstool typeguard 
  ```
  ---
  
  If you encounter issues related to dynamic library loading, you may need to temporarily **disable SIP**.  
  Please refer to Apple’s official documentation or the ROS 2 macOS setup guide for instructions on disabling and re-enabling SIP safely. [ROS 2 macOS Setup - Disable SIP](https://developer.apple.com/library/archive/documentation/Security/Conceptual/System_Integrity_Protection_Guide/ConfiguringSystemIntegrityProtection/ConfiguringSystemIntegrityProtection.html)
  
  ---

## 🛠️ Installation Steps

Follow these steps to install Gazebo Ionic and build ROS 2 from source.

### 1. Install Gazebo Ionic (from source)

Gazebo Ionic is built **from source** on macOS to avoid dependency and ABI mismatches introduced by Homebrew—most notably **protobuf**.

Clone and build Gazebo Ionic using the instructions provided in the following repository:

👉 https://github.com/idesign0/gz-macOS/tree/ionic

This repository includes:
- Gazebo Ionic and required GZ libraries as submodules
- A source-built protobuf version compatible with `gz-msgs11`
- macOS-specific patches and configuration for Apple Silicon

> **Note**  
> Homebrew currently installs a newer protobuf version that is ABI-incompatible with Gazebo Ionic. This causes build failures in `gz-msgs11`, `gz-fuel-tools10`, and `ros_gz_bridge`. Building everything from source ensures a consistent protobuf runtime across the entire stack.

### 2. 🔨 Build
```bash
cd ~/kilted-ros2
```
> ⚠️ **Note:**  
> The build process might require several attempts to complete successfully,  
> as you may encounter some common errors — mostly related to `CMAKE_PREFIX_PATH`.  
> Please refer to the **Troubleshooting** section below for specific instructions based on the error type.

```bash
colcon build \
  --packages-ignore qt_gui_cpp rqt_gui_cpp \
  --executor parallel \
  --parallel-workers $(sysctl -n hw.ncpu) \
  --cmake-args -DCMAKE_TOOLCHAIN_FILE=$(pwd)/src/cmake/toolchain.cmake \
  --continue-on-error \
  --merge-install
```
source:
```bash
source ~/.zshrc
```

What does each option mean?
- `--packages-ignore qt_gui_cpp rqt_gui_cpp python_orocos_kdl_vendor`  
  Skips these two packages known to have macOS issues. See: [ros2/ros2#1139](https://github.com/ros2/ros2/issues/1139)

- `--executor parallel`  
  Runs build tasks in parallel.

- `--parallel-workers $(sysctl -n hw.ncpu)`  
  Sets the number of parallel jobs to your CPU core count (maximizing build speed).

- `--merge-install`
  Installs all packages into a single merged install space instead of separate per-package directories. This simplifies library discovery and linking on macOS and avoids RPATH and dependency resolution issues, especially when building large stacks like ROS 2 and Gazebo.

- `--continue-on-error`
  Continues building remaining packages even if one package fails, allowing you to identify multiple build issues in a single run instead of stopping at the first failure.

---

### 3. ⚠️ Troubleshooting
Most of the source-related errors are fixed in the macOS-specific source code patches, but there are still some dependency errors which might interrupt a complete build.  
Right now, I have no issues completing the build with more than **499** packages, but as mentioned earlier, it depends on which dependencies are missing on individual Macs.

This section will be constantly updated based on user feedback. Below are some of the most common errors I have faced so far:

### Errors:

1. **ModuleNotFoundError: No module named 'some_library'**
   This error occurs when Python packages are missing during runtime. Simply install the missing python-package with:

   ```bash
   python3 -m pip install some_library
   ```

---

2. **Missing `Config.cmake` Files**

    Sometimes during the build you may encounter errors complaining about missing `SomeLibraryConfig.cmake` files.
    
    **How to check if Homebrew has the required library:**
    
    Use the following command to check info about the library (replace `some_library` with the actual name):
    
    ```bash
    brew info some_library
    ```
    
    If Homebrew shows that the library is not installed, install it with:

   ```bash
    brew install some_library
    ```

    If Homebrew shows the library is already installed, but CMake still can't find it, you need to add the library’s path to your environment:

    ```bash
    export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:$(brew --prefix some_library)"
    ```

    Add this line to your shell config file (e.g., ~/.zshrc or ~/.bash_profile) to make it persistent.
    > **Note:**  
    > Always double-check the exact library name reported in the error message, and use that in the brew info and brew install commands.

---

## ✅ Test Your ROS 2 Installation

After building and setting up your environment, verify your ROS 2 installation by running the basic talker and listener example.

For detailed instructions and examples, please refer to the official ROS 2 Kilted macOS development setup guide:  
[ROS 2 Kilted macOS Development Setup — Talker and Listener Example](https://docs.ros.org/en/kilted/Installation/Alternatives/macOS-Development-Setup.html#id8)
