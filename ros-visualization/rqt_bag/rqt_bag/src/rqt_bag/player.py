# Software License Agreement (BSD License)
#
# Copyright (c) 2012, Willow Garage, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

"""
Player listens to messages from the timeline and publishes them to ROS.
"""

import rclpy.qos

from builtin_interfaces.msg import Time
from python_qt_binding.QtCore import QObject
from rclpy import logging
from rclpy.qos import QoSProfile
from .qos import yaml_to_qos_profiles, gen_publisher_qos_profile

CLOCK_TOPIC = "/clock"


class Player(QObject):

    """
    This object handles publishing messages as the playhead passes over their position
    """

    def __init__(self, node, timeline):
        super(Player, self).__init__()
        self._node = node
        self._logger = self._node.get_logger().get_child('rqt_bag.Player')
        self.timeline = timeline

        self._publishing = set()
        self._publishers = {}

        self._publish_clock = False
        self._last_clock = Time()
        self._resume = False

    def resume(self):
        self._resume = True

    def is_publishing(self, topic):
        return topic in self._publishing

    def start_publishing(self, topic):
        if topic in self._publishing:
            return
        self._publishing.add(topic)
        self.timeline.add_listener(topic, self)

    def stop_publishing(self, topic):
        if topic not in self._publishing:
            return
        self.timeline.remove_listener(topic, self)

        if topic in self._publishers:
            self._node.destroy_publisher(self._publishers[topic])
            del self._publishers[topic]

        self._publishing.remove(topic)

    def start_clock_publishing(self):
        if CLOCK_TOPIC not in self._publishers:
            # Activate clock publishing only if the publisher was created successful
            self._publish_clock = self.create_publisher(CLOCK_TOPIC, Time)

    def stop_clock_publishing(self):
        self._publish_clock = False
        if CLOCK_TOPIC in self._publishers:
            self._node.destroy_publisher(self._publishers[topic])
            del self._publishers[CLOCK_TOPIC]

    def stop(self):
        for topic in list(self._publishing):
            self.stop_publishing(topic)
        self.stop_clock_publishing()

    def create_publisher(self, topic, ros_message, offered_qos_profiles = ""):
        try:
            # Publish based on the original recorded QoS settings
            qos_profiles = yaml_to_qos_profiles(offered_qos_profiles)
            profile_to_use = gen_publisher_qos_profile(qos_profiles)
            self._publishers[topic] = self._node.create_publisher(type(ros_message), topic, qos_profile=profile_to_use)
            return True
        except Exception as ex:
            # Any errors, stop listening/publishing to this topic
            self._logger.error(
                'Error creating publisher on topic %s for type %s. \nError text: %s' %
                (topic, str(type(ros_message)), str(ex)))
            if topic != CLOCK_TOPIC:
                self.stop_publishing(topic)
            return False

    def message_viewed(self, bag, entry):
        """
        When a message is viewed publish it
        :param bag: the bag the message is in, ''rosbag.bag''
        :param entry: the bag entry
        """
        # Don't publish unless the playhead is moving.
        if self.timeline.play_speed <= 0.0:
            return

        (ros_message, _, topic) = bag.deserialize_entry(entry)

        # Create publisher if this is the first message on the topic
        if topic not in self._publishers:
            topic_metadata = bag.get_topic_metadata(topic)
            self.create_publisher(topic, ros_message, topic_metadata.offered_qos_profiles)

        if self._publish_clock:
            time_msg = Time()
            time_msg.clock = entry.timestamp
            if self._resume or self._last_clock.clock < time_msg.clock:
                self._resume = False
                self._last_clock = time_msg
                self._publishers[CLOCK_TOPIC].publish(time_msg)

        self._publishers[topic].publish(ros_message)

    def message_cleared(self):
        pass

    def event(self, event):
        """
        This function will be called to process events posted by post_event
        it will call message_cleared or message_viewed with the relevant data
        """
        bag, entry = event.data
        if entry:
            self.message_viewed(bag, entry)
        else:
            self.message_cleared()
        return True
