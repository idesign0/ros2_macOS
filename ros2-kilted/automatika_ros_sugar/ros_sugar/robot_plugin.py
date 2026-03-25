from abc import abstractmethod
from typing import Optional, Callable, Type
import inspect
from rclpy.node import Node
from .io.publisher import Publisher
from .io.supported_types import SupportedType, add_additional_datatypes
from .io.callbacks import GenericCallback
from .base_clients import ServiceClientHandler


def create_supported_type(
    ros_msg_type: type,
    converter: Optional[Callable] = None,
    callback: Optional[Callable] = None,
) -> Type[SupportedType]:
    """Add a new SupportedType derived class to the existing class"""
    if not hasattr(ros_msg_type, "SLOT_TYPES"):
        raise TypeError("ros_msg_type must be a valid ROS2 message type")

    # Dynamically create the new class
    class_name = f"{ros_msg_type.__name__}"
    class_bases = (SupportedType,)
    # Create the new class.
    class_attrs = {
        "_ros_type": ros_msg_type,
    }

    if converter:
        # Inspect the converter method to accept a python type and return the same ros_msg_type
        converter_sig = inspect.signature(converter)
        c_params = list(converter_sig.parameters.keys())

        # Must have at least one input parameter
        if len(c_params) < 1:
            raise TypeError("Converter must take at least one parameter")

        if converter_sig.return_annotation != ros_msg_type:
            raise TypeError(
                f"Converter return annotation ({converter_sig.return_annotation}) "
                f"must match ros_msg_type ({ros_msg_type})"
            )

        def _convert(cls: object, *args, **kwargs):
            return converter(*args, **kwargs)

        class_attrs["convert"] = classmethod(_convert)

    if callback:
        # Inspect the callback to accept the ros_msg_type and return a known python type
        callback_sig = inspect.signature(callback)
        cb_params = list(callback_sig.parameters.values())

        if not cb_params:
            raise TypeError("Callback function must accept at least one argument.")

        # Check that the first argument's type hint matches the ROS message type
        if cb_params[0].annotation != ros_msg_type:
            raise TypeError(
                f"Callback's first argument annotation ({cb_params[0].annotation}) "
                f"must match ros_msg_type ({ros_msg_type})"
            )

        # Check that the return type is a python type (not the ROS message itself)
        if (
            callback_sig.return_annotation == ros_msg_type
            or callback_sig.return_annotation == inspect.Parameter.empty
        ):
            raise TypeError(
                "Callback must have a return annotation of a non-ROS, Python type (e.g., str, int, Dict)."
            )

        def _get_output(self: object, **_):
            return callback(self.msg, **_)

        # Create callback class
        callback_class_attrs = {
            "_get_output": _get_output,  # Make the converter a classmethod
        }
        callback_class = type(
            f"{class_name}Callback", (GenericCallback,), callback_class_attrs
        )
        class_attrs["callback"] = callback_class

    new_type = type(class_name, class_bases, class_attrs)
    add_additional_datatypes([new_type])
    return new_type


class RobotPluginServiceClient(ServiceClientHandler):
    """Template for creating robot specific plugin clients"""

    def __init__(
        self,
        client_node: Node,
        srv_name: str = "robot_serice_name",
        srv_type: Optional[type] = None,
    ):
        super().__init__(
            srv_type=srv_type,
            srv_name=srv_name,
            client_node=client_node,
        )
        self.__name = srv_name
        self.__type = srv_type.__class__.__name__
        self.__publisher_pre_processors: Optional[Publisher] = None

    def replace_publisher(self, current_publisher: Publisher) -> bool:
        """Replace an existing publisher to parse its existing pre processors"""
        if current_publisher._pre_processors is not None:
            self.__publisher_pre_processors = current_publisher

    @property
    def name(self) -> str:
        """Get the service name

        :return: Service name
        :rtype: str
        """
        return self.__name

    @property
    def msg_type(self) -> str:
        """Get the service type

        :return: Service type
        :rtype: str
        """
        return self.__type

    def publish(self, *args, **kwargs) -> bool:
        """Publish service calls"""
        # Call the pre processors if any
        if self.__publisher_pre_processors:
            args = self.__publisher_pre_processors._prepare_for_publish(*args)
        # Publish the service call
        result = self._publish(*args, **kwargs)
        return result

    @abstractmethod
    def start(self, *_) -> bool:
        """Implement any calls to execute on the start of the connection"""
        raise NotImplementedError

    @abstractmethod
    def _publish(self, *args, **kwargs) -> bool:
        """Send service calls / publish commands"""
        raise NotImplementedError

    @abstractmethod
    def end(self, *_) -> bool:
        """Implement any calls to execute on the end of the connection"""
        raise NotImplementedError
