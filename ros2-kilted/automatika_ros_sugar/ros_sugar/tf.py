"""ROS TF Listener"""

from typing import Optional
import numpy as np
from attrs import define, field
from rclpy.logging import get_logger
from rclpy.time import Time
from rclpy.timer import Timer
from tf2_ros import ConnectivityException, LookupException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener

from .config import BaseAttrs, base_validators


@define
class TFListenerConfig(BaseAttrs):
    """
    TF listener config
    """

    lookup_rate: float = field(
        default=30.0, validator=base_validators.in_range(min_value=1e-6, max_value=1e6)
    )

    source_frame: str = field(default="")  # TF lookup source frame
    goal_frame: str = field(default="")  # TF lookup goal frame
    static_tf: bool = field(default=False)  # If the transform is static


class TFListener:
    """
    ROS TF listener class to lookup a transformation and execute it
    """

    def __init__(
        self,
        tf_config: Optional[TFListenerConfig] = None,
        node_name: str = "",
    ) -> None:
        """
        Sets up a transform listener in ros

        :param node: ROS node using the transform
        :type node: Node
        :param tf_config: Lookup config, defaults to TFListenerConfig()
        :type tf_config: TFListenerConfig, optional
        """
        if not tf_config:
            tf_config = TFListenerConfig()

        self._tf_buffer = Buffer()
        self.node_name = node_name
        self.config = tf_config

        self._tf_listener = None

        self._timer = None  # timer to lookup the transform with given rate

        # result
        self.transform = None
        self.got_transform = False

    @property
    def translation(self) -> Optional[np.ndarray]:
        """
        Getter of transform listener TF translation as numpy array

        :return: Translation vector
        :rtype: Optional[np.ndarray]
        """
        if self.got_transform:
            trans = self.transform.transform.translation
            return np.array([trans.x, trans.y, trans.z], dtype=np.float32)
        get_logger(self.node_name).debug(
            "Did not get transform. Not applying translation"
        )
        return None

    @property
    def rotation(self) -> Optional[np.ndarray]:
        """
        Getter of transform listener TF rotation as numpy array

        :return: Rotation Quaternion vector
        :rtype: Optional[np.ndarray]
        """
        if self.got_transform:
            rot = self.transform.transform.rotation
            return np.array([rot.x, rot.y, rot.z, rot.w], dtype=np.float32)
        get_logger(self.node_name).debug("Did not get transform. Not applying rotation")
        return None

    @property
    def tf_buffer(self):
        """
        Getter of transform listener TF buffer

        :return:
        :rtype: Buffer
        """
        return self._tf_buffer

    def set_listener(self, tf_listener: TransformListener):
        """
        Set the TF listener created in the node

        :param tf_listener: Node transform listener
        :type tf_listener: TransformListener
        """
        self._tf_listener = tf_listener

    @property
    def timer(self) -> Optional[Timer]:
        """
        Timer used for the transformation lookup

        :return:
        :rtype: Timer
        """
        return self._timer

    @timer.setter
    def timer(self, node_timer: Timer) -> None:
        """
        Timer used for the transformation lookup

        :param node_timer: ROS timer from the Node
        :type node_timer: Timer
        """
        self._timer = node_timer

    def timer_callback(self):
        """
        Timer callback to performe the requested transformation lookup
        """
        # Lookup transform if the listener is set
        if self._tf_listener:
            try:
                # update the transform if found
                self.transform = self._tf_buffer.lookup_transform(
                    self.config.goal_frame, self.config.source_frame, Time()
                )
                self.got_transform = True

            except (LookupException, ConnectivityException):
                get_logger(self.node_name).debug(
                    f"Failed to get transform from {self.config.source_frame} to {self.config.goal_frame}"
                )

    def check_tf(self) -> bool:
        """
        Check if the transform is found

        :return: Transform lookup found
        :rtype: bool
        """
        return self.got_transform
