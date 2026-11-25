# Testing ROS 2 on macOS: how you can help

The Homebrew release of ROS 2 Jazzy ships about 2,000 packages from roughly 650 upstream
repositories. CI builds all of them. It also installs the release on a clean Mac and runs
a small set of checks: talker/listener, the Gazebo physics step, an rviz2 load and reading
a Nav2 map. Everything else has only been built, not run. We need people who use these
frameworks to run them on their own Macs and tell us what works, what breaks, and what
could be better.

You don't need to test everything. Pick the areas you actually use, run the checks below,
and report the result with the
[runtime report form](https://github.com/idesign0/ros2_macOS/issues/new?template=runtime-report.yml).
"It worked" is a useful report too: it tells us which parts are verified.

## Setup

```zsh
brew install idesign0/ros2/ros2-jazzy
```

Then follow the caveats `brew` prints: the Python 3.11 modules (`pip3.11 install ...`), the
`source /opt/homebrew/opt/ros2-jazzy/setup.zsh` line and the Cyclone DDS settings. For
every report, please include:

```zsh
sw_vers; uname -m; brew info --json=v2 ros2-jazzy | python3 -c 'import json,sys; print(json.load(sys.stdin)["formulae"][0]["installed"][0]["version"])'
echo $RMW_IMPLEMENTATION
```

## What to check, by area

Each area lists a few starting commands. Going beyond them with your own launch files,
robots, data and workflows is exactly what we can't do in CI.

### 1. Core: nodes, topics, services, actions, parameters, launch
```zsh
ros2 run demo_nodes_cpp talker          # terminal 1
ros2 run demo_nodes_py listener         # terminal 2 (C++ to Python)
ros2 launch demo_nodes_cpp talker_listener_launch.py
ros2 run demo_nodes_cpp add_two_ints_server   # then: ros2 run demo_nodes_cpp add_two_ints_client
ros2 run action_tutorials_cpp fibonacci_action_server   # then: ros2 action send_goal /fibonacci action_tutorials_interfaces/action/Fibonacci "{order: 10}"
ros2 doctor --report
```

### 2. Middleware (RMW): Fast DDS, Cyclone DDS, Zenoh
Run the core checks under each RMW, between two processes and, if you can, between a Mac
and a Linux machine on the same network.
```zsh
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp      # or rmw_cyclonedds_cpp
export RMW_IMPLEMENTATION=rmw_zenoh_cpp && ros2 run rmw_zenoh_cpp rmw_zenohd   # router first
```
Report latency or CPU numbers if you notice anything unusual. macOS timing differs from Linux.

### 3. Command-line tools and recording
```zsh
ros2 bag record -a -s mcap -o test_bag      # then: ros2 bag info test_bag, ros2 bag play test_bag
ros2 topic hz /chatter; ros2 topic bw /chatter; ros2 param list
```

### 4. GUI tools: rviz2, rqt, PlotJuggler, turtlesim
```zsh
rviz2
rqt                       # try rqt_graph, rqt_console, rqt_plot, rqt_image_view
ros2 run plotjuggler plotjuggler
ros2 run turtlesim turtlesim_node   # then: ros2 run turtlesim turtle_teleop_key
ros2 launch urdf_tutorial display.launch.py model:=urdf/01-myfirst.urdf
```
Please mention your display setup (Retina, external monitor) if rendering looks wrong.

### 5. Web bridges
```zsh
ros2 launch foxglove_bridge foxglove_bridge_launch.xml     # connect Foxglove to ws://localhost:8765
ros2 launch rosbridge_server rosbridge_websocket_launch.xml
```

### 6. Gazebo (Harmonic) and ros_gz
On macOS the server and the GUI run as separate processes:
```zsh
gz sim -s shapes.sdf      # terminal 1: server
gz sim -g                 # terminal 2: GUI (Ogre 2 / Metal)
ros2 run ros_gz_bridge parameter_bridge /clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock
```
Try your own worlds, sensors (cameras, lidar, IMU) and plugins.

### 7. Navigation: Nav2, SLAM, localization
```zsh
ros2 launch nav2_bringup tb4_simulation_launch.py
ros2 launch nav2_bringup tb3_simulation_launch.py
ros2 launch slam_toolbox online_async_launch.py
ros2 launch robot_localization ekf.launch.py
```
Set a goal in rviz2 and say whether the robot reaches it. Also: Nav2 plugins you use
(MPPI, Smac, collision monitor), keepout and speed filters, waypoint following.

### 8. Manipulation: MoveIt 2
```zsh
ros2 launch moveit_resources_panda_moveit_config demo.launch.py
```
Plan and execute in rviz2. Also: MoveIt Task Constructor, pick_ik, servo, your own robot's config.

### 9. Control: ros2_control and MuJoCo
```zsh
ros2 launch gz_ros2_control_demos diff_drive_example.launch.py
ros2 launch mujoco_ros2_control_demos 01_basic_robot.launch.py
```
Also: controllers you use (joint trajectory, admittance, diff drive) and real-time behaviour.

### 10. Perception: images, point clouds, markers
```zsh
ros2 run image_tools cam2image      # Mac camera; then: ros2 run image_tools showimage
python3.11 -c "import cv_bridge, rclpy; print('cv_bridge ok')"
```
Also: image_proc, image_transport plugins (compressed, theora), pcl_ros, apriltag_ros,
aruco_ros, laser filters, event cameras.

### 11. Mapping and state estimation: Cartographer, MOLA/MRPT, grid_map, RTAB-Map
```zsh
ros2 launch grid_map_demos filters_demo_launch.py
```
Cartographer and MOLA need recorded data, so your own bags are ideal. RTAB-Map
(`rtabmap_ros`) arrives in the next Jazzy release.

### 12. Fleet: Open-RMF
```zsh
ros2 launch rmf_demos_gz office.launch.xml
```

### 13. Other simulators: Webots
```zsh
ros2 launch webots_ros2_turtlebot robot_launch.py   # needs the Webots app installed
```

### 14. Hardware drivers (USB, network, serial)
These need real devices, and nobody can test them in CI. If you own any of these, a short
"connects and publishes" report helps a lot:
- cameras: Intel RealSense (`realsense2_camera`), GStreamer/UVC (`gscam`), FLIR pan-tilt (`flir_ptu_driver`)
- lidars: Ouster (`ouster_ros`), RPLIDAR (`rplidar_ros`), Velodyne (`velodyne_driver`), SICK (`sick_scan_xd`, `sick_safetyscanners2`), RoboSense (`rslidar_sdk`), Hesai (`nebula_hesai`)
- IMU and GNSS: MicroStrain (`microstrain_inertial_driver`), BNO055, u-blox (`ublox_dgnss`), NMEA (`nmea_navsat_driver`), NTRIP (`ntrip_client`)
- robots and actuators: Dynamixel (`dynamixel_hardware`), Clearpath, Husarion, KUKA (`kuka_drivers`)
- drones: MAVROS (PX4/ArduPilot), Crazyflie
- input: gamepads (`ros2 run joy joy_node`), `teleop_twist_keyboard`

### 15. Autonomy and behaviour: Autoware Core, Lanelet2, behaviour trees, planning
```zsh
python3.11 -c "import lanelet2; print('lanelet2 ok')"
```
Also: BehaviorTree.CPP, SMACC2, YASMIN, SMACH, FlexBE, PlanSys2.

### 16. Building your own packages on top (overlay)
This is how most people really use ROS, and it shows whether the release ships
everything a build needs:
```zsh
mkdir -p ~/ros_ws/src && cd ~/ros_ws/src
ros2 pkg create --build-type ament_cmake --dependencies rclcpp std_msgs my_pkg
cd ~/ros_ws && colcon build && source install/setup.zsh
```
Then build a real workspace of yours, especially packages that use MoveIt, Nav2, PCL,
OpenCV, Gazebo or ros2_control. Report any `find_package` failure, missing header or link
error, with the full error.

### 17. Long runs and everyday use
- Does anything degrade over hours (memory, CPU, dropped messages)?
- How does performance compare with the same setup on Linux?
- What was missing, slow, or awkward in your normal workflow?

## Known gaps
- Excluded from this release on purpose (`ci/skip-list.txt` explains why): for example
  mvsim, mapviz, turtlebot3_gazebo, depthai_ros_driver, rtabmap_rviz_plugins.
- Some drivers ship only their messages or descriptions, not the driver itself, for example
  Orbbec, Spinnaker, Allied Vision Vimba, Phidgets, NovAtel, Kobuki, Kinova (`kortex_driver`),
  ros2_socketcan, plus `ffmpeg_image_transport` and Open3D. If you need one of these, say
  so in a report; that tells us what to fix first.
- No NVIDIA/CUDA/TensorRT. Packages that need them are built without GPU support, or not at all.
- Kilted and Humble have their own formulae (`ros2-kilted`, `ros2-humble`). Reports for
  those are welcome too; please name the distro.

## Reporting
- **Works / doesn't work / could be better**: use the
  [runtime report form](https://github.com/idesign0/ros2_macOS/issues/new?template=runtime-report.yml).
- **A crash or error**: include the exact command, the full output, and for a crash the
  `~/Library/Logs/DiagnosticReports/*.ips` file.
- **Installation problems**: [idesign0/homebrew-ros2 issues](https://github.com/idesign0/homebrew-ros2/issues).
