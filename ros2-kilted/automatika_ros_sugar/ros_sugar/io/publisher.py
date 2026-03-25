"""ROS Publishers"""

from rclpy.clock import Clock, ClockType
from typing import Any, Callable, Optional, Union, List
from socket import socket

from rclpy.logging import get_logger
from rclpy.publisher import Publisher as ROSPublisher

from std_msgs.msg import Header

from . import utils


class Publisher:
    """Publisher."""

    def __init__(self, output_topic, node_name: str = '') -> None:
        """__init__.

        :param input_topic:
        :type topic: Input
        :rtype: None
        """

        self.output_topic = output_topic

        # Node name can be changed to a node that the callback is executed in
        # at the time of setting subscriber using set_node_name
        self.node_name: str = node_name

        self._publisher: Optional[ROSPublisher] = None
        self._pre_processors: Optional[List[Union[Callable, socket]]] = None

    def set_node_name(self, node_name: str) -> None:
        """Set node name.

        :param node_name:
        :type node_name: str
        :rtype: None
        """
        self.node_name = node_name

    def set_publisher(self, publisher: ROSPublisher) -> None:
        """set_publisher.

        :param publisher: Publisher
        :rtype: None
        """
        self._publisher = publisher

    def add_pre_processors(self, processors: List[Union[Callable, socket]]):
        """Add a pre processor for publisher message

        :param method: Pre processor methods or sockets
        :type method: Callable
        """
        self._pre_processors = processors

    def _prepare_for_publish(self, *output) -> Any:
        """Prepare the output for publishing by applying the pre-processors

        :return: Pre-processed output rerady for converting and publishing
        :rtype: Any
        """
        output_types = [type(arg) for arg in output]
        if self._pre_processors:
            for processor in self._pre_processors:
                pre_output = utils.run_external_processor(
                    self.node_name, self.output_topic.name, processor, *output
                )
                # if any processor output is None, then dont publish
                if pre_output is None:
                    return None
                pre_output_types = [type(arg) for arg in pre_output]
                # type check processor output if incorrect, raise an error
                if not all(
                    out_type == pre_output_type
                    for out_type, pre_output_type in zip(
                        output_types, pre_output_types, strict=True
                    )
                ):
                    get_logger(self.node_name).warn(
                        f"The output produced by the component for topic {self.output_topic.name} is of type {output}. Got pre_processor output of type {pre_output_types}"
                    )
                # if all good, set output equal to post output
                output = pre_output
        return output

    def publish(
        self,
        *output,
        frame_id: Optional[str] = None,
        **kwargs,
    ) -> None:
        """
        Publish using the publisher

        :param output: ROS message to publish
        :type output: Any
        """
        # Apply any output pre_processors sequentially before publishing, if defined
        if self._publisher is None:
            get_logger(self.node_name).error(
                f"No valid ROS2 publisher is available for topic {self.output_topic.name}.. Skipping message publishing"
            )
            return
        output = self._prepare_for_publish(*output)
        if output is None:
            get_logger(self.node_name).error(
                f"No valid output found for topic {self.output_topic.name}.. Skipping message publishing"
            )
            return
        msg = self.output_topic.msg_type.convert(*output, **kwargs)
        if msg:
            if frame_id and not hasattr(msg, "header"):
                get_logger(self.node_name).debug(
                    f"Cannot add a header to non-stamped message of type '{type(msg)}'"
                )
            elif hasattr(msg, "header"):
                # Add a header
                msg.header = Header()
                msg.header.frame_id = frame_id or msg.header.frame_id or ""
                msg.header.stamp = Clock(clock_type=ClockType.ROS_TIME).now().to_msg()
            self._publisher.publish(msg)
