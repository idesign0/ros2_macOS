# ROS MIT Apriltag detector package

This repository has ROS2 components for detecting Apriltags using the
[MIT apriltag library implementation](https://people.csail.mit.edu/kaess/apriltags).

Note that this package also provides a plugin that works with [pluginlib](https://github.com/ros/pluginlib/tree/ros2), meaning you can load this detector directly into your code without going through a ROS2 component.

For more documentation on how to install and use this component, refer to the documentation
of the [apriltag_detector](https://github.com/ros-misc-utilities/apriltag_detector) package.

## Components

### apriltag_detector_mit::Component

- Topics (subscribed, but ONLY when there is a subscriber to ``tags``):

    - ``image``: image topic to use.
    - ``tags``: the detected tags from the apriltag library.

- Topics (published):

    - ``tags``: tag detections.

- Parameters:

    - ``black_border_width``. Integer specifying the thickness of the black border (in bits).       Default: 1.
    - ``image_qos_profile``: QoS profile for image subscription. Allowed values: ``default``,
        ``sensor_data``.
        Use this parameter to achieve QoS compatibility when subscribing to image data. Defaults to ``default``. 
    - ``image_transport``: The type of image transport to use, e.g. ``compressed``. Default: `raw``.
    - ``tag_family``: Apriltag family. Allowed values: ``tf16h5``, ``tf25h9``, ``tf36h11``.
        Default: ``tf36h11``

## License

This software is issued under the Apache License Version 2.0.
Note that this package links against the [MIT apriltag detector](https://github.com/ros-misc-utilities/apriltag_mit) which is licensed under LGPLv2.1.

