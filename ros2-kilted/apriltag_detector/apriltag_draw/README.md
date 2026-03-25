# ROS Apriltag drawing package

This repository has a ROS2 component for drawing apriltags onto an image. You will find this package handy
when trouble shooting apriltag detection problems. The node in this package is usually launched
from a launch file in the [apriltag_detector](https://github.com/ros-misc-utilities/apriltag_detector) package.
Refer there for more information.

## Components

### apriltag\_draw::ApriltagDraw

- Topics (subscribed):
    - ``tags``: the detected tags from the apriltag library.
    - ``image``: the image from which the tags were detected. Must be synced with ``tags``, i.e.
      identical header time stamps in both streams.

- Topics (published):
    - ``image_tags``: image with the tags drawn onto

- Parameters:
    - ``image_transport``: What image transport to use (default ``raw``)
    - ``qos_profile``: What profile to use for subscribing to images. Allowed are ``sensor_data``
      and ``default``. Default ``default``.
    - ``max_queue_size``: How many images or tag detections to keep. Default: 200.


## Launch files

### draw.launch.py

Arguments:
  - ``camera``: name of the camera, e.g. ``/camera_0``. Default: ``camera``.
  - ``image``: name of the image underneath the camera node, e.g. ``image_raw``. The node
      will then subscribe to images ``/camera_name/image_raw``. Default: ``image_raw``.
  - ``image_transport``: the transport to use, e.g. ``compressed``, ``ffmpeg`` etc. Default: ``raw``.

### composable.launch.py

This launch script has identical arguments to ``draw.launch.py``, but is implemented as a composable
node for illustration purposes.

## Example usage

```
ros2 launch apriltag_draw draw.launch.py  image_transport:=ffmpeg camera:=/cam_sync/cam3
```

Will create a node using the compressed ffmpeg image transport, using the following topics,
and publishing an image under ``/cam_sync/cam3/tags``.

```
 Subscribers:
    /cam_sync/cam3/image_raw/ffmpeg: ffmpeg_image_transport_msgs/msg/FFMPEGPacket
    /cam_sync/cam3/tags: apriltag_msgs/msg/AprilTagDetectionArray
  Publishers:
    /cam_sync/cam3/image_tags: sensor_msgs/msg/Image
    /cam_sync/cam3/image_tags/ffmpeg: ffmpeg_image_transport_msgs/msg/FFMPEGPacket
```

## License

This software is issued under the Apache License Version 2.0.
