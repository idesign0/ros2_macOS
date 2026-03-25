"""The following classes provide wrappers for data being transmitted via ROS topics. These classes form the inputs and outputs of [Components](agents.components.md)."""

from typing import Union, Any, Dict, List, Tuple, Optional
import numpy as np
from attrs import define, field, Factory
from importlib.util import find_spec
from rclpy.logging import get_logger

from sensor_msgs.msg import JointState as JointStateROS

# FROM SUGARCOAT
from ros_sugar.supported_types import (
    SupportedType,
    Audio,
    Image,
    CompressedImage,
    OccupancyGrid,
    Odometry,
    String,
    ROSImage,
    ROSCompressedImage,
    add_additional_datatypes,
)
from ros_sugar.io.topic import QoSConfig, Topic as BaseTopic

from ros_sugar.config import (
    BaseComponentConfig,
    ComponentRunType,
    BaseAttrs,
    base_validators,
)
from ros_sugar.core import BaseComponent
from ros_sugar.core.component import MutuallyExclusiveCallbackGroup
from ros_sugar import UI_EXTENSIONS
from ros_sugar.utils import component_action, component_fallback
from ros_sugar.io.utils import run_external_processor
from ros_sugar.core import Event
from ros_sugar.core import Action
from ros_sugar import actions

from ros_sugar import Launcher

# AGENTS TYPES
from automatika_embodied_agents.msg import (
    Point2D,
    Bbox2D,
    Detections2D,
    Detections2DMultiSource,
)
from automatika_embodied_agents.msg import (
    StreamingString as ROSStreamingString,
    Video as ROSVideo,
    Trackings as ROSTrackings,
    TrackingsMultiSource as ROSTrackingsMultiSource,
    PointsOfInterest as ROSPointsOfInterest,
)
from automatika_embodied_agents.action import VisionLanguageAction
from .callbacks import (
    DetectionsCallback,
    DetectionsMultiSourceCallback,
    PointsOfInterestCallback,
    RGBDCallback,
    VideoCallback,
    StreamingStringCallback,
    JointStateCallback,
)

from .utils.actions import JointsData

__all__ = [
    "String",
    "StreamingString",
    "Video",
    "Audio",
    "Image",
    "CompressedImage",
    "OccupancyGrid",
    "Odometry",
    "Detections",
    "DetectionsMultiSource",
    "PointsOfInterest",
    "Trackings",
    "TrackingsMultiSource",
    "RGBD",
    "JointTrajectoryPoint",
    "JointTrajectory",
    "JointJog",
    "JointState",
    "Topic",
    "QoSConfig",
    "FixedInput",
    "base_validators",
    "BaseAttrs",
    "BaseComponent",
    "BaseComponentConfig",
    "ComponentRunType",
    "Launcher",
    "MapLayer",
    "Route",
    "MutuallyExclusiveCallbackGroup",
    "Event",
    "actions",
    "Action",
    "component_fallback",
    "component_action",
    "VisionLanguageAction",
    "run_external_processor",
]

# =========================================================================
# Additional Datatypes (Augment Sugarcoat datatypes)
# =========================================================================


class StreamingString(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.StreamingString` message type.

    This type represents a string that is being streamed (e.g., token by token from an LLM).
    It contains fields to indicate if the stream is active and if the transmission is complete.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/StreamingString`
    """

    callback = StreamingStringCallback
    _ros_type = ROSStreamingString

    @classmethod
    def convert(
        cls,
        output: str,
        stream: bool = False,
        done: bool = True,
        **_,
    ) -> ROSStreamingString:
        """
        Takes a string and streaming info to return a streaming string custom msg
        :return: ROSStreamingString
        """
        msg = ROSStreamingString()
        msg.stream = stream
        msg.done = done
        msg.data = output
        return msg


class Video(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.Video` message type.

    This type represents a sequence of images (frames). It can handle both raw images and compressed images, bundling them into a single video message structure.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/Video`
    """

    _ros_type = ROSVideo
    callback = VideoCallback

    @classmethod
    def convert(
        cls,
        output: Union[List[ROSImage], List[ROSCompressedImage], List[np.ndarray]],
        **_,
    ) -> ROSVideo:
        """
        Takes an list of images and returns a video message (Image Array)
        :return: Video
        """
        msg = ROSVideo()
        frames = []
        compressed_frames = []
        for frame in output:
            if isinstance(frame, ROSCompressedImage):
                compressed_frames.append(CompressedImage.convert(frame))
            else:
                frames.append(Image.convert(frame))
        msg.frames = frames
        msg.compressed_frames = compressed_frames
        return msg


class Detections(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.Detections2D` message type.

    This type represents 2D object detections, including bounding boxes, labels, and confidence scores.
    It can optionally bundle the source image (RGB or RGBD) associated with the detections.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/Detections2D`
    """

    _ros_type = Detections2D
    callback = DetectionsCallback

    @classmethod
    def convert(
        cls,
        output: Union[Dict, List[Dict]],
        images: Union[
            ROSImage,
            ROSCompressedImage,
            np.ndarray,
            List[ROSImage],
            List[ROSCompressedImage],
            List[np.ndarray],
        ],
        **_,
    ) -> Detections2D:
        """
        Takes object detection data and converts it into a ROS message
        of type Detection2D
        :return: Detection2D
        """
        if isinstance(output, List):
            output = output[0]
            images = images[0] if images else []
        msg = Detections2D()
        msg.scores = output["scores"]
        msg.labels = output["labels"]
        boxes = []
        for bbox in output["bboxes"]:
            box = Bbox2D()
            box.top_left_x = float(bbox[0])
            box.top_left_y = float(bbox[1])
            box.bottom_right_x = float(bbox[2])
            box.bottom_right_y = float(bbox[3])
            boxes.append(box)

        msg.boxes = boxes
        if images:
            if isinstance(images, ROSCompressedImage):
                msg.compressed_image = CompressedImage.convert(images)
            # Handle RealSense RGBD msgs
            elif hasattr(images, "depth"):
                msg.image = Image.convert(images.rgb)
                msg.depth = Image.convert(images.depth)
            else:
                msg.image = Image.convert(images)
        return msg


class DetectionsMultiSource(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.Detections2DMultiSource` message type.

    This type handles a list of `Detections2D` messages, typically used when receiving
    detection data from multiple cameras or sources simultaneously.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/Detections2DMultiSource`
    """

    _ros_type = Detections2DMultiSource
    callback = DetectionsMultiSourceCallback

    @classmethod
    def convert(cls, output: List, images: List, **_) -> Detections2DMultiSource:
        """
        Takes object detections data and converts it into a ROS message
        of type Detections2D
        :return: Detections2D
        """
        msg = Detections2DMultiSource()
        detections = []
        for img, detection in zip(images, output, strict=True):
            detections.append(Detections.convert(detection, img))
        msg.detections = detections
        return msg


class PointsOfInterest(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.PointsOfInterest` message type.

    This type represents a set of 2D coordinates (x, y) on an image that are of interest,
    bundled with the source image or depth map.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/PointsOfInterest`
    """

    _ros_type = ROSPointsOfInterest
    callback = PointsOfInterestCallback  # not defined

    @classmethod
    def convert(
        cls,
        output: List[Tuple[int, int]],
        img: Union[ROSImage, ROSCompressedImage, np.ndarray],
        **_,
    ) -> ROSPointsOfInterest:
        """
        Takes points of interest on an image and converts it into a ROS message
        of type PointsOfInterest
        :return: PointsOfInterest
        """
        msg = ROSPointsOfInterest()
        points = []
        for p in output:
            point = Point2D()
            point.x = float(p[0])
            point.y = float(p[1])
            points.append(point)
        msg.points = points

        if isinstance(img, ROSCompressedImage):
            msg.compressed_image = CompressedImage.convert(img)
        # Handle RealSense RGBD msgs
        elif hasattr(img, "depth"):
            msg.image = Image.convert(img.rgb)
            msg.depth = Image.convert(img.depth)
        else:
            msg.image = Image.convert(img)
        return msg


class Trackings(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.Trackings` message type.

    This type represents tracked objects over time. It includes object IDs, tracked labels,
    bounding boxes, centroids, and estimated velocities, along with the source image.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/Trackings`
    """

    _ros_type = ROSTrackings
    callback = None  # Not defined in EmbodiedAgents

    @classmethod
    def convert(
        cls,
        output: Union[Dict, List[Dict]],
        images: Union[
            ROSImage,
            ROSCompressedImage,
            np.ndarray,
            List[ROSImage],
            List[ROSCompressedImage],
            List[np.ndarray],
        ],
    ) -> ROSTrackings:
        """
        Takes tracking data and converts it into a ROS message
        of type Tracking
        :return: ROSTracking
        """
        # Only consider the first datapoint if a list is sent
        if isinstance(output, List):
            output = output[0]
            images = images[0]
        msg = ROSTrackings()
        msg.ids = output.get("ids") or []
        msg.labels = output.get("tracked_labels") or []

        estimated_velocities = []
        if o_estimated_velocities := output.get("estimated_velocities"):
            for obj_vels in o_estimated_velocities:
                for obj_instance_v in obj_vels:
                    estimated_velocity = Point2D()
                    estimated_velocity.x = obj_instance_v[0]
                    estimated_velocity.y = obj_instance_v[1]
                    estimated_velocities.append(estimated_velocity)

        tracked_boxes = []
        centroids = []
        if o_tracked_points := output.get("tracked_points"):
            for bbox in o_tracked_points:
                # Each 3 points represent one object (top-left, bottom-right, center)
                box = Bbox2D()
                box.top_left_x = bbox[0][0]
                box.top_left_y = bbox[0][1]
                box.bottom_right_x = bbox[1][0]
                box.bottom_right_y = bbox[1][1]
                tracked_boxes.append(box)
                centroid = Point2D()
                centroid.x = bbox[2][0]
                centroid.y = bbox[2][1]
                centroids.append(centroid)

        msg.boxes = tracked_boxes
        msg.centroids = centroids
        msg.estimated_velocities = estimated_velocities
        if isinstance(images, ROSCompressedImage):
            msg.compressed_image = CompressedImage.convert(images)
        # Handle RealSense RGBD msgs
        elif hasattr(images, "depth"):
            msg.image = Image.convert(images.rgb)
            msg.depth = Image.convert(images.depth)
        else:
            msg.image = Image.convert(images)
        return msg


class TrackingsMultiSource(SupportedType):
    """
    Wraps the `automatika_embodied_agents.msg.TrackingsMultiSource` message type.

    This type handles a list of `Trackings` messages, typically used for multi-camera
    tracking scenarios.

    **ROS2 Message Type**: `automatika_embodied_agents/msg/TrackingsMultiSource`
    """

    _ros_type = ROSTrackingsMultiSource
    callback = None  # Not defined

    @classmethod
    def convert(cls, output: List, images: List, **_) -> ROSTrackingsMultiSource:
        """
        Takes trackings data and converts it into a ROS message
        of type ROSTrackings
        :return: ROSTrackings
        """
        msg = ROSTrackingsMultiSource()
        trackings = []
        for img, tracking in zip(images, output, strict=True):
            trackings.append(Trackings.convert(tracking, img))
        msg.trackings = trackings
        return msg


class RGBD(SupportedType):
    """
    Wraps the `realsense2_camera_msgs.msg.RGBD` message type.

    This type represents aligned RGB and Depth images typically produced by RealSense cameras.
    It requires the `realsense2_camera_msgs` package to be installed.

    **ROS2 Message Type**: `realsense2_camera_msgs/msg/RGBD`
    """

    callback = RGBDCallback

    @classmethod
    def get_ros_type(cls) -> type:
        if find_spec("realsense2_camera_msgs") is None:
            raise ModuleNotFoundError(
                "'realsense2_camera_msgs' module is required to use 'RGBD' msg type but it is not installed"
            )
        from realsense2_camera_msgs.msg import RGBD as RealSenseRGBD

        return RealSenseRGBD


class JointTrajectoryPoint(SupportedType):
    """
    Wraps the `trajectory_msgs.msg.JointTrajectoryPoint` message type.

    This type represents a single point in a joint trajectory, including positions,
    velocities, accelerations, and effort for a specific point in time.

    **ROS2 Message Type**: `trajectory_msgs/msg/JointTrajectoryPoint`
    """

    @classmethod
    def get_ros_type(cls) -> type:
        if find_spec("trajectory_msgs") is None:
            raise ModuleNotFoundError(
                "'trajectory_msgs' module is required to use 'JointTrajectory' msg type but it is not installed. Please install the 'ros-<distro>-trajectory-msgs' package."
            )
        from trajectory_msgs.msg import JointTrajectoryPoint as JointTrajectoryPointROS

        return JointTrajectoryPointROS

    @classmethod
    def convert(cls, output: JointsData, index: Optional[int] = None, **_) -> Any:
        """
        Takes joint state data and converts it into a ROS message
        of type JointTrajectoryPoint

        :return: JointTrajectory
        """
        msg = cls.get_ros_type()()
        msg.time_from_start = output.delay

        if index is None:
            msg.positions = output.positions.tolist()
            msg.velocities = output.velocities.tolist()
            msg.accelerations = output.velocities.tolist()
            msg.effort = output.efforts.tolist()
            return msg

        if index < output.positions.shape[0]:
            msg.positions = output.positions[index].tolist()
        if index < output.velocities.shape[0]:
            msg.velocities = output.velocities[index].tolist()
        if index < output.accelerations.shape[0]:
            msg.accelerations = output.accelerations[index].tolist()
        if index < output.efforts.shape[0]:
            msg.effort = output.efforts[index].tolist()
        return msg


class JointTrajectory(SupportedType):
    """
    Wraps the `trajectory_msgs.msg.JointTrajectory` message type.

    This type represents a full joint trajectory, containing a list of `JointTrajectoryPoint`s
    and the names of the joints being controlled.

    **ROS2 Message Type**: `trajectory_msgs/msg/JointTrajectory`
    """

    callback = None

    @classmethod
    def get_ros_type(cls) -> type:
        if find_spec("trajectory_msgs") is None:
            raise ModuleNotFoundError(
                "'trajectory_msgs' module is required to use 'JointTrajectory' msg type but it is not installed. Please install the 'ros-<distro>-trajectory-msgs' package."
            )
        from trajectory_msgs.msg import JointTrajectory as JointTrajectoryROS

        return JointTrajectoryROS

    @classmethod
    def convert(cls, output: JointsData, **_) -> Any:
        """
        Takes joint state data and converts it into a ROS message
        of type JointTrajectory

        :return: JointTrajectory
        """
        msg = cls.get_ros_type()()
        msg.joint_names = output.joints_names
        msg.points = []

        if output.positions.ndim == 1:
            # a single point
            point_msg = JointTrajectoryPoint.convert(output)
            msg.points.append(point_msg)
            return msg

        if output.positions.ndim != 2:
            get_logger("joint_trajectory_publisher").error(
                f"Trying to publish invalid joint trajectory data. Expecting joint positions array dimension 2, got: `{output.positions.ndim}`"
            )
            return None

        # Get points data
        for idx in range(output.positions.shape[0]):
            point_msg = JointTrajectoryPoint.convert(output, index=idx)
            msg.points.append(point_msg)
        return msg


class JointJog(SupportedType):
    """
    Wraps the `control_msgs.msg.JointJog` message type.

    This type represents a command to jog joints, specifying displacements, velocities,
    or duration for immediate execution.

    **ROS2 Message Type**: `control_msgs/msg/JointJog`
    """

    callback = None

    @classmethod
    def get_ros_type(cls) -> type:
        if find_spec("control_msgs") is None:
            raise ModuleNotFoundError(
                "'control_msgs' module is required to use 'JointJog' msg type but it is not installed. Please install the 'ros-<distro>-control-msgs' package."
            )
        from control_msgs.msg import JointJog as JointJogROS

        return JointJogROS

    @classmethod
    def convert(cls, output: JointsData, **_) -> Any:
        """
        Takes joint state data and converts it into a ROS message
        of type JointJog

        :return: JointJog
        """
        msg = cls.get_ros_type()()
        msg.joint_names = output.joints_names

        msg.displacements = output.positions.tolist()
        msg.velocities = output.velocities.tolist()
        msg.duration = output.duration.tolist()

        return msg


class JointState(SupportedType):
    """
    Wraps the `sensor_msgs.msg.JointState` message type.

    This type represents the current state of a set of joints, including their names,
    positions, velocities, and efforts.

    **ROS2 Message Type**: `sensor_msgs/msg/JointState`
    """

    _ros_type = JointStateROS
    callback = JointStateCallback

    @classmethod
    def convert(cls, output: JointsData, **_) -> JointStateROS:
        """
        Takes joint state data and converts it into a ROS message
        of type JointState

        :return: JointState
        """
        msg = JointStateROS()
        msg.name = output.joints_names

        msg.position = output.positions.tolist()
        msg.velocity = output.velocities.tolist()
        msg.effort = output.efforts.tolist()

        return msg


agent_types = [
    StreamingString,
    Video,
    Detections,
    DetectionsMultiSource,
    Trackings,
    TrackingsMultiSource,
    PointsOfInterest,
    RGBD,
    JointState,
    JointJog,
    JointTrajectory,
    JointTrajectoryPoint,
]


add_additional_datatypes(agent_types)

# =========================================================================
# Additional UI Elements (Augment Sugarcoat UI elements)
# =========================================================================


def augment_ui():
    from .ui_elements import INPUT_ELEMENTS, OUTPUT_ELEMENTS

    return INPUT_ELEMENTS, OUTPUT_ELEMENTS


UI_EXTENSIONS["agents"] = augment_ui

# =========================================================================
# Additional Primitives (Augment Sugarcoat primitives)
# =========================================================================


@define(kw_only=True)
class Topic(BaseTopic):
    """
    A topic is an idomatic wrapper for a ROS2 topic, Topics can be given as inputs or outputs to components. When given as inputs, components automatically create listeners for the topics upon their activation. And when given as outputs, components create publishers for publishing to the topic.

    :param name: Name of the topic
    :type name: str
    :param msg_type: One of the SupportedTypes. This parameter can be set by passing the SupportedType data-type name as a string. See a list of supported types [here](https://automatika-robotics.github.io/sugarcoat/advanced/types.html)
    :type msg_type: Union[type[supported_types.SupportedType], str]
    :param qos_profile: QoS profile for the topic
    :type qos_profile: QoSConfig

    Example usage:
    ```python
    position = Topic(name="odom", msg_type="Odometry")
    map_meta_data = Topic(name="map_meta_data", msg_type="MapMetaData")
    ```
    """

    pass


@define(kw_only=True)
class FixedInput(Topic):
    """
    A FixedInput can be provided to components as input and is similar to a Topic except components do not create a subscriber to it and whenever they _read_ it, they always get the same data. The nature of the data depends on the _msg_type_ specified.

    :param name: Name of the topic
    :type name: str
    :param msg_type: One of the SupportedTypes. This parameter can be set by passing the SupportedType data-type name as a string
    :type msg_type: Union[type[supported_types.SupportedType], str]
    :param fixed: Fixed input string or path to a file. Various SupportedTypes implement FixedInput processing differently.
    :type fixed: str | Path

    Example usage:
    ```python
    text0 = FixedInput(
        name="text2",
        msg_type="String",
        fixed="What kind of a room is this? Is it an office, a bedroom or a kitchen? Give a one word answer, out of the given choices")
    ```
    """

    fixed: Any = field()


def _get_topic(topic: Union[Topic, Dict]) -> Topic:
    """Converter to get back a topic or an empty dictionary"""
    if isinstance(topic, Topic):
        return topic
    return Topic(**topic)


def _get_topic_or_action(
    entity: Union[Topic, Action, Dict, List[Union[Topic, Action, Dict]]],
) -> Union[Topic, Action, List[Topic], List[Action]]:
    """Converter to get back a topic, action, or a list of them."""

    # Handle List Input
    if isinstance(entity, list):
        converted_list = []
        for item in entity:
            if isinstance(item, (Topic, Action)):
                converted_list.append(item)
            elif isinstance(item, dict):
                # Convert Dict -> Topic
                converted_list.append(Topic(**item))
            else:
                raise TypeError(f"Invalid route item: {item}")
        return converted_list

    # Handle Single Item Input
    if isinstance(entity, (Topic, Action)):
        return entity

    # Handle Dict Input -> Topic
    return Topic(**entity)


def _get_np_coordinates(
    pre_defined: List[Union[List, Tuple[np.ndarray, str]]],
) -> List[Union[List, Tuple[np.ndarray, str]]]:
    pre_defined_list = []
    for item in pre_defined:
        pre_defined_list.append((np.array(item[0]), item[1]))
    return pre_defined_list


@define(kw_only=True)
class MapLayer(BaseAttrs):
    """A MapLayer represents a single input for a MapEncoding component. It can subscribe to a specific text topic.

    :param subscribes_to: The topic that this map layer is subscribed to.
    :type subscribes_to: Topic
    :param temporal_change: Indicates whether the map should store changes over time for the same position. Defaults to False.
    :type temporal_change: bool
    :param resolution_multiple: A positive multiplication factor for the base resolution of the map grid, for fine or coarse graining the map. Defaults to 1.
    :type resolution_multiple: int
    :param pre_defined: An optional list of pre-defined data points in the layer. Each datapoint is a tuple of [position, text], where position is a numpy array of coordinates.
    :type pre_defined: list[tuple[np.ndarray, str]]

    Example of usage:
    ```python
    my_map_layer = MapLayer(subscribes_to='my_topic', temporal_change=True)
    ```
    """

    subscribes_to: Topic = field(converter=_get_topic)
    temporal_change: bool = field(default=False)
    resolution_multiple: int = field(
        default=1, validator=base_validators.in_range(min_value=0.1, max_value=10)
    )
    pre_defined: List[Union[List, Tuple[np.ndarray, str]]] = field(
        default=Factory(list), converter=_get_np_coordinates
    )


@define(kw_only=True)
class Route(BaseAttrs):
    """
    A Route defines a topic to be routed to by the SemanticRouter, along with samples of similar text that the input must match to for the route to be used.

    :param routes_to: The topic that the input to the SemanticRouter is routed to.
    :type routes_to: Topic
    :param samples: A list of sample text strings associated with this route.
    :type samples: list[str]

    Example of usage:
    ```python
    goto_route = Route(routes_to='goto', samples=['Go to the door', 'Go to the kitchen'])
    ```
    """

    routes_to: Union[Topic, Action, List[Topic], List[Action]] = field(
        converter=_get_topic_or_action
    )  # Only topics would get deserialized here
    samples: List[str] = field()
