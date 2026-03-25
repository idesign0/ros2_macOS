"""ROS Subscribers Callback Classes"""

import os
from abc import abstractmethod
from typing import Any, Callable, Optional, Union, Dict, List
from socket import socket
import base64

import cv2
import numpy as np
from geometry_msgs.msg import Pose
from jinja2.environment import Template
from nav_msgs.msg import OccupancyGrid, Odometry
from std_msgs.msg import Header
from rclpy.logging import get_logger
from rclpy.subscription import Subscription
from tf2_ros import TransformStamped

from . import utils


class GenericCallback:
    """GenericCallback."""

    def __init__(self, input_topic, node_name: str = "") -> None:
        """__init__

        :param input_topic:
        :type input_topic: Topic
        :param node_name: Name of the ros node using the callback object, defaults to None
        :type node_name: Optional[str], optional
        """

        self.input_topic = input_topic

        # Node name can be changed to a node that the callback is executed in
        # at the time of setting subscriber using set_node_name
        self.node_name: str = node_name
        self.msg = None
        self.__got_msg = False

        # Coordinates frame of the message data (if available)
        self._frame_id: Optional[str] = None

        self._extra_callback: Optional[Callable] = None
        self._get_processed: bool = True  # utilized only if extra callback is set
        self._subscriber: Optional[Subscription] = None
        self._post_processors: Optional[List[Union[Callable, socket]]] = None

    @property
    def frame_id(self) -> Optional[str]:
        """Getter of the message frame ID if available

        :return: Header frame ID
        :rtype: Optional[str]
        """
        return self._frame_id

    def set_node_name(self, node_name: str) -> None:
        """Set node name.

        :param node_name:
        :type node_name: str
        :rtype: None
        """
        self.node_name = node_name

    def set_subscriber(self, subscriber: Subscription) -> None:
        """set_subscriber.

        :param subscriber:
        :rtype: None
        """
        self._subscriber = subscriber

    def on_callback_execute(self, callback: Callable, get_processed=True) -> None:
        """Attach a method to be executed on topic callback

        :param callback:
        :type callback: Callable
        """
        self._extra_callback = callback
        self._get_processed = get_processed

    def callback(self, msg) -> None:
        """
        Topic subscriber callback

        :param msg: Received ros msg
        :type msg: Any
        """
        self.msg = msg
        self.__got_msg = True

        # Get the frame if available
        if hasattr(msg, "header") and isinstance(msg.header, Header):
            self._frame_id = msg.header.frame_id

        if self._extra_callback:
            if self._get_processed:
                self._extra_callback(
                    msg=msg, topic=self.input_topic, output=self.get_output()
                )
            else:
                # Get output would have to be called by the calling method
                self._extra_callback(msg=msg, topic=self.input_topic)

    def add_post_processors(self, processors: List[Union[Callable, socket]]):
        """Add a post processor for callback message

        :param method: Post processor methods or sockets
        :type method: List[Union[Callable, socket]]
        """
        self._post_processors = processors

    def get_output(self, clear_last: bool = False, **kwargs) -> Any:
        """Post process outputs based on custom processors (if any) and return it
        :param output:
        :param args:
        :param kwargs:
        """
        output = self._get_output(**kwargs)
        output_type = type(output)
        if self._post_processors and output:
            # Apply post processors sequentially if defined
            for processor in self._post_processors:
                post_output = utils.run_external_processor(
                    self.node_name, self.input_topic.name, processor, output
                )
                # if any processor output is None, then send None
                if not post_output:
                    return None
                # type check processor output if incorrect, raise an error
                if type(post_output) is not output_type:
                    raise TypeError(
                        f"The output type of a post_processor for topic {self.input_topic.name} callback should be of type {output_type.__name__} | None. Got post_processor output of type {type(post_output).__name__}"
                    )
                # if all good, set output equal to post output
                output = post_output
        # Clear the last message
        if clear_last:
            self.clear_last_msg()

        return output

    @abstractmethod
    def _get_output(self, **_) -> Any:
        """
        Gets the output.
        :returns:   Topic content
        :rtype:     Any
        """
        return self.msg

    @abstractmethod
    def _get_ui_content(self, **_) -> Union[str, Dict]:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        return self.get_output(clear_last=True)

    @property
    def got_msg(self):
        """
        Property is true if an input is received on the topic
        """
        return self.__got_msg

    def clear_last_msg(self):
        """Clears the last received message on the topic"""
        self.msg = None


class StdMsgCallback(GenericCallback):
    """std_msgs callback"""

    def __init__(self, input_topic, node_name: str = "") -> None:
        super().__init__(input_topic, node_name)

    def _get_output(self, **_):
        """
        Gets std_msg data
        """
        if not self.msg:
            return None
        return self.msg.data


class StdMsgArrayCallback(GenericCallback):
    """std_msgs callback"""

    def __init__(self, input_topic, node_name: str = "") -> None:
        super().__init__(input_topic, node_name)

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets std_msg data
        """
        if not self.msg:
            return None

        # Shape of the array from the dimensions
        dims = self.msg.layout.dim
        shape = []
        for dim in dims:
            shape.append(dim.size)

        # If no dimensions are specified, treat as 1D array
        if not shape:
            return np.array(self.msg.data)

        # Convert flat data array to properly shaped numpy array
        try:
            # Ensure total size matches the product of dimensions
            expected_size = np.prod(shape)
            if len(self.msg.data) != expected_size:
                raise ValueError(
                    f"Data size {len(self.msg.data)} doesn't match layout dimensions product {expected_size}"
                )

            # Reshape the data according to the layout
            return np.array(self.msg.data).reshape(shape)

        except Exception as e:
            raise ValueError(f"Failed to parse MultiArray message: {str(e)}") from e


class ImageCallback(GenericCallback):
    """
    Image Callback class. Its get method saves an image as bytes
    """

    def __init__(self, input_topic, node_name: str = "") -> None:
        """
        Constructs a new instance.
        :param      input_topic:  Subscription topic
        :type       input_topic:  Input
        """
        super().__init__(input_topic, node_name)
        # fixed image needs to be a path to cv2 readable image
        if hasattr(input_topic, "fixed"):
            if os.path.isfile(input_topic.fixed):
                try:
                    _image = cv2.imread(input_topic.fixed)
                    self.msg = cv2.cvtColor(_image, cv2.COLOR_BGR2RGB)

                except Exception:
                    get_logger(self.node_name).error(
                        f"Fixed path {input_topic.fixed} provided for Image topic is not a cv2 readable image"
                    )
            else:
                get_logger(self.node_name).error(
                    f"Fixed path {input_topic.fixed} provided for Image topic is not a valid file path"
                )
        self.encoding = None

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets image as a byte array.
        :returns:   Image as bytes
        :rtype:     bytes
        """
        if not self.msg:
            return None

        # return bytes if fixed image has been read
        if isinstance(self.msg, np.ndarray):
            return self.msg
        else:
            if not self.encoding:
                self.encoding = utils.process_encoding(self.msg.encoding)
            # pre-process and reshape
            return utils.image_pre_processing(self.msg, *self.encoding)

    def _get_ui_content(self, **_) -> str:
        """Get ui content for image"""
        output = self.get_output()
        return utils.convert_img_to_jpeg_str(output, self.node_name)


class CompressedImageCallback(ImageCallback):
    """
    CompressedImage Callback class. Its get method saves an image as bytes
    """

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets image as a byte array.
        :returns:   Image as bytes
        :rtype:     bytes
        """
        if not self.msg:
            return None

        # return bytes if fixed image has been read
        if isinstance(self.msg, np.ndarray):
            return self.msg
        else:
            if not self.encoding:
                self.encoding = utils.parse_format(self.msg.format)
            # pre-process
            return utils.read_compressed_image(self.msg, self.encoding)


class TextCallback(GenericCallback):
    """
    Text Callback class. Its get method returns the text
    """

    def __init__(self, input_topic, node_name: str = "") -> None:
        """
        Constructs a new instance.

        :param      input_topic:  Subscription topic
        :type       input_topic:  str
        """
        super().__init__(input_topic, node_name)
        self.msg = input_topic.fixed if hasattr(input_topic, "fixed") else None
        self._template: Optional[Template] = None

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
            if template := self._template:
                context = {self.input_topic.name: self.msg.data}
                return template.render(context)
            return self.msg.data


class AudioCallback(GenericCallback):
    """
    Audio Callback class. Its get method returns the audio
    """

    def __init__(self, input_topic, node_name: str = "") -> None:
        """
        Constructs a new instance.

        :param      input_topic:  Subscription topic
        :type       input_topic:  str
        """
        super().__init__(input_topic, node_name)
        if hasattr(input_topic, "fixed"):
            if os.path.isfile(input_topic.fixed):
                try:
                    with open(input_topic.fixed, "rb") as wavfile:
                        self.msg = wavfile.read()
                except Exception as e:
                    get_logger(self.node_name).error(
                        f"Exception occurred: {e}. Fixed path {input_topic.fixed} provided for Audio topic is not readable wav file"
                    )
            else:
                get_logger(self.node_name).error(
                    f"Fixed path {input_topic.fixed} provided for Audio topic is not a valid file path"
                )

    def _get_output(self, **_) -> Optional[bytes]:
        """
        Gets the audio as byte array.
        :returns:   The output.
        :rtype:     { return_type_description }
        """
        if not self.msg:
            return None
        # send multibyte array from fixed file if it exists
        if isinstance(self.msg, bytes):
            return self.msg
        else:
            # read ROS topic data into a byte array
            audio = bytes()
            audio = b"".join(self.msg.data)

            ### uncomment for testing audio reception - requires the wave package ###
            # import wave
            # with wave.open('./test' + '.wav', 'wb') as file:
            #     file.setnchannels(1)
            #     file.setsampwidth(2)
            #     file.setframerate(16000)
            #     file.writeframes(audio)

            return audio

    def _get_ui_content(self, **_) -> str:
        """Get ui content for audio"""
        # Encode audio bytes to base64 to send as a JSON string
        output = self.get_output()
        return base64.b64encode(output).decode("utf-8")


class MapMetaDataCallback(GenericCallback):
    """
    OccupancyGrid MetaData Callback class. Its get method returns dict of meta data from the occupancy grid topic
    """

    def __init__(self, input_topic, node_name: str = "") -> None:
        """
        Constructs a new instance.

        :param      input_topic:  Subscription topic
        :type       input_topic:  str
        """
        super().__init__(input_topic, node_name)
        self.msg = input_topic.fixed if hasattr(input_topic, "fixed") else None

    def _get_output(self, **_) -> Optional[Dict]:
        """
        Get map metadata
        """

        if not self.msg:
            return None
        # send fixed dict of map metadata if it exists
        if isinstance(self.msg, Dict):
            return self.msg
        # send map metadata from ROS message
        else:
            return {
                "resolution": self.msg.resolution,
                "width": self.msg.width,
                "height": self.msg.height,
            }


class OdomCallback(GenericCallback):
    """
    Ros Odometry Callback Handler to get the robot state in 2D
    """

    def __init__(
        self,
        input_topic,
        node_name: str = "",
    ) -> None:
        super().__init__(input_topic, node_name)
        self.__tf: Optional[TransformStamped] = None

    @property
    def transformation(self) -> Optional[TransformStamped]:
        """
        Sets a new transformation value

        :return:  Detected transform from source to desired goal frame
        :rtype: TransformStamped
        """
        return self.__tf

    @transformation.setter
    def transformation(self, transform: TransformStamped) -> None:
        """
        Sets a new transformation value

        :param transform: Detected transform from source to desired goal frame
        :type transform: TransformStamped
        """
        self.__tf = transform

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Get position data.
        """
        if not self.msg:
            return None
        # send fixed position array if it exists
        if isinstance(self.msg, np.ndarray):
            return self.msg

        # If a transform is given apply it to the message
        if self.transformation:
            odom_transformed = self._transform(self.msg, self.transformation)
            return self._process(odom_transformed)

        # send position data from ROS message
        return self._process(self.msg)

    def _get_ui_content(self, **_) -> Dict:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        return {"frame_id": self.frame_id, "data": self.get_output(clear_last=True)}

    def _process(self, msg: Odometry) -> np.ndarray:
        """Takes Odometry ROS object and converts it to a numpy array with [x, y, z, heading, speed]

        :param msg: Input ROS odometry message
        :type msg: Odometry

        :return: [x, y, z, heading, speed]
        :rtype: np.ndarray
        """
        heading = 2 * np.arctan2(
            msg.pose.pose.orientation.z, msg.pose.pose.orientation.w
        )

        speed = np.sqrt(msg.twist.twist.linear.y**2 + msg.twist.twist.linear.y**2)

        position = msg.pose.pose.position

        return np.array([position.x, position.y, position.z, heading, speed])

    def _transform(self, msg: Odometry, transform: TransformStamped) -> Odometry:
        """
        Applies a transform to a given Odometry message

        :param msg: Odometry message in source frame
        :type msg: Odometry
        :param transform: Odometry transform from current to goal frame
        :type transform: TransformStamped

        :return: Odometry data in new frame
        :rtype: Odometry
        """
        odom_in_map_pose_data = Odometry()
        odom_in_map_pose_data.pose.pose.position.x = transform.transform.translation.x
        odom_in_map_pose_data.pose.pose.position.y = transform.transform.translation.y
        odom_in_map_pose_data.pose.pose.position.z = transform.transform.translation.z
        odom_in_map_pose_data.pose.pose.orientation = transform.transform.rotation

        odom_in_goal_pose_data: Odometry = utils.odom_from_frame1_to_frame2(
            pose_1_in_2=odom_in_map_pose_data, pose_target_in_1=msg
        )
        return odom_in_goal_pose_data


class PointCallback(GenericCallback):
    """
    Ros Pose Callback Handler to get the robot state in 2D
    """

    def __init__(
        self,
        input_topic,
        node_name: str = '',
    ) -> None:
        super().__init__(input_topic, node_name)

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets the point message as a numpy array

        :returns:   [x, y, z]
        :rtype:     Optional[np.ndarray]
        """
        if not self.msg:
            return None

        return np.array([self.msg.x, self.msg.y, self.msg.z])

    def _get_ui_content(self, **_) -> Dict:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        return {"data": self.get_output(clear_last=True)}


class PointStampedCallback(GenericCallback):
    """
    Ros Pose Callback Handler to get the robot state in 2D
    """

    def __init__(
        self,
        input_topic,
        node_name: str = '',
    ) -> None:
        super().__init__(input_topic, node_name)

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets the point stamped message as a numpy array

        :returns:   [x, y, z]
        :rtype:     Optional[np.ndarray]
        """
        if not self.msg:
            return None

        return np.array([self.msg.point.x, self.msg.point.y, self.msg.point.z])

    def _get_ui_content(self, **_) -> Dict:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        return {"frame_id": self.frame_id, "data": self.get_output(clear_last=True)}


class PoseCallback(GenericCallback):
    """
    Ros Pose Callback Handler to get the robot state in 2D
    """

    def __init__(
        self,
        input_topic,
        node_name: str = '',
    ) -> None:
        super().__init__(input_topic, node_name)
        self.__tf: Optional[TransformStamped] = None

    @property
    def transformation(self) -> Optional[TransformStamped]:
        """
        Sets a new transformation value

        :return:  Detected transform from source to desired goal frame
        :rtype: TransformStamped
        """
        return self.__tf

    @transformation.setter
    def transformation(self, transform: TransformStamped) -> None:
        """
        Sets a new transformation value

        :param transform: Detected transform from source to desired goal frame
        :type transform: TransformStamped
        """
        self.__tf = transform

    def _process(self, msg: Pose) -> np.ndarray:
        """Takes Pose ROS object and converts it to a numpy array with [x, y, z, heading]

        :param msg: Input ROS Pose message
        :type msg: Pose

        :return: [x, y, z, heading]
        :rtype: np.ndarray
        """
        heading = 2 * np.arctan2(msg.orientation.z, msg.orientation.w)

        position = msg.position

        return np.array([position.x, position.y, position.z, heading])

    def _transform(self, msg: Pose, transform: TransformStamped) -> Pose:
        """
        Applies a transform to a given Odometry message

        :param msg: Pose message in source frame
        :type msg: Pose
        :param transform: Pose transform from current to goal frame
        :type transform: TransformStamped

        :return: Pose data in new frame
        :rtype: Pose
        """
        odom_in_map_pose_data = Odometry()
        odom_in_map_pose_data.pose.pose.position = transform.transform.translation
        odom_in_map_pose_data.pose.pose.orientation = transform.transform.rotation

        msg_as_odom = Odometry()
        msg_as_odom.pose.pose.position = msg.position
        msg_as_odom.pose.pose.orientation = msg.orientation

        odom_in_goal_pose_data: Odometry = utils.odom_from_frame1_to_frame2(
            pose_1_in_2=odom_in_map_pose_data, pose_target_in_1=msg_as_odom
        )
        output = Pose()
        output.position = odom_in_goal_pose_data.pose.pose.position
        output.orientation = odom_in_goal_pose_data.pose.pose.orientation

        return output

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets the Pose message as a numpy array

        :returns:   [x, y, z, heading]
        :rtype:     Optional[np.ndarray]
        """
        if not self.msg:
            return None

        # If a transform is given apply it to the message
        if self.transformation:
            pose_transformed = self._transform(self.msg, self.transformation)
            return self._process(pose_transformed)

        # send position data from ROS message
        return self._process(self.msg)

    def _get_ui_content(self, **_) -> Dict:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        return {"data": self.get_output(clear_last=True)}


class PoseStampedCallback(PoseCallback):
    """
    Ros PoseStamped Callback Handler to get the robot state in 2D
    """

    def __init__(
        self,
        input_topic,
        node_name: str = '',
    ) -> None:
        super().__init__(input_topic, node_name)

    def _get_output(self, **_) -> Optional[np.ndarray]:
        """
        Gets the PoseStamped message as a numpy array

        :returns:   [x, y, z, heading]
        :rtype:     Optional[np.ndarray]
        """
        if not self.msg:
            return None

        # If a transform is given apply it to the message
        if self.transformation:
            pose_transformed = self._transform(self.msg.pose, self.transformation)
            return self._process(pose_transformed)

        # send position data from ROS message
        return self._process(self.msg.pose)

    def _get_ui_content(self, **_) -> Dict:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        return {"frame_id": self.frame_id, "data": self.get_output(clear_last=True)}


class PathCallback(GenericCallback):
    """
    Ros PoseStamped Callback Handler to get the robot state in 2D
    """

    def __init__(
        self,
        input_topic,
        node_name: str = '',
    ) -> None:
        super().__init__(input_topic, node_name)

    def _get_ui_content(self, **_) -> Dict:
        """
        Utility method to get UI compatible content.
        To be used with external callbacks in UI Node
        :returns:   Topic content
        :rtype:     Any
        """
        if not self.msg.poses:
            return {}

        # Downsampling (Simple Distance Filter)
        # Only keep points that are at least 5cm apart from the previous point
        filtered_points = []
        last_x, last_y = -999, -999
        min_sq_dist = 0.05 * 0.05  # 5cm squared

        for pose_stamped in self.msg.poses:
            x = pose_stamped.pose.position.x
            y = pose_stamped.pose.position.y

            # Always keep the first point
            if not filtered_points:
                filtered_points.extend([x, y])
                last_x, last_y = x, y
                continue

            # Check distance
            dx = x - last_x
            dy = y - last_y
            if (dx * dx + dy * dy) > min_sq_dist:
                filtered_points.extend([x, y])
                last_x, last_y = x, y

        # Always add the very last point to ensure the path reaches the goal
        last_pose = self.msg.poses[-1].pose.position
        if len(filtered_points) >= 2:
            # Check if the last point we added is different from the actual end
            lx = filtered_points[-2]
            ly = filtered_points[-1]
            if last_pose.x != lx or last_pose.y != ly:
                filtered_points.extend([last_pose.x, last_pose.y])

        return {"frame_id": self.frame_id, "data": filtered_points}


class OccupancyGridCallback(GenericCallback):
    def __init__(
        self,
        input_topic,
        node_name: str = '',
        to_numpy: bool = True,
        twoD_to_threeD_conversion_height: float = 0.01,
    ) -> None:
        super().__init__(input_topic, node_name)
        self.__to_numpy = to_numpy
        self.__twoD_to_threeD_conversion_height = twoD_to_threeD_conversion_height

    def _get_output(
        self,
        get_metadata: bool = False,
        get_obstacles: bool = True,
        get_three_d: bool = True,
        **_,
    ) -> Optional[Union[OccupancyGrid, np.ndarray, Dict]]:
        """
        Gets the OccupancyGrid message raw or as a numpy array

        :returns:   Map as an OccupancyGrid or numpy array
        :rtype:     Optional[Union[OccupancyGrid, np.ndarray]]
        """
        if not self.msg:
            return None

        origin_x = self.msg.info.origin.position.x
        origin_y = self.msg.info.origin.position.y
        origin_yaw = 2 * np.arctan2(
            self.msg.info.origin.orientation.z, self.msg.info.origin.orientation.w
        )

        # Get only the map meta data from the message
        if get_metadata:
            return {
                "resolution": self.msg.info.resolution,
                "width": self.msg.info.width,
                "height": self.msg.info.height,
                "origin_x": origin_x,
                "origin_y": origin_y,
                "origin_yaw": origin_yaw,
            }

        # If conversion to numpy is set to False return the ROS message as it is
        if not self.__to_numpy:
            return self.msg

        # Get 2D numpy array
        # index (0,0) is the lower right corner -> add transpose
        grid_data = np.transpose(
            np.asarray(self.msg.data, dtype=np.int8).reshape(
                self.msg.info.height, self.msg.info.width
            )
        )

        if not get_obstacles:
            return grid_data

        # Convert to a numpy array with occupied cells' coordinates in world frame
        occupied_coordinates = (
            np.array(np.where(grid_data != 0), dtype=np.float32) + 0.5
        ) * self.msg.info.resolution

        rotation_matrix = np.array([
            [np.cos(origin_yaw), -np.sin(origin_yaw)],
            [np.sin(origin_yaw), np.cos(origin_yaw)],
        ])

        transformed_coordinates = np.array([origin_x, origin_y]) + np.transpose(
            rotation_matrix @ occupied_coordinates
        )

        if not get_three_d:
            return transformed_coordinates

        threeD_coordinates = np.pad(
            transformed_coordinates,
            ((0, 0), (0, 1)),
            mode="constant",
            constant_values=self.__twoD_to_threeD_conversion_height,
        )

        return threeD_coordinates

    def _get_ui_content(self, **_) -> str:
        """Get ui content for occupancy grid"""
        # Convert occupancy grid values to grayscale image
        output = self.get_output(get_obstacles=False, get_three_d=False)
        img = np.zeros_like(output, dtype=np.uint8)
        img[output == -1] = 127  # unknown
        img[output == 0] = 255  # free
        img[output > 0] = 0  # occupied

        # Flip vertically (ROS map origin is bottom-left, OpenCV image top-left)
        img = np.flipud(img)
        return utils.convert_img_to_jpeg_str(img, self.node_name)
