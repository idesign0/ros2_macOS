## 🤖 Navigation 2 on macOS (Apple Silicon)

Navigation2 is the advanced navigation framework for ROS 2, providing autonomous robot path planning, obstacle avoidance, and localization.
It offers lifecycle-managed components for SLAM, path planning, controller execution, and recovery behaviors — enabling reliable, scalable robot navigation across various platforms.

This setup demonstrates Navigation2 running smoothly on macOS with Apple Silicon (M1/M2), leveraging Gazebo simulation, ROS 2 Humble, and native macOS tooling.

With this, you can run full mapping, localization, and navigation pipelines on Apple’s latest hardware — opening up new opportunities for development and testing on macOS environments.

[![The Test](https://github.com/idesign0/ros2_macOS/raw/main/ros-planning/nav2.png)](https://github.com/idesign0/ros2_macOS/raw/main/ros-planning/demo.mp4)

## Install Homebrew Packages  

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

## Build the Navigation packages:

This build focuses on nav2 and related packages only.

```bash
cd ~/ros2_humble
colcon build --base-paths src/ros-planning/ --symlink-install \
  --cmake-args -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=Off \
  --executor parallel \
  --parallel-workers $(sysctl -n hw.ncpu)
```
```bash
source ~/.zshrc
```
## Test Nav2 with TurtleBot4 Simulator

To get started with the TurtleBot4 simulation and Navigation2 (Nav2), refer to the official TurtleBot4 simulator demo:

🔗 [TurtleBot4 Simulator User Manual](https://turtlebot.github.io/turtlebot4-user-manual/software/turtlebot4_simulator.html)

You can also explore the original source repositories:
- 🧩 [turtlebot4](https://github.com/turtlebot/turtlebot4)
- 🧩 [turtlebot4_simulator](https://github.com/turtlebot/turtlebot4_simulator)
---

## 🚀 macOS Users: Use the Patched Version for Gazebo Harmonic

Since you're on **macOS** and likely using the **Gazebo Harmonic** release, it's highly recommended to use the modified TurtleBot4 packages here:

👉 [Modified turtlebot4 packages for macOS + Gazebo Harmonic](https://github.com/idesign0/ROS2_Humble/tree/Mac-main/turtlebot4)

These packages include:
- ✅ Full support for **Gazebo Harmonic**
- ✅ Fixes for **launch-related freezes**
- ✅ Compatibility with **Apple Silicon** and **Xcode toolchain**

---

## 🛠️ Having Issues?

If you encounter errors or unexpected behavior:

👉 First, check the **[Troubleshooting section](https://github.com/idesign0/ros2_macOS/blob/main/README.md#3-%EF%B8%8F-troubleshooting)** in the root `README.md`.  
It contains solutions for common macOS-specific issues, including build errors, launch hangs, and Gazebo compatibility problems.
