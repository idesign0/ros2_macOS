# ROS2 Apriltag tools package

This repository has a ROS2 tools for image processing related to AprilTags. For more about AprilTag detection under ROS2, see the [apriltag_detector package](https://github.com/ros-misc-utilities/apriltag_detector).


## Nodes

### detect\_from\_bag

This node runs an apriltag detector on a bag with images and produces an output bag that has the original bag content plus the apriltag detections and debug images.

- Parameters:
    - ``in_bag``: path of input bag
    - ``image_topic``: name of image topic on which to run the AprilTag detector
    - ``image_tag_topic``: output topic for images with detected AprilTags shown
    - ``tag_topic``: output topic for detected tags
    - ``out_bag``: path of output bag

Example:
```bash
ros2 run apriltag_tools detect_from_bag --ros-args -p "in_bag:=./input_bag" -p "image_topic:=/camera/image_raw" -p "out_bag:=./output_bag"
```

## License

This software is issued under the Apache License Version 2.0.
