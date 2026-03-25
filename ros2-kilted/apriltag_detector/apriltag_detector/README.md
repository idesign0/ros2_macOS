# ROS Apriltag detector package

This package implements a generic apriltag detector as both a node and a composable
component. The package also has the base class header files for the apriltag detector plugins implemented by the [UMich](../apriltag_detector_umich/README.md) and the
[MIT](../apriltag_detector_mit/README.md) detector.

With the package you can run any apriltag detector that implements the plugin interface, so long as the required plugin is installed. Note that UMich detector is more sensitive and faster, but it does not work when tags are encroached by black markers or have double-wide black borders (Kalibr boards!). The MIT detector must be used in the latter situation.

For more documentation on how to install this package refer to the documentation
of the [apriltag_detector](https://github.com/ros-misc-utilities/apriltag_detector) repository.

NOTE: Because running an apriltag detector is a heavy weight operation, the detector is very frugal about subscribing to image topics. Unless you bring up e.g. an image viewer to connect to the debug images, the detector will just sit there and do nothing.

## Components

### DetectorComponent

Parameters:
  - ``black_border_width``: (only for MIT). Width (in bits) of the outer black border of
    the apriltags. Note that Kalibr board tags often have border width of 2 bits. Default: 1.
  - ``blur``: (only for UMich). Gaussian blur sigma (in pixels). Default: 0 (no blur).
  - ``decimate_factor``: ratio of decimation, default: 1.0 (no decimation).
  - ``image_transport``: the transport to use, e.g. ``compressed``, ``ffmpeg`` etc.
    Default: ``raw``.
  - ``max_allowed_hamming_distance``: (only for UMich) when more than this number
     of bits are different from a proper code, disregard the tag. Default: 0 (perfect match required).
  - ``num_threads``: (only for UMich). Number of threads to run in parallel. Default: 1.
  - ``tag_family``: Apriltag family. Supported families depend on detector type. Default: "tf36h11".
  - ``type``: What type of detector to use. Valid are ``mit``, ``umich``. Default: ``umich``.

Topics:
  - ``image`` (subscribed to): remap this to the camera topic on which to run the detector.
  - ``tags``(published): remap this to the topic under which the tag detections should be published.

## Launch files

### detect.launch.py

This convenience launch file launches both a detector and a [draw node](../apriltag_draw/README.md) for easy viewing of the tags.

Arguments:
  - ``black_border_width``: (only for MIT). Width (in bits) of the outer black border of
    the apriltags. Note that Kalibr board tags often have border width of 2 bits. Default: 1.
  - ``blur``: (only for UMich). Gaussian blur sigma (in pixels). Default: 0 (no blur).
  - ``camera``: name of the camera, e.g. ``/camera_0``. Default: ``camera``.
  - ``image``: name of the image underneath the camera node, e.g. ``image_raw``. The node
      will then subscribe to images ``/camera_name/image_raw``. Default: ``image_raw``.
  - ``image_transport``: the transport to use, e.g. ``compressed``, ``ffmpeg`` etc.
    Default: ``raw``.
  - ``max_allowed_hamming_distance``: (only for UMich) when more than this number of bits are different from a proper code, disregard the tag. Default: 0 (perfect match required).
    Default: ``raw``.
  - ``tags``: The topic name under which to publish the tags. Default: ``tags``.
  - ``type``: What type of detector to use. Valid are ``mit``, ``umich``. Default: ``umich``.


## Example usage

Start detector and drawing node:

```
ros2 launch apriltag_detector detect.launch.py camera:=/camera
```

Now you have to start a camera server that publishes images under ``/camera/image_raw``, or play such data from a bag.

For the detector to do anything you also have to subscribe to one of its output topics, for instance with ``rqt_image_view``:

```
ros2 run rqt_image_view rqt_image_view /camera/image_tags
```

Alternatively you can look at the decoded tag rate:
```
ros2 topic hz /camera/tags
```

## License

This software is issued under the Apache License Version 2.0.
