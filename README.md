# ROS 2 Humble + MoveIt2 + Nav2 + Gazebo Harmonic Setup on macOS (Apple Silicon)

This repository provides a streamlined setup for running **ROS 2 Humble** and **Gazebo Sim Harmonic** on **macOS with Apple Silicon (M1/M2/M3)**. ROS 2 is built from source with macOS patches, and Gazebo is installed via Homebrew.

---

## ✅ What's Included

This repository provides a full build of **ROS 2 Humble** from source for **ARM64 macOS**, including core packages and key frameworks such as:

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
- **Gazebo Harmonic** installed via Homebrew  
- macOS-specific fixes and configurations  
- A clean, tested installation process and environment 

---

## 📦 ROS 2 macOS Prerequisites

This guide will walk you through:

- Setting up Homebrew
- Installing Python 3.11 and core dependencies
- Installing required build tools (`colcon`, `vcstool`, etc.)
- Creating the ROS 2 workspace structure

## ⚡ One-Step Installation (Recommended)

To automate the entire setup—including dependency installation, repository cloning, and the full workspace build—copy and paste the following command into your terminal:

```bash
curl -fsSL https://raw.githubusercontent.com/idesign0/ros2_macOS/humble/scripts/ros2_mac_build.zsh -o build.zsh && chmod +x build.zsh && ./build.zsh
```

## 🛠️ Manual Installation (Step-by-Step)


### 1️⃣ Setup ROS 2 Workspace and Clone Repository

First, create your ROS 2 workspace and clone this repository into the `src` folder:

```bash
mkdir -p ~/humble-ros2/src
cd ~/humble-ros2/src
git clone -b humble https://github.com/idesign0/ros2_macOS.git .
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
cd ~/humble-ros2
./src/brew-packages/install_brew_packages.sh
```
This script installs essential tools and libraries needed for building ROS 2 Humble on macOS ARM64.

You can verify or manually install additional required packages with this command (these should already be installed by the script, but it’s good to double-check):

```bash
brew install asio assimp bison bullet cmake console_bridge cppcheck \
cunit eigen freetype graphviz opencv openssl orocos-kdl pcre poco \
pyqt@5 python qt@5 sip spdlog tinyxml2
```

unlink some brew packages:
```bash
brew unlink boost boost-pytnon3 xtensor xdm xtl qt asio orocos-kdl yaml-cpp
```

#### Eigen3 Symlinks (Critical):

Currently, Homebrew installs Eigen into versioned folders (like `eigen(5)` or `eigen@3`). However, the majority of ROS 2 and MoveIt packages are hard-coded to look for a directory named exactly `eigen3`. 

To ensure a smooth build and avoid "Eigen/Core not found" errors, you **must** create a symbolic link to map the Homebrew installation to the path expected by the compiler.

```bash
# Link versioned eigen@3 to the 'eigen3' path expected by ROS packages
sudo ln -sfn /opt/homebrew/opt/eigen@3/include/eigen3 /opt/homebrew/include/eigen3
```

### 3️⃣ Official ROS 2 macOS Prerequisites

  This section covers the essential setup steps to prepare your macOS environment for building ROS 2 Humble.
  
  #### 1️⃣ Setup Some Environment Variables
  
  To ensure ROS 2 and its dependencies work correctly on your macOS system, add the following environment variables and aliases to your shell configuration (`~/.zshrc` or `~/.bash_profile`):
  
  ```bash
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

# Gazebo Harmonic environment variables
export GZ_VERSION=harmonic
export GZ_SIM_SYSTEM_PLUGIN_PATH=~/humble-ros2/install/gz_ros2_control/lib/

# Source ROS 2 workspace setup script
# source ~/humble-ros2/install/setup.zsh

# Source your overlay workspace setup script
# source ~/ros2_ws/install/setup.zsh

# Enable Python argcomplete for colcon
eval "$(register-python-argcomplete colcon)"
export PATH="$HOME/bin:$PATH"
  ```
  > - **Please uncomment these lines after:**
  >   1. You have installed **Gazebo Harmonic** and verified it works correctly, and you have successfully built all ROS 2 packages without errors.
  >   2. You have created and built your **separate overlay workspace** (`ros2_ws`).
  > 
  > This ensures that your environment is properly configured only once the related components are ready, avoiding errors during the initial setup.
  
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
    pytest-mock rosdep rosdistro setuptools==59.6.0 vcstool typeguard jinja2
  ```
  ---

## 🛠️ Installation Steps

Follow these steps to install Gazebo Harmonic and build ROS 2 from source.

### 1. Install Gazebo Harmonic

Install Gazebo Harmonic using Homebrew by running:

```bash
brew tap osrf/simulation
brew install gz-harmonic
```
> ⚠️ **Note (Xcode compatibility)**  
> Some users have reported installation or build errors when installing **Gazebo Harmonic** with **Xcode 16.2**, typically due to missing or incompatible Metal / rendering toolchain components.
>  
> **Workaround:**  
> - Install and build `gz-harmonic` using the **latest available Xcode**.  
> - After Gazebo is installed successfully, you can **switch back to Xcode 16.2** for building and running **ROS 2**.
>  
> This is not ideal; I will try to make the entire setup work cleanly with the latest Xcode in a future update.

### 2. 🔨 Build
```bash
cd ~/humble-ros2
```
> ⚠️ **Note:**  
> The build process might require several attempts to complete successfully,  
> as you may encounter some common errors — mostly related to `CMAKE_PREFIX_PATH`.  
> Please refer to the **Troubleshooting** section below for specific instructions based on the error type.

```bash
colcon build \
  --packages-ignore qt_gui_cpp rqt_gui_cpp nav2_system_tests  \
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
- `--packages-ignore qt_gui_cpp rqt_gui_cpp nav2_system_tests`  
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

For detailed instructions and examples, please refer to the official ROS 2 Humble macOS development setup guide:  
[ROS 2 Humble macOS Development Setup — Talker and Listener Example](https://docs.ros.org/en/humble/Installation/Alternatives/macOS-Development-Setup.html#id8)

---

## 🚀 Project Demos & Integration Gallery

This section showcases the core capabilities of the workspace, specifically highlighting the successful implementation of ROS 2, MoveIt2, and Nav2 on **macOS**.

### 1. ROS 2 ↔ Gazebo Integration
Seamless communication and synchronization between the ROS 2 transport layer and the Gazebo simulation engine. 

* **Key Features:** Clock synchronization, sensor data bridge, and joint state broadcasting.
* **[View Full Demo on LinkedIn](https://www.linkedin.com/posts/classy29_ros2-ros-robotics-activity-7403848396744110080-IDUQ?utm_source=share&utm_medium=member_desktop&rcm=ACoAACDnI-UBIj7r5BhM-eVhnjDAOHOyP7x5-k8)**

![ros2-gz-demo](https://github.com/user-attachments/assets/6e4eeeaf-ac1a-4bb1-8472-6e8bf16a7792)

---

### 2. MoveIt2 Motion Planning (macOS Native)
Advanced manipulation and motion planning. Achieving stable visualization and execution of MoveIt2 on macOS ARM64/Intel architecture.

* **Key Features:** Inverse Kinematics (IK) solving, collision checking, and trajectory execution.
* **[View Full Demo on LinkedIn](https://www.linkedin.com/posts/classy29_ros2-ros-moveit2-activity-7404628200003194885-OY06?utm_source=share&utm_medium=member_desktop&rcm=ACoAACDnI-UBIj7r5BhM-eVhnjDAOHOyP7x5-k8)**

![moveit2-macos](https://github.com/user-attachments/assets/3b57bf14-19c1-4b4b-85c5-b4712c6a6cda)

---

### 3. Nav2 Navigation Stack
Autonomous navigation, global/local pathfinding, and obstacle avoidance visualized in RViz2.

* **Key Features:** Costmap generation, SLAM integration, and behavior tree-based navigation.
* **[View Full Demo on LinkedIn](https://www.linkedin.com/posts/classy29_ros2-ros-navigation2-activity-7409757575837036544-vJtq?utm_source=share&utm_medium=member_desktop&rcm=ACoAACDnI-UBIj7r5BhM-eVhnjDAOHOyP7x5-k8)**

![nav2-stack-macos](https://github.com/user-attachments/assets/6beb3794-2877-425b-999b-b7b592c54500)

---
