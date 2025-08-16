# Software License Agreement (BSD License)
#
# Copyright (c) 2019, PickNik Consulting.
# Copyright (c) 2020, Open Source Robotics Foundation, Inc.
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

"""A rosbag abstraction with functionality required by rqt_bag."""

from collections import namedtuple
import os
import pathlib
import sqlite3

from rclpy.clock import Clock, ClockType
from rclpy.duration import Duration
from rclpy.serialization import deserialize_message
from rclpy.time import Time
import rosbag2_py
from rosidl_runtime_py.utilities import get_message
import yaml


class Rosbag2:

    def __init__(self, bag_path, recording=False, topics={},
                 serialization_format='cdr', storage_id='sqlite3'):
        self.bag_path = bag_path

        if recording:
            # If we're recording, the new rosbag doesn't have a metadata.yaml file yet, since
            # it is written out when the database is closed. So, let's initialize based on the
            # caller-supplied topic information and whatever else we can infer

            # Construct a full path to the db3 file
            bag_name = os.path.split(bag_path)[1]
            database_relative_name = bag_name + '_0.db3'

            # Set the fields we need
            self.db_name = os.path.join(self.bag_path, database_relative_name)
            self.start_time = Clock(clock_type=ClockType.SYSTEM_TIME).now()
            self.duration = Duration(nanoseconds=0)
            self.topic_metadata_map = topics
        else:
            # Open the metadata file and grab the fields we need
            with open(bag_path + '/metadata.yaml') as f:
                full_bag_info = yaml.safe_load(f)
                bag_info = full_bag_info['rosbag2_bagfile_information']
                database_relative_name = bag_info['relative_file_paths'][0]

                self.db_name = os.path.join(self.bag_path, database_relative_name)
                self.start_time = Time(nanoseconds=bag_info['starting_time']
                                       ['nanoseconds_since_epoch'])
                self.duration = Duration(nanoseconds=bag_info['duration']['nanoseconds'])
                self.topic_metadata_map = {
                    topic['topic_metadata']['name']:
                    rosbag2_py.TopicMetadata(
                        name=topic['topic_metadata']['name'],
                        type=topic['topic_metadata']['type'],
                        serialization_format=topic['topic_metadata']['serialization_format'],
                        offered_qos_profiles=topic['topic_metadata']['offered_qos_profiles'])
                    for topic in bag_info['topics_with_message_count']
                }

    def size(self):
        """Get the size of the rosbag."""
        return sum(f.stat().st_size
                   for f in pathlib.Path(self.bag_path).glob('**/*') if f.is_file())

    def get_earliest_timestamp(self):
        """Get the timestamp of the earliest message in the bag."""
        return self.start_time

    def get_latest_timestamp(self):
        """Get the timestamp of the most recent message in the bag."""
        return self.start_time + self.duration

    def get_topics(self):
        """Get all of the topics used in this bag."""
        return sorted(self.topic_metadata_map.keys())

    def get_topic_type(self, topic):
        """Get the topic type for a given topic name."""
        if topic not in self.topic_metadata_map:
            return None
        return self.topic_metadata_map[topic].type

    def get_topic_metadata(self, topic):
        """Get the full metadata for a given topic name."""
        if topic not in self.topic_metadata_map:
            return None
        return self.topic_metadata_map[topic]

    def get_topics_by_type(self):
        """Return a map of topic data types to a list of topics publishing that type."""
        topics_by_type = {}
        for name, topic in self.topic_metadata_map.items():
            topics_by_type.setdefault(topic.type, []).append(name)
        return topics_by_type

    def get_entry(self, timestamp, topic=None):
        """Get the (serialized) entry for a specific timestamp.

        Returns the entry that is closest in time (<=) to the provided timestamp.
        """
        sql_query = 'timestamp<={} ORDER BY messages.timestamp ' \
                    'DESC LIMIT 1;'.format(timestamp.nanoseconds)
        result = self._execute_sql_query(sql_query, topic)
        return result[0] if result else None

    def get_entry_after(self, timestamp, topic=None):
        """Get the next entry after a given timestamp."""
        sql_query = 'timestamp>{} ORDER BY messages.timestamp ' \
                    'LIMIT 1;'.format(timestamp.nanoseconds)
        result = self._execute_sql_query(sql_query, topic)
        return result[0] if result else None

    def get_entries_in_range(self, t_start, t_end, topic=None):
        """Get a list of all of the entries within a given range of timestamps (inclusive)."""
        sql_query = 'timestamp>={} AND timestamp<={} ' \
                    'ORDER BY messages.timestamp;'.format(t_start.nanoseconds, t_end.nanoseconds)
        return self._execute_sql_query(sql_query, topic)

    def deserialize_entry(self, entry):
        """Deserialize a bag entry into its corresponding ROS message."""
        msg_type_name = self.get_topic_type(entry.topic)
        msg_type = get_message(msg_type_name)
        ros_message = deserialize_message(entry.data, msg_type)
        return (ros_message, msg_type_name, entry.topic)

    def _execute_sql_query(self, sql_query, topic):
        Entry = namedtuple('Entry', ['topic', 'data', 'timestamp'])
        base_query = 'SELECT topics.name, data, timestamp FROM messages ' \
                     'JOIN topics ON messages.topic_id = topics.id WHERE '

        # If there was a topic requested, make sure it is in this bag
        if topic is not None:
            if topic not in self.topic_metadata_map:
                return []
            base_query += 'topics.name="{}"AND '.format(topic)

        with sqlite3.connect(self.db_name) as db:
            cursor = db.cursor()
            entries = cursor.execute(base_query + sql_query).fetchall()
            cursor.close()
            return [Entry(*entry) for entry in entries]
