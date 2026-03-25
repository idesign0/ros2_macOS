from typing import Optional
import os
import cv2
import numpy as np
from ros_sugar.io import (
    GenericCallback,
    TextCallback,
    get_logger,
)

from ros_sugar.io.utils import (
    image_pre_processing,
    process_encoding,
    read_compressed_image,
    parse_format,
    convert_img_to_jpeg_str,
)

from .utils import (
    create_detection_context,
    draw_detection_bounding_boxes,
    draw_points_2d,
)

from .utils.actions import JointsData

__all__ = ["GenericCallback", "TextCallback"]


class StreamingStringCallback(TextCallback):
    def _get_output(self, **_) -> Optional[str]:
        """Gets text.
        :rtype: str | None
        """

        if not self.msg:
            return None

        # return str if fixed str has been read
        if isinstance(self.msg, str):
            return self.msg
        # return ROS message data
        else:
            if self._template:
                get_logger(self.node_name).warning(
                    "StreamingString topics cannot render templated strings. Discarding template."
                )
            return self.msg.data


class VideoCallback(GenericCallback):
    """
    Video Callback class. Its get method saves a video as an array of arrays
    """

    def __init__(self, input_topic, node_name: Optional[str] = None) -> None:
        """
        Constructs a new instance.
        :param      input_topic:  Subscription topic
        :type       input_topic:  Input
        """
        super().__init__(input_topic, node_name)
        # fixed video needs to be a path to cv2 readable video
        if hasattr(input_topic, "fixed"):
            if os.path.isfile(input_topic.fixed):
                try:
                    # read all video frames
                    video = []
                    cap = cv2.VideoCapture(input_topic.fixed)
                    if not cap.isOpened():
                        raise TypeError()
                    while cap.isOpened():
                        ret, frame = cap.read()
                        if ret:
                            video.append(frame)
                        else:
                            break
                    # Convert frame list to ndarray
                    self.msg = np.array(video)
                except Exception:
                    get_logger(self.node_name).error(
                        f"Fixed path {self.msg} provided for Vidoe topic is not readable Video file"
                    )
            else:
                get_logger(self.node_name).error(
                    f"Fixed path {self.msg} provided for Video topic is not a valid file path"
                )

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets video as a numpy array.
        :returns:   Video as nd_array
        :rtype:     np.ndarray
        """
        if not self.msg:
            return None

        # return np.ndarray if fixed video has been read
        if isinstance(self.msg, np.ndarray):
            return self.msg

        # pre-process in case of weird encodings and reshape ROS topic
        video = []
        for img in self.msg.frames:
            if not getattr(self, "image_encoding", None):
                self.image_encoding = process_encoding(img.encoding)
            video.append(image_pre_processing(img, *self.image_encoding))
        for img in self.msg.compressed_frames:
            if not getattr(self, "compressed_encoding", None):
                self.compressed_encoding = parse_format(img.format)
            video.append(read_compressed_image(img, self.compressed_encoding))
        return np.array(video)


class RGBDCallback(GenericCallback):
    """
    RGBD Callback class. Its get method returns numpy array of the RGB part
    """

    def __init__(self, input_topic, node_name: Optional[str] = None) -> None:
        """
        Constructs a new instance.
        :param      input_topic:  Subscription topic
        :type       input_topic:  Input
        """
        super().__init__(input_topic, node_name)
        self.msg = None
        # fixed RGBD message cannot be read from a file
        if hasattr(input_topic, "fixed"):
            get_logger(self.node_name).error(
                "RGBD message cannot be read from a fixed file"
            )

    def _get_output(self, get_depth=False, **_) -> Optional[np.ndarray]:
        """
        Gets RGBD image as a numpy array.
        :returns:   Image and Depth as nd_array
        :rtype:     np.ndarray
        """
        if not self.msg or not self.msg.rgb:
            return None

        # pre-process and reshape the RGB image
        if not getattr(self, "rgb_encoding", None):
            self.rgb_encoding = process_encoding(self.msg.rgb.encoding)
        rgb = image_pre_processing(self.msg.rgb, *self.rgb_encoding)
        if get_depth:
            if not getattr(self, "depth_encoding", None):
                self.depth_encoding = process_encoding(self.msg.depth.encoding)
            depth = image_pre_processing(self.msg.depth, *self.depth_encoding)
            # Ensure depth has shape (H, W, 1)
            depth_expanded = np.expand_dims(depth, axis=-1)
            # Concatenate along the channel axis and return rgbd
            return np.concatenate((rgb, depth_expanded), axis=-1)
        else:
            return rgb

    def _get_ui_content(self, **_) -> str:
        """Get ui content for image"""
        output = self.get_output()
        return convert_img_to_jpeg_str(output, self.node_name)


class DetectionsMultiSourceCallback(GenericCallback):
    """
    Object detection Callback class for Detections2DMultiSource msg
    Its get method returns the bounding box data
    """

    def __init__(self, input_topic, node_name: Optional[str] = None) -> None:
        """
        Constructs a new instance.

        :param      input_topic:  Subscription topic
        :type       input_topic:  str
        """
        super().__init__(input_topic, node_name)
        self.msg = input_topic.fixed if hasattr(input_topic, "fixed") else None
        self.encoding = None

    def _get_output(self, **_) -> Optional[str]:
        """
        Processes labels and returns a context string for
        prompt engineering

        :returns:   Comma separated classnames
        :rtype:     str
        """
        if not self.msg:
            return None
        # send fixed list of labels if it exists
        if isinstance(self.msg, list):
            return create_detection_context(self.msg)

        # send labels from ROS message
        label_list = [
            label for detection in self.msg.detections for label in detection.labels
        ]
        detections_string = create_detection_context(label_list)
        return detections_string

    def _get_ui_content(self, **_) -> str:
        """Get UI content for the first Detections2D msg in Detections2DMultiSource: draw bounding boxes and labels on the image."""
        if not self.msg:
            return ""

        # If msg is a list, return precomputed detection context
        if isinstance(self.msg, list):
            return create_detection_context(self.msg)

        detections = self.msg.detections
        if not detections:
            return ""

        img = None

        # Decode image or compressed image
        # NOTE: Only checks first detections source
        if self.msg.detections[0].compressed_image.data:
            compressed = self.msg.compressed_image
            if not getattr(self, "encoding", None):
                self.encoding = parse_format(compressed.format)
            img = read_compressed_image(compressed, self.encoding)

        elif self.msg.detections[0].image.data:
            image = self.msg.image
            if not getattr(self, "encoding", None):
                self.encoding = process_encoding(image.encoding)
            img = image_pre_processing(image, *self.encoding)

        # Ensure image exists
        if img is None:
            # Create blank white canvas if no image is available
            img = np.ones((480, 640, 3), dtype=np.uint8) * 255

        # Extract bounding boxes and labels
        bounding_boxes = getattr(detections[0], "boxes", [])
        labels = getattr(detections[0], "labels", [])

        img = draw_detection_bounding_boxes(img, bounding_boxes, labels)

        return convert_img_to_jpeg_str(img, getattr(self, "node_name", "ui"))


class DetectionsCallback(GenericCallback):
    """
    Object detection Callback class for Detections2D msg
    Its get method returns the bounding box data
    """

    def __init__(self, input_topic, node_name: Optional[str] = None) -> None:
        """
        Constructs a new instance.

        :param      input_topic:  Subscription topic
        :type       input_topic:  str
        """
        super().__init__(input_topic, node_name)
        self.msg = input_topic.fixed if hasattr(input_topic, "fixed") else None
        self.encoding = None

    def _get_output(self, **_) -> Optional[str]:
        """
        Processes labels and returns a context string for
        prompt engineering

        :returns:   Comma separated classnames
        :rtype:     str
        """
        if not self.msg:
            return None

        # send fixed list of labels if it exists
        if isinstance(self.msg, list):
            return create_detection_context(self.msg)

        # send labels from ROS message
        label_list = list(self.msg.labels)
        detections_string = create_detection_context(label_list)
        return detections_string

    def _get_ui_content(self, **_) -> str:
        """Get UI content for Detections2D: draw bounding boxes and labels on the image."""
        if not self.msg:
            return ""

        # If msg is a list, return precomputed detection context
        if isinstance(self.msg, list):
            return create_detection_context(self.msg)

        img = None

        # Decode image or compressed image
        if self.msg.compressed_image.data:
            compressed = self.msg.compressed_image
            if not getattr(self, "encoding", None):
                self.encoding = parse_format(compressed.format)
            img = read_compressed_image(compressed, self.encoding)

        elif self.msg.image.data:
            image = self.msg.image
            if not getattr(self, "encoding", None):
                self.encoding = process_encoding(image.encoding)
            img = image_pre_processing(image, *self.encoding)

        # Ensure image exists
        if img is None:
            # Create blank white canvas if no image is available
            img = np.ones((480, 640, 3), dtype=np.uint8) * 255

        # Extract bounding boxes and labels
        bounding_boxes = getattr(self.msg, "boxes", [])
        labels = getattr(self.msg, "labels", [])

        img = draw_detection_bounding_boxes(img, bounding_boxes, labels)

        return convert_img_to_jpeg_str(img, getattr(self, "node_name", "ui"))


class PointsOfInterestCallback(GenericCallback):
    """
    Callback class for PointsOfInterest msg
    Its get method returns the bounding box data
    """

    def __init__(self, input_topic, node_name: Optional[str] = None) -> None:
        """
        Constructs a new instance.

        :param      input_topic:  Subscription topic
        :type       input_topic:  str
        """
        super().__init__(input_topic, node_name)
        self.msg = input_topic.fixed if hasattr(input_topic, "fixed") else None
        self.encoding = None

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Processes labels and returns a context string for
        prompt engineering

        :returns:   Comma separated classnames
        :rtype:     str
        """
        if not self.msg:
            return None

        # send fixed list of points if it exists
        if isinstance(self.msg, list):
            return np.array(self.msg)

        # send points from ROS message
        points = []
        for point in self.msg.points:
            points.append([point.x, point.y])
        return np.array(points)

    def _get_ui_content(self, **_) -> str:
        """Get UI content for PointsOfInterest: draw points on the image."""

        if not self.msg:
            return ""

        points = self.get_output()

        img = None
        # Decode image or compressed image
        if self.msg.compressed_image.data:
            compressed = self.msg.compressed_image
            if not getattr(self, "encoding", None):
                self.encoding = parse_format(compressed.format)
            img = read_compressed_image(compressed, self.encoding)

        elif self.msg.image.data:
            image = self.msg.image
            if not getattr(self, "encoding", None):
                self.encoding = process_encoding(image.encoding)
            img = image_pre_processing(image, *self.encoding)

        # Ensure image exists
        if img is None:
            # Create blank white canvas if no image is available
            img = np.ones((480, 640, 3), dtype=np.uint8) * 255

        img = draw_points_2d(img, points)  # draw points as red circles

        return convert_img_to_jpeg_str(img, getattr(self, "node_name", "ui"))


class JointStateCallback(GenericCallback):
    """
    sensor_msgs/JointState Callback class.

    The state of each joint (revolute or prismatic) is defined by:
    * the position of the joint (rad or m),
    * the velocity of the joint (rad/s or m/s) and
    * the effort that is applied in the joint (Nm or N)
    """

    def _get_output(self, **_) -> Optional[JointsData]:
        """
        Gets joint states as a dictionary with 'joint_names' and 'positions' keys.
        :returns:   Joint states as dict
        :rtype:     dict
        """
        if not self.msg:
            return None

        return JointsData(
            joints_names=self.msg.name,
            positions=np.array(self.msg.position),
            velocities=np.array(self.msg.velocity),
            efforts=np.array(self.msg.effort)
        )
