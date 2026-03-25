# ROS wrapper for ArUco Opencv module

## Quick start
1. Prepare the camera topics.

The ROS driver for the camera you are using should publish 2 topics containing: Camera images (most commonly`image_raw`, `image_color`, `image_mono` or `image_rect` topic) and Camera calibration data (`camera_info` topic). Both topics should be published in the same namespace. Let's assume the driver publishes on these topics:

```
/camera1/image_raw
/camera1/camera_info
```

> :warning: As of now the wrapper does not support multiple camera sources published on the same topics.

The camera should be calibrated (the better the calibration the better the marker pose estimation). If the camera is uncalibrated (the matrices D, K, R, P in `camera_info` messages are set to 0) or the calibration is invalid, you should [recalibrate the camera](https://navigation.ros.org/tutorials/docs/camera_calibration.html). 

Also, make sure that the `frame_id` published with the Camera image headers is not empty.

2. Prepare the marker.

You can use scripts in `aruco_opencv` package to generate PDF files with ArUco markers.

Let's create a 0.2 x 0.2 m marker (including white margin) with ID 0: 
```
ros2 run aruco_opencv create_marker 0
```
The output will look like this:
```
ArUco dictionary: 4X4_50
Inner marker bits: 4
Marker border bits: 1
Pixels per bit: 1
Margin pixels: 1
Marker side size: 0.1500 m - 6 pixels
Output image side size: 0.200 m - 8 pixels
Output DPI: 1.016
Generating marker with ID 0...
Converting images to pdf and writing to output file markers.pdf...
```

Please note the `Marker side size` value. It will be passed to the marker tracker as a parameter.

Make sure to print the marker in the original scale.

3. Run the marker tracker.

Start the `aruco_tracker` node with `cam_base_topic` parameter set to the Camera image topic name and `marker_size` to the value noted in the previous step:
```
ros2 run aruco_opencv aruco_tracker --ros-args -p cam_base_topic:=camera1/image_raw -p marker_size:=0.15
```
If the images are rectified (undistorted), you should also set `image_is_rectified` parameter to `true`:
```
ros2 run aruco_opencv aruco_tracker --ros-args -p cam_base_topic:=camera1/image_rect -p image_is_rectified:=true -p marker_size:=0.075 
```
`aruco_tracker` is a Managed (Lifecycle) Node. It will start in `unconfigured` state, so you will need to configure and activate it manually:
```
ros2 lifecycle set /aruco_opencv configure
ros2 lifecycle set /aruco_opencv activate
```

Alternatively you can use `aruco_tracker_autostart` node which will automatically configure and activate itself:
```
ros2 run aruco_opencv aruco_tracker_autostart --ros-args -p cam_base_topic:=camera1/image_raw -p marker_size:=0.15
```

4. Visualize the data.

For each received image, `aruco_tracker` will publish a message on `aruco_detections` topic:
```
ros2 topic echo /aruco_detections
```

Put the marker in front of the camera. If the marker is detected, the `markers` array should contain the marker poses.

The marker poses are also published on TF. You can visualize the data in RViz by setting fixed frame to the `frame_id` of the camera and adding the TF display.

On the `/aruco_tracker/debug` topic you can see the Camera images with the frame axes of detected markers drawn on top of it.
