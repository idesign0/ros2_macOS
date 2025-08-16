# ROS 2 Humble + MoveIt2 + Gazebo Harmonic Setup on macOS (Apple Silicon)

This repository provides a streamlined setup for running **ROS 2 Humble** and **Gazebo Sim Harmonic** on **macOS with Apple Silicon (M1/M2/M3)**. ROS 2 is built from source with macOS patches, and Gazebo is installed via Homebrew.

---

## ✅ What's Included

This repository provides a full build of **ROS 2 Humble** from source for **ARM64 macOS**, including core packages and key frameworks such as:

- **ament** build system  
- **backward_ros** for stack tracing  
- **eProsima** middleware components  
- **gazebo-release** simulation packages  
- **gzsim** components, including `ros2_gz_bridge`, `gz_ros2_control`, and more  
- **osrf** organization packages  
- **ros-drivers** (e.g., `ackermann_msgs`)  
- **ros-perception** packages  
- **ros-planning** (e.g., `navigation_msgs`)  
- **ros-teleop** tools  
- **ros-tooling** utilities  
- **ros-visualization** tools  
- Core **ros** and **ros2** packages  
- **ros2_control** framework  
- **rviz** visualization tool  
- **moveit2** (with a dedicated README inside the `moveit` folder for testing and debugging)
- **navi2** (with a dedicated README inside the `ros-planning` folder for testing and debugging)
- **SLAM_toolbox** ('ros-planning/')

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

### 1️⃣ Setup ROS 2 Workspace and Clone Repository

First, create your ROS 2 workspace and clone this repository into the `src` folder:

```bash
mkdir -p ~/ros2_humble/src
cd ~/ros2_humble/src
git clone https://github.com/idesign0/ros2_macOS.git .
```

### 2️⃣ Install Homebrew Packages  

If you don’t already have Homebrew installed (needed to install more dependencies), follow the instructions at:  
https://brew.sh/

Optional: After installing, check your system health with:

```bash
brew doctor
```

and now, Go back to the root of your workspace and run the Homebrew packages installation script:

```bash
cd ~/ros2_humble
./brew-packages/install_brew_packages.sh
```
This script installs essential tools and libraries needed for building ROS 2 Humble on macOS ARM64.

You can verify or manually install additional required packages with this command (these should already be installed by the script, but it’s good to double-check):

```bash
brew install asio assimp bison bullet cmake console_bridge cppcheck \
cunit eigen freetype graphviz opencv openssl orocos-kdl pcre poco \
pyqt@5 python qt@5 sip spdlog osrf/simulation/tinyxml1 tinyxml2
```

### 3️⃣ Official ROS 2 macOS Prerequisites

  This section covers the essential setup steps to prepare your macOS environment for building ROS 2 Humble.
  
  #### 1️⃣ Install Xcode 16.2
  
  ROS 2 Humble requires **Xcode 16.2** for a successful build.
  
  - Download Xcode 16.2 from the [Apple Developer website](https://xcodereleases.com).  
  - Install Xcode 16.2.  
  - Set Xcode 16.2 as the active developer directory by running:
  
  ```bash
  sudo xcode-select -s /Applications/Xcode_16.2.app/Contents/Developer
  xcodebuild -version
  ```
  Expected Output:
  ```
  Xcode 16.2
  Build version 16C5032a
  ```
  
  #### 2️⃣ Setup Some Environment Variables
  
  To ensure ROS 2 and its dependencies work correctly on your macOS system, add the following environment variables and aliases to your shell configuration (`~/.zshrc` or `~/.bash_profile`):
  
  ```bash
  # Minimum required CMake policy version and C++ standard
  export CMAKE_POLICY_VERSION_MINIMUM=3.5
  export CMAKE_CXX_STANDARD=17
  
  # Qt 5 paths (Homebrew)
  export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:$(brew --prefix qt@5)"
  export PATH="$PATH:$(brew --prefix qt@5)/bin"
  
  # OMPL path (Homebrew)
  export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:$(brew --prefix ompl)"
  
  # Google Benchmark path
  export benchmark_DIR="$(brew --prefix google-benchmark)/lib/cmake/benchmark"
  
  # Generic Homebrew prefixes for CMake to find packages
  export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:/usr/local/opt:/opt/homebrew/opt"
  
  # Python 3.11 framework path
  export PATH="/Library/Frameworks/Python.framework/Versions/3.11/bin:$PATH"
  
  # Python pip aliases for convenience
  alias pip3.10="python3.10 -m pip"
  alias pip3.11="python3.11 -m pip"
  
  # OpenSSL root directory (Homebrew)
  export OPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl@3
  
  # Gazebo Harmonic environment variables
  export GZ_VERSION=harmonic
  # export GZ_SIM_SYSTEM_PLUGIN_PATH= ~/ros2_humble/install/gz_ros2_control/lib/
  
  # Source ROS 2 workspace setup script
  source ~/humble-ros2/install/setup.zsh
  
  # Source your overlay workspace setup script
  # source ~/ros2_ws/install/setup.zsh
  
  # Enable Python argcomplete for colcon
  eval "$(register-python-argcomplete colcon)"
  ```
  > - **Please uncomment these lines after:**
  >   1. You have installed **Gazebo Harmonic** and verified it works correctly, and you have successfully built all ROS 2 packages without errors.
  >   2. You have created and built your **separate overlay workspace** (`ros2_ws`).
  > 
  > This ensures that your environment is properly configured only once the related components are ready, avoiding errors during the initial setup.
  
  #### 3️⃣ Install Additional Python Packages
  
  Use `python3 -m pip` (instead of just `pip`) to avoid confusion between Python 2 and Python 3 installations.
  
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
  
  #### 4️⃣ Disable System Integrity Protection (SIP) if necessary
  
  macOS versions 10.11 and later have System Integrity Protection (SIP) enabled by default,  
  which restricts the use of environment variables like `DYLD_LIBRARY_PATH` that ROS 2 relies on to locate shared libraries during runtime.
  
  ---
  
  ##### Why SIP and ROS 2 don’t always work well together
  
  System Integrity Protection is a macOS security feature designed to prevent unauthorized modification of critical system files and processes—even by root users.
  
  ROS 2 depends on environment variables such as `DYLD_LIBRARY_PATH` to find its dynamic libraries at runtime. However, SIP **blocks these environment variables from being inherited by ROS 2 processes**, causing failures in loading necessary shared libraries.
  
  This results in ROS 2 nodes crashing or failing to launch properly.
  
  Because SIP restricts this dynamic linking behavior, temporarily disabling SIP is often necessary during ROS 2 development on macOS.
  
  ---
  
  If you encounter issues related to dynamic library loading, you may need to temporarily **disable SIP**.  
  Please refer to Apple’s official documentation or the ROS 2 macOS setup guide for instructions on disabling and re-enabling SIP safely. [ROS 2 macOS Setup - Disable SIP](https://developer.apple.com/library/archive/documentation/Security/Conceptual/System_Integrity_Protection_Guide/ConfiguringSystemIntegrityProtection/ConfiguringSystemIntegrityProtection.html)
  
  ---

## 🛠️ Installation Steps

Follow these steps to install Gazebo Harmonic and build ROS 2 from source.

### 1. Install Gazebo Harmonic

Install Gazebo Harmonic using Homebrew by running:

```bash
brew tap osrf/simulation
brew install gz-harmonic
```

### 2. 🔨 Build
```bash
cd ~/ros2_humble
```
> ⚠️ **Note:**  
> The build process might require several attempts to complete successfully,  
> as you may encounter some common errors — mostly related to `CMAKE_PREFIX_PATH`.  
> Please refer to the **Troubleshooting** section below for specific instructions based on the error type.

```bash
colcon build --symlink-install \
  --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=Off \
  --packages-ignore qt_gui_cpp rqt_gui_cpp \
  --executor parallel \
  --parallel-workers $(sysctl -n hw.ncpu)
```
source:
```bash
source ~/.zshrc
```

What does each option mean?
- `--symlink-install`  
  Uses symlinks for installed files instead of copying — useful for faster iterative development.

- `--cmake-args -DCMAKE_BUILD_TYPE=Release`  
  Builds with **Release** optimizations (faster, optimized binaries). You can switch to `Debug` for debug symbols. *I will keep exploring more ways to optimize the build further and am open to suggestions as well.*

- `-DBUILD_TESTING=Off`  
Disables building test packages. This is recommended because some tests (e.g., in controller_manager) use obsolete C++ methods like random_shuffle, which cause build errors on newer compilers.

- `--packages-ignore qt_gui_cpp rqt_gui_cpp`  
  Skips these two packages known to have macOS issues. See: [ros2/ros2#1139](https://github.com/ros2/ros2/issues/1139)

- `--executor parallel`  
  Runs build tasks in parallel.

- `--parallel-workers $(sysctl -n hw.ncpu)`  
  Sets the number of parallel jobs to your CPU core count (maximizing build speed).

---

### 3. ⚠️ Troubleshooting
Most of the source-related errors are fixed in the macOS-specific source code patches, but there are still some dependency errors which might interrupt a complete build.  
Right now, I have no issues completing the build with more than **499** packages, but as mentioned earlier, it depends on which dependencies are missing on individual Macs.

This section will be constantly updated based on user feedback. Below are some of the most common errors I have faced so far:

### Errors:
1. **Case sensitivity issues in Gazebo cmake target files** (e.g., `tinyxml2::tinyxml2` vs `TINYXML2::TINYXML2`)  
    These errors occur due to capitalization mismatches in Homebrew-installed Gazebo cmake files.  
    You will need to manually edit the respective cmake files (e.g., `gz-msgs10-targets.cmake`, `gz-gui8-targets.cmake`) to use the correct lowercase target names.
    
    Specifically, replace occurrences of `TINYXML2::TINYXML2` with `tinyxml2::tinyxml2` (and similar uppercase target names) to lowercase versions.
    
    - `open /opt/homebrew/Cellar/gz-gui/8.4.0_6/lib/cmake/gz-gui8/gz-gui8-targets.cmake`
    - `open /opt/homebrew/Cellar/gz-msgs10/10.3.2_4/lib/cmake/gz-msgs10/gz-msgs10-targets.cmake`
    

    > **Note:**  
    > Open the file based on the location shown in your error output, as versions and paths may differ.

---

2. **ModuleNotFoundError: No module named 'some_library'**
   This error occurs when Python packages are missing during runtime. Simply install the missing python-package with:

   ```bash
   python3 -m pip install some_library
   ```

---

3. **Missing `Config.cmake` Files**

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
