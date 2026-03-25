# Copyright (C) 2023 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from ._transport import Node as _Node
from ._transport import *
import sys
from typing import TypeVar, Callable
import traceback

# The "ProtoMsg" TypeVar represents an actual msg of a protobuf type.
# On the other hand, the "ProtoMsgType" TypeVar represents the protobuf type.
# For example, let's take the following code:
#   from gz.msgs11.stringmsg_pb2 import StringMsg
#   proto_msg = StringMsg()
# The variable `proto_msg` would be the expected input for a "ProtoMsg"
# type argument and `StringMsg` would be the expected input for a
# "ProtoMsgType" type argument.
ProtoMsg = TypeVar("ProtoMsg")
ProtoMsgType = TypeVar("ProtoMsgType")


class Publisher(_Node.Publisher):
    """
    A wrapper class representing extends the _Node.Publisher class for
    in order to publish serialized protocol buffer messages.

    """

    def publish(self, proto_msg: ProtoMsg):
        """
        Publishes a serialized protocol buffer message.

        Args:
            proto_msg (ProtoMsg): The protocol buffer message to be published.

        Returns:
            None

        """
        msg_string = proto_msg.SerializeToString()
        msg_type = proto_msg.DESCRIPTOR.full_name
        return self.publish_raw(msg_string, msg_type)


class Node(_Node):
    """
    A wrapper class that extends the _Node class for managing
    publishers, subscribers and service request.
    """

    def advertise(
        self, topic: str, msg_type: ProtoMsg, options=_transport.AdvertiseMessageOptions()
    ):
        """
        Advertises a topic for publishing messages of a specific type.

        Args:
            topic (str): The name of the topic to advertise.
            msg_type (ProtoMsg): The type of the messages to be published.
            options (AdvertiseMessageOptions): Options for advertising
              the topic. Defaults to AdvertiseMessageOptions().

        Returns:
            Publisher: A publisher object associated with the advertised topic.

        """
        return Publisher(
            _Node.advertise(self, topic, msg_type.DESCRIPTOR.full_name,
                            options)
        )

    def subscribe(
        self,
        msg_type: ProtoMsg,
        topic: str,
        callback: Callable,
        options=_transport.SubscribeOptions(),
    ):
        """
        Subscribes to a topic to receive messages of a specific type and
        invokes a callback function for each received message.

        Args:
            msg_type (ProtoMsg): The type of the messages to subscribe to.
            topic (str): The name of the topic to subscribe to.
            callback (Callable): The callback function to be invoked
            options (SubscribeOptions): Options for subscribing to
              the topic. Defaults to SubscribeOptions().

        Returns:
            None.

        """

        def cb_deserialize(proto_msg, msg_info):
            # The callback might throw an exception and there's nothing else to
            # catch since this will be in its own thread. This could cause
            # crashes when used in gz-sim python systems, so we'll catch and
            # print a traceback here.
            try:
                deserialized_msg = msg_type()
                deserialized_msg.ParseFromString(proto_msg)
                callback(deserialized_msg)
            except Exception as e:
                print(traceback.format_exc(), sys.stderr)

        return self.subscribe_raw(
            topic, cb_deserialize, msg_type.DESCRIPTOR.full_name, options
        )

    def request(
        self,
        service: str,
        request: ProtoMsg,
        request_type: ProtoMsgType,
        response_type: ProtoMsgType,
        timeout: int,
    ):
        """
        Sends a request of a specific type to a service and waits for a
        response of a specific type.

        Args:
            service (str): The name of the service to send the request to.
            request (ProtoMsg): The request message to be sent.
            request_type (ProtoMsgType): The type of the request.
            response_type (ProtoMsgType): The expected type of the response.
            timeout (int): The maximum time (in ms) to wait for a response.

        Returns:
            tuple: A tuple containing the result of the request (bool) and the
              deserialized response message.

        """
        result, serialized_response = self.request_raw(
            service,
            request.SerializeToString(),
            request_type.DESCRIPTOR.full_name,
            response_type.DESCRIPTOR.full_name,
            timeout,
        )
        deserialized_response = response_type()
        deserialized_response.ParseFromString(serialized_response)
        return result, deserialized_response
