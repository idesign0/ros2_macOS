# ROS UMich Apriltag detector package

This repository has ROS2 components for detecting Apriltags using the
[UMich Apriltag library](https://april.eecs.umich.edu/software/apriltag).

Note that this package also provides a plugin that works with [pluginlib](https://github.com/ros/pluginlib/tree/ros2), meaning you can load this detector directly into your code without going through a ROS2 component.

For more documentation on how to install and use this component, refer to the documentation
of the [apriltag_detector](https://github.com/ros-misc-utilities/apriltag_detector) package.

## Components

### apriltag_detector_umich::Component

- Topics (subscribed, but ONLY when there is a subscriber to ``tags``):

    - ``image``: image topic to use.
    - ``tags``: the detected tags from the apriltag library.

- Topics (published):

    - ``tags``: tag detections.

- Parameters:

    - ``blur``: Sigma (in pixels) for gaussian blur. Default: 0 (no blur).
    - ``decimate_factor``: Decimate image by this amount. Factor 2 will decimate by 2 in both directions.
        Default: 1 (no decimation).
    - ``image_qos_profile``: QoS profile for image subscription. Allowed values: ``default``, `` sensor_data``.
        Use this parameter to achieve QoS compatibility when subscribing to image data. Defaults to ``default``. 
    - ``image_transport``: The type of image transport to use, e.g. ``compressed``. Default: ``raw``.
    - ``max_allowed_hamming_distance``: discard tags with hamming error larger than this.
        Default: 0 (no error allowed).
    - ``num_threads``: number of threads to use for processing. Default: 1.
    - ``tag_family``: Apriltag family. Allowed values: ``tf16h5``, ``tf25h9``, ``tf36h11``,
        ``tfCircle21h7``, ``tfCircle49h12``, ``tfStandard41h12``, ``tfStandard52h13``, ``tfCustom48h12``.
        Default: ``tf36h11``


## License

This software is issued under the Apache License Version 2.0.
