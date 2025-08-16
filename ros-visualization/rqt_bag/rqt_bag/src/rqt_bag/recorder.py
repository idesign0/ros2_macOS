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

"""Recorder subscribes to ROS messages and writes them to a bag file."""

try:
    from queue import Queue
    from queue import Empty
except ImportError:
    from Queue import Queue
    from Queue import Empty
import re

import sys
import threading
import time

from rclpy.clock import Clock, ClockType
from rclpy.duration import Duration
from rclpy.serialization import serialize_message
from rclpy.topic_or_service_is_hidden import topic_or_service_is_hidden
import rosbag2_py
from rosidl_runtime_py.utilities import get_message

from .qos import gen_subscriber_qos_profile, get_qos_profiles_for_topic, qos_profiles_to_yaml
from .rosbag2 import Rosbag2


class Recorder(object):

    def __init__(self, node, filename, bag_lock=None, all_topics=True,
                 topics=[], regex=False, limit=0, master_check_interval=1.0):
        """
        Subscribe to ROS messages and record them to a bag file.

        @param filename: filename of bag to write to
        @type  filename: str
        @param all_topics: all topics are to be recorded [default: True]
        @type  all_topics: bool
        @param topics: topics (or regexes if regex is True) to record [default: empty list]
        @type  topics: list of str
        @param regex: topics should be considered as regular expressions [default: False]
        @type  regex: bool
        @param limit: record only this number of messages on each topic (if non-positive, then
            unlimited) [default: 0]
        @type  limit: int
        @param master_check_interval: period (in seconds) to check master for new topic
            publications [default: 1]
        @type  master_check_interval: float
        """
        self._node = node
        self._all = all_topics
        self._topics = topics
        self._limit = limit
        self._master_check_interval = master_check_interval
        self._bag_lock = bag_lock if bag_lock else threading.Lock()
        self._listeners = []
        self._subscriber_helpers = {}
        self._limited_topics = set()
        self._failed_topics = set()
        self._last_update = time.time()
        self._write_queue = Queue()
        self._paused = False
        self._stop_condition = threading.Condition()
        self._stop_flag = False
        self._regex = regex
        self._regexes = [re.compile(t) for t in self._topics if self._regex]
        self._message_count = {}  # topic -> int (track number of messages recorded on each topic)

        self._serialization_format = 'cdr'
        self._storage_id = 'sqlite3'
        self._storage_options = rosbag2_py.StorageOptions(
            uri=filename, storage_id=self._storage_id)
        self._converter_options = rosbag2_py.ConverterOptions(
            input_serialization_format=self._serialization_format,
            output_serialization_format=self._serialization_format)
        self.rosbag_writer = rosbag2_py.SequentialWriter()
        self.rosbag_writer.open(self._storage_options, self._converter_options)
        self._bag = self._create_initial_rosbag(filename, topics)

        # Launch our two worker threads
        self._master_check_thread = threading.Thread(target=self._run_master_check)
        self._write_thread = threading.Thread(target=self._run_write)

    def _create_initial_rosbag(self, filename, topics):
        # Create any active topics in the database. Could potentially have multiple types
        # associated with this topic name, but just use the first
        topic_type_map = {}
        for topic, msg_type_names in self.get_topic_names_and_types():
            if topic in topics:
                offered_qos_profiles = ''
                qos_profiles = get_qos_profiles_for_topic(self._node, topic)
                if qos_profiles:
                    offered_qos_profiles = qos_profiles_to_yaml(qos_profiles)
                topic_metadata = rosbag2_py.TopicMetadata(
                    name=topic, type=msg_type_names[0],
                    serialization_format=self._serialization_format,
                    offered_qos_profiles=offered_qos_profiles)
                self.rosbag_writer.create_topic(topic_metadata)
                topic_type_map[topic] = topic_metadata

        return Rosbag2(filename, recording=True, topics=topic_type_map)

    def add_listener(self, listener):
        """Add a listener which gets called whenever a message is recorded.

        @param listener: function to call
        @type  listener: function taking (topic, message, time)
        """
        self._listeners.append(listener)

    def start(self):
        """Start subscribing and recording messages to bag."""
        self._master_check_thread.start()
        self._write_thread.start()

    @property
    def paused(self):
        return self._paused

    def pause(self):
        self._paused = True

    def unpause(self):
        self._paused = False

    def toggle_paused(self):
        self._paused = not self._paused

    def stop(self):
        """Stop recording."""
        with self._stop_condition:
            self._stop_flag = True
            self._stop_condition.notify_all()

        self._write_queue.put(self)

    def get_topic_names_and_types(self, include_hidden_topics=False):
        topic_names_and_types = self._node.get_topic_names_and_types()
        if not include_hidden_topics:
            topic_names_and_types = [
                (n, t) for (n, t) in topic_names_and_types
                if not topic_or_service_is_hidden(n)]
        return topic_names_and_types

    def _run_master_check(self):
        try:
            while not self._stop_flag:
                # Check for new topics
                for topic, msg_type_names in self.get_topic_names_and_types():
                    # Check if:
                    #    the topic is already subscribed to, or
                    #    we've failed to subscribe to it already, or
                    #    we've already reached the message limit, or
                    #    we don't want to subscribe
                    for msg_type_name in msg_type_names:
                        if topic in self._subscriber_helpers or \
                                topic in self._failed_topics or \
                                topic in self._limited_topics or \
                                not self._should_subscribe_to(topic):
                            continue
                        try:
                            self._message_count[topic] = 0
                            self._subscriber_helpers[topic] = _SubscriberHelper(
                                self._node, self, topic, msg_type_name)
                        except Exception as ex:
                            print('Error subscribing to %s (ignoring): %s' %
                                  (topic, str(ex)), file=sys.stderr)
                            self._failed_topics.add(topic)

                # Wait a while
                self._stop_condition.acquire()
                self._stop_condition.wait(self._master_check_interval)

        except Exception as ex:
            print('Error recording to bag: %s' % str(ex), file=sys.stderr)

        # Unsubscribe from all topics
        for topic in list(self._subscriber_helpers.keys()):
            self._unsubscribe(topic)

    def _should_subscribe_to(self, topic):
        if self._all:
            return True

        if not self._regex:
            return topic in self._topics

        for regex in self._regexes:
            if regex.match(topic):
                return True

        return False

    def _unsubscribe(self, topic):
        try:
            self._node.destroy_subscription(self, self._subscriber_helpers[topic].subscriber)
        except Exception:
            return

        del self._subscriber_helpers[topic]

    def _record(self, topic, msg, msg_type_name):
        if self._paused:
            return

        if self._limit and self._message_count[topic] >= self._limit:
            self._limited_topics.add(topic)
            self._unsubscribe(topic)
            return

        now = Clock(clock_type=ClockType.SYSTEM_TIME).now()
        self._write_queue.put((topic, msg, msg_type_name, now))
        self._message_count[topic] += 1

    def _run_write(self):
        try:
            poll_interval = 1.0
            while not self._stop_flag:
                try:
                    item = self._write_queue.get(block=False, timeout=poll_interval)
                except Empty:
                    continue

                if item == self:
                    continue

                # Write the next message to the bag
                topic, msg, msg_type_name, t = item
                with self._bag_lock:
                    self.rosbag_writer.write(topic, serialize_message(msg), t.nanoseconds)

                    # Update the overall duration for this bag based on the message just added
                    duration_ns = t.nanoseconds - self._bag.start_time.nanoseconds
                    self._bag.duration = Duration(nanoseconds=duration_ns)

                # Notify listeners that a message has been recorded
                for listener in self._listeners:
                    listener(topic, msg, t)
        except Exception as ex:
            print('Error writing to bag: %s' % str(ex), file=sys.stderr)


class _SubscriberHelper(object):

    def __init__(self, node, recorder, topic, msg_type_name):
        self.recorder = recorder
        self.topic = topic
        self.msg_type_name = msg_type_name

        # Get all of the QoS profiles for this topic
        qos_profiles = get_qos_profiles_for_topic(node, self.topic)
        if qos_profiles:
            # Select one of them to use for the subscription
            self.qos_profile = gen_subscriber_qos_profile(qos_profiles)
            self.subscriber = node.create_subscription(
                get_message(msg_type_name), topic, self.callback, self.qos_profile)

    def callback(self, msg):
        self.recorder._record(self.topic, msg, self.msg_type_name)
