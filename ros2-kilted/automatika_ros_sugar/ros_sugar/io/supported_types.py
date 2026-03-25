"""ROS Topics Supported Message Types"""

from typing import Any, Union, Optional, List, Dict
import base64
import numpy as np
import importlib

# GEOMETRY MSGS SUPPORTED ROS TYPES
from geometry_msgs.msg import Point as ROSPoint
from geometry_msgs.msg import PointStamped as ROSPointStamped
from geometry_msgs.msg import Pose as ROSPose
from geometry_msgs.msg import PoseStamped as ROSPoseStamped
from geometry_msgs.msg import Twist as ROSTwist

# NAV_MSGS SUPPORTED ROS TYPES
from nav_msgs.msg import MapMetaData as ROSMapMetaData
from nav_msgs.msg import OccupancyGrid as ROSOccupancyGrid
from nav_msgs.msg import Odometry as ROSOdometry
from nav_msgs.msg import Path as ROSPath
from automatika_ros_sugar.msg import ComponentStatus as ROSComponentStatus

# SENSOR_MSGS SUPPORTED ROS TYPES
from sensor_msgs.msg import Image as ROSImage, CompressedImage as ROSCompressedImage
from sensor_msgs.msg import LaserScan as ROSLaserScan

# STD_MSGS SUPPORTED ROS TYPES
from std_msgs.msg import ByteMultiArray
from std_msgs.msg import String as ROSString
from std_msgs.msg import Bool as ROSBool
from std_msgs.msg import (
    Float32 as ROSFloat32,
    Float32MultiArray as ROSFloat32MultiArray,
)
from std_msgs.msg import (
    Float64 as ROSFloat64,
    Float64MultiArray as ROSFloat64MultiArray,
)

from . import callbacks
from .utils import numpy_to_multiarray


_additional_types = {}


# GENERIC HELPER FUNCTIONS
def _update_supportedtype_callback(existing_class: type, new_class: type) -> None:
    if not new_class.callback or new_class.callback == existing_class.callback:
        # If new type has no callback or it is the same as the current callback -> exit
        return
    if not existing_class.callback:
        # No callback is found for the existing class -> get callback from new type
        existing_class.callback = new_class.callback
    else:
        # If a callback already exists -> augment the list with a new callback
        if isinstance(existing_class.callback, List) and isinstance(
            new_class.callback, List
        ):
            existing_class.callback.extend(new_class.callback)
        elif (
            isinstance(existing_class.callback, List)
            and not isinstance(new_class.callback, List)
            and new_class.callback not in existing_class.callback
        ):
            existing_class.callback.append(new_class.callback)
        elif not isinstance(existing_class.callback, List):
            existing_class.callback = [
                existing_class.callback,
                new_class.callback,
            ]


def _update_supportedtype_conversion(existing_class: type, new_class: type) -> None:
    if not new_class.convert or new_class.convert == existing_class.convert:
        return
    if not existing_class.convert:
        existing_class.convert = new_class.convert
    else:
        if isinstance(existing_class.convert, List) and isinstance(
            new_class.convert, List
        ):
            existing_class.convert.extend(new_class.convert)
        elif (
            isinstance(existing_class.convert, List)
            and not isinstance(new_class.convert, List)
            and new_class.convert not in existing_class.convert
        ):
            existing_class.convert.append(new_class.convert)
        elif not isinstance(existing_class.convert, List):
            existing_class.convert = [
                existing_class.convert,
                new_class.convert,
            ]


# FOR DERIVED PACKGES
def add_additional_datatypes(types: List[type]) -> None:
    """Add additional SupportedType classes to the list of supported ROS2 messages

    :param types: List of supported types
    :type types: List[type]
    """
    global _additional_types
    # Create a dictionary for quick lookup of existing classes by name
    type_dict = {t.__name__: t for t in _additional_types.values()}

    for new_class in types:
        if new_class.__name__ in type_dict:
            # Update the existing class with non-None attributes from the new class
            existing_class = type_dict[new_class.__name__]

            if existing_class == SupportedType or existing_class == new_class:
                # Skip parent
                continue

            _update_supportedtype_callback(existing_class, new_class)

            if hasattr(new_class, "_ros_type") and (
                not hasattr(existing_class, "_ros_type") or not existing_class._ros_type
            ):
                existing_class._ros_type = new_class._ros_type

            _update_supportedtype_conversion(existing_class, new_class)

        else:
            # Add the new class to the list
            _additional_types[f"{new_class.__module__}.{new_class.__qualname__}"] = (
                new_class
            )


# HELPER FUNCTIONS FOR UI CREATION
def get_ros_msg_fields_dict(msg_class: type) -> Dict[str, Any]:
    """
    Parses the fields names and types from any ROS message into a dict.
    Handles nested messages and arrays.

    :return: Dict where value is type string, or nested dict (or list of dicts for arrays)
    """
    parsed_dict = {}
    msg_fields = msg_class.get_fields_and_field_types()

    for field_name, field_type_str in msg_fields.items():
        # Check if it is an array (dynamic '[]' or static '[3]')
        is_array = "[" in field_type_str

        # Extract base type (remove array notation)
        base_type = field_type_str.split("[")[0]

        # Check if it is a complex type (contains slash)
        if "/" in base_type:
            try:
                module_str_name, msg_str_name = base_type.split("/")
                msg_module = importlib.import_module(f"{module_str_name}.msg")
                nested_msg_class = getattr(msg_module, msg_str_name)
                # Get fields dict for nested type
                nested_schema = get_ros_msg_fields_dict(nested_msg_class)
                # If it's an array, wrap the schema in a list to indicate iterable structure
                parsed_dict[field_name] = [nested_schema] if is_array else nested_schema

            except (ModuleNotFoundError, ValueError, AttributeError):
                # Fallback to store the complex type as it is
                parsed_dict[field_name] = field_type_str
        else:
            # Simple types
            parsed_dict[field_name] = field_type_str

    return parsed_dict


def set_ros_msg_from_dict(msg_class: type, data_dict: Dict[str, Any]) -> Any:
    """
    Creates a ROS message object from a dictionary structure.

    :param msg_class: The ROS message class to instantiate (e.g., geometry_msgs.msg.Point)
    :param data_dict: Dictionary containing the values (keys must match message fields)
    :return: An instance of msg_class populated with data
    """
    # Instantiate the message
    msg = msg_class()

    # Get the type definitions
    msg_fields_types = msg_class.get_fields_and_field_types()

    for field_name, field_value in data_dict.items():
        # Skip keys in the dict that don't exist in the message definition
        if field_name not in msg_fields_types:
            continue

        ros_type = msg_fields_types[field_name]

        # Check if the field is an array/list (contains '[')
        is_array = "[" in ros_type

        # Handle Complex Types (nested messages, e.g., 'geometry_msgs/Point')
        if "/" in ros_type:
            # Extract base type name (remove array brackets if present)
            base_type = ros_type.split("[")[0]
            (module_str_name, msg_str_name) = base_type.split("/")

            try:
                msg_module = importlib.import_module(f"{module_str_name}.msg")
                nested_msg_class = getattr(msg_module, msg_str_name)

                if is_array:
                    # If it's a list of complex objects, recurse for each item
                    # field_value is expected to be a list of dicts
                    complex_array = []
                    for item in field_value:
                        complex_array.append(
                            set_ros_msg_from_dict(nested_msg_class, item)
                        )
                    setattr(msg, field_name, complex_array)
                else:
                    # Single complex object, recurse once
                    # field_value is expected to be a dict
                    nested_msg = set_ros_msg_from_dict(nested_msg_class, field_value)
                    setattr(msg, field_name, nested_msg)

            except (ModuleNotFoundError, ValueError, AttributeError) as e:
                print(f"Error instantiating nested message for {field_name}: {e}")

        # Handle Simple Types (int, float, string, etc.)
        else:
            attr_type = type(getattr(msg, field_name))
            setattr(msg, field_name, attr_type(field_value))

    return msg


# SUPPORTED TYPES
class Meta(type):
    """Meta."""

    def __repr__(cls):
        """__repr__.
        :param cls:
        """
        return cls.__name__

    def __str__(cls):
        """__str__.
        :param cls:
        """
        return cls.__name__


class SupportedType:
    """
    Class used to define all supported data types (ROS messages)
    callback: Callback associated with specified type
    _publish: ROS message creation method associate with specified type
    """

    # set metaclass
    __metaclass__ = Meta

    # associated ROS type
    _ros_type: type

    # callback class
    callback = callbacks.GenericCallback

    @classmethod
    def convert(cls, *output, **_) -> Any:
        """ROS message converter function for datatype
        :param args:
        :type _: Any
        :rtype: Any
        """
        return output

    @classmethod
    def get_ros_type(cls) -> type:
        """Getter of the ROS2 message type

        :return: ROS2 type
        :rtype: type
        """
        return cls._ros_type

    @classmethod
    def convert_ui_dict(cls, data: Dict, **_) -> str:
        return data.get("data", "")


class String(SupportedType):
    """String."""

    _ros_type = ROSString
    callback = callbacks.TextCallback

    @classmethod
    def convert(cls, output: str, **_) -> ROSString:
        """
        Takes a string and returns a ROS message of type String
        :return: String
        """
        msg = ROSString()
        msg.data = output
        return msg


class Bool(SupportedType):
    """Bool."""

    _ros_type = ROSBool
    callback = callbacks.StdMsgCallback

    @classmethod
    def convert(cls, output: bool, **_) -> ROSBool:
        """
        Takes a bool and returns a ROS message of type Bool
        :return: Bool
        """
        msg = ROSBool()
        msg.data = output
        return msg

    @classmethod
    def convert_ui_dict(cls, data: Dict, **_) -> str:
        val = data.get("data", "")
        return val == "on" or val == "1"


class Float32(SupportedType):
    """Float32."""

    _ros_type = ROSFloat32
    callback = callbacks.StdMsgCallback

    @classmethod
    def convert(cls, output: float, **_) -> ROSFloat32:
        """
        Takes a float and returns a ROS message of type Float32
        :return: Float32
        """
        msg = ROSFloat32()
        msg.data = output
        return msg


class Float32MultiArray(SupportedType):
    """Float32MultiArray."""

    _ros_type = ROSFloat32MultiArray
    callback = callbacks.StdMsgArrayCallback

    @classmethod
    def convert(cls, output: np.ndarray, **_) -> ROSFloat32MultiArray:
        """
        Takes a numpy array and returns a ROS message of type Float32MultiArray
        :return: Float32
        """
        return numpy_to_multiarray(output, ROSFloat32MultiArray)


class Float64(SupportedType):
    """Float64."""

    _ros_type = ROSFloat64
    callback = callbacks.StdMsgCallback

    @classmethod
    def convert(cls, output: float, **_) -> ROSFloat64:
        """
        Takes a float and returns a ROS message of type Float64
        :return: Float64
        """
        msg = ROSFloat64()
        msg.data = output
        return msg


class Float64MultiArray(SupportedType):
    """Float64MultiArray."""

    _ros_type = ROSFloat64MultiArray
    callback = callbacks.StdMsgArrayCallback

    @classmethod
    def convert(cls, output: np.ndarray, **_) -> ROSFloat64MultiArray:
        """
        Takes a numpy array and returns a ROS message of type Float64MultiArray
        :return: Float32
        """
        return numpy_to_multiarray(output, ROSFloat64MultiArray)


class Image(SupportedType):
    """Image."""

    _ros_type = ROSImage
    callback = callbacks.ImageCallback

    @classmethod
    def convert(cls, output: Union[ROSImage, np.ndarray], **_) -> ROSImage:
        """
        Takes a ROS Image message or numpy array and returns a ROS Image message
        :return: ROSImage
        """
        if isinstance(output, ROSImage):
            return output
        msg = ROSImage()
        msg.height = output.shape[0]
        msg.width = output.shape[1]
        msg.data = output.flatten()
        return msg


class CompressedImage(Image):
    """CompressedImage format usually provided by camera vendors"""

    _ros_type = ROSCompressedImage
    callback = callbacks.CompressedImageCallback

    @classmethod
    def convert(
        cls, output: Union[ROSCompressedImage, np.ndarray], **_
    ) -> ROSCompressedImage:
        """
        Takes a ROS CompressedImage message or numpy array and returns
        a ROS CompressedImage message
        :return: ROSCompressedImage
        """
        if isinstance(output, ROSCompressedImage):
            return output
        msg = ROSCompressedImage()
        msg.format = "png"
        msg.data = output.flatten()
        return msg


class Audio(SupportedType):
    """Audio."""

    _ros_type = ByteMultiArray
    callback = callbacks.AudioCallback

    @classmethod
    def convert(cls, output: Union[str, bytes], **_) -> ByteMultiArray:
        """
        Takes an array of audio data and returns a ROS message
         of type AudioData
        :return: AudioData
        """
        # Handle base64 encoded strings
        if isinstance(output, str):
            output = base64.b64decode(output)

        msg = ByteMultiArray()
        msg.data = [bytes([b]) for b in output]
        return msg


class MapMetaData(SupportedType):
    """MapMetaData."""

    _ros_type = ROSMapMetaData
    callback = callbacks.MapMetaDataCallback


class Odometry(SupportedType):
    """Odometry"""

    _ros_type = ROSOdometry
    callback = callbacks.OdomCallback


class LaserScan(SupportedType):
    """LaserScan"""

    _ros_type = ROSLaserScan
    callback = callbacks.GenericCallback


class Path(SupportedType):
    """Path"""

    _ros_type = ROSPath
    callback = callbacks.PathCallback

    @classmethod
    def convert(cls, output, **_) -> Any:
        """ROS message converter function for datatype
        :param args:
        :type _: Any
        :rtype: Any
        """
        return output


class OccupancyGrid(SupportedType):
    """OccupancyGrid"""

    _ros_type = ROSOccupancyGrid
    callback = callbacks.OccupancyGridCallback

    @classmethod
    def convert(
        cls,
        output: Union[np.ndarray, ROSOccupancyGrid],
        resolution: float,
        origin: Optional[ROSPose] = None,
        **_,
    ) -> ROSOccupancyGrid:
        """ROS message converter function for datatype OccupancyGrid.

        :param output:
        :type output: np.ndarray
        :param _:
        :rtype: ROSOccupancyGrid
        """
        if isinstance(output, ROSOccupancyGrid):
            return output

        if not len(output.shape) == 2:
            raise TypeError("OccupancyGrid data must be a 2D array")

        msg = ROSOccupancyGrid()

        # Set MetaData
        msg.info = ROSMapMetaData()
        msg.info.map_load_time = msg.header.stamp
        msg.info.width = output.shape[0]
        msg.info.height = output.shape[1]
        msg.info.resolution = resolution
        msg.info.origin = origin if origin else Pose()

        # flatten by column
        # index (0,0) is the lower right corner of the grid in ROS
        msg.data = output.flatten("F").astype(np.int8).tolist()
        return msg


class Point(SupportedType):
    """Point"""

    _ros_type = ROSPoint
    callback = callbacks.PointCallback

    @classmethod
    def convert(cls, output: np.ndarray, **_) -> ROSPoint:
        """ROS message converter function for datatype Point.

        :param output:
        :type output: np.ndarray
        :param _:
        :rtype: ROSPoint
        """
        msg = ROSPoint()
        if output.shape[0] < 3:
            raise ValueError(
                f"Cannot convert given value {output} to a ROS Point message"
            )
        msg.x = output[0]
        msg.y = output[1]
        msg.z = output[2]
        return msg

    @classmethod
    def convert_ui_dict(cls, data: Dict, **_) -> str:
        return np.array(
            [
                float(data.get("x", 0.0)),
                float(data.get("y", 0.0)),
                float(data.get("z", 0.0)),
            ],
            dtype=np.float64,
        )


class PointStamped(Point):
    """PointStamped"""

    _ros_type = ROSPointStamped
    callback = callbacks.PointStampedCallback

    @classmethod
    def convert(cls, output: np.ndarray, **_) -> ROSPointStamped:
        """ROS message converter function for datatype Point.

        :param output:
        :type output: np.ndarray
        :param _:
        :rtype: ROSPointStamped
        """
        msg = ROSPointStamped()
        if output.shape[0] < 3:
            raise ValueError(
                f"Cannot convert given value '{output}' to a ROS PointStamped message"
            )
        msg.point.x = output[0]
        msg.point.y = output[1]
        msg.point.z = output[2]
        return msg


class Pose(SupportedType):
    """Pose"""

    _ros_type = ROSPose
    callback = callbacks.PoseCallback

    @classmethod
    def convert(cls, output: np.ndarray, **_) -> ROSPose:
        """ROS message converter function for datatype Point.

        :param output:
        :type output: np.ndarray
        :param _:
        :rtype: ROSPose
        """
        msg = ROSPose()
        if output.shape[0] < 3:
            raise ValueError(
                f"Cannot convert given value '{output}' to a ROS Pose message"
            )
        msg.position.x = output[0]
        msg.position.y = output[1]
        msg.position.z = output[2]

        # Check for orientation
        if output.shape[0] == 7:
            msg.orientation.w = output[3]
            msg.orientation.x = output[4]
            msg.orientation.y = output[5]
            msg.orientation.z = output[6]
        return msg

    @classmethod
    def convert_ui_dict(cls, data: Dict, **_) -> str:
        return np.array(
            [
                float(data.get("x", 0.0)),
                float(data.get("y", 0.0)),
                float(data.get("z", 0.0)),
                float(
                    data.get("ori_w", 0.0) or "1.0"
                ),  # 'or' is added to handle empty inputs (orientation is optional)
                float(data.get("ori_x", 0.0) or "0.0"),
                float(data.get("ori_y", 0.0) or "0.0"),
                float(data.get("ori_z", 0.0) or "0.0"),
            ],
            dtype=np.float64,
        )


class PoseStamped(Pose):
    """PoseStamped"""

    _ros_type = ROSPoseStamped
    callback = callbacks.PoseStampedCallback

    @classmethod
    def convert(cls, output: np.ndarray, **_) -> ROSPoseStamped:
        """ROS message converter function for datatype Point.

        :param output:
        :type output: np.ndarray
        :param _:
        :rtype: ROSPoseStamped
        """
        msg = ROSPoseStamped()
        if output.shape[0] < 3:
            raise ValueError(
                f"Cannot convert given value '{output}' to a ROS PoseStamped message"
            )
        msg.pose.position.x = output[0]
        msg.pose.position.y = output[1]
        msg.pose.position.z = output[2]
        # Check for orientation
        if output.shape[0] == 7:
            msg.pose.orientation.w = output[3]
            msg.pose.orientation.x = output[4]
            msg.pose.orientation.y = output[5]
            msg.pose.orientation.z = output[6]
        return msg


class ComponentStatus(SupportedType):
    """Component Health Status"""

    _ros_type = ROSComponentStatus


class Twist(SupportedType):
    """Twist for Control Commands"""

    _ros_type = ROSTwist

    @classmethod
    def convert(cls, vx: float, vy: float, omega: float, **_) -> ROSTwist:
        """ROS message converter function for datatype Point.

        :param output:
        :type output: np.ndarray
        :param _:
        :rtype: ROSPoseStamped
        """
        msg = ROSTwist()
        msg.linear.x = vx
        msg.linear.y = vy
        msg.angular.z = omega
        return msg
