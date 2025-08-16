# Copyright (c) 2020, Open Source Robotics Foundation, Inc.
# All rights reserved.
#
# Software License Agreement (BSD License 2.0)
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
#  * Neither the name of the copyright holder nor the names of its
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
QoS-related utility functions
"""

import yaml
import math
import rclpy.qos
import builtin_interfaces.msg

from rclpy.qos import QoSProfile
from rclpy.duration import Duration
from rclpy.time import Time


def duration_to_node(duration):
    t = Time(nanoseconds=duration.nanoseconds)
    node = {}
    (node['sec'], node['nsec']) = t.seconds_nanoseconds()
    return node


def node_to_duration(node):
    return Duration(seconds=int(node['sec']), nanoseconds=int(node['nsec']))


def qos_profiles_to_yaml(qos_profiles):
    profiles_list = []
    for qos_profile in qos_profiles:
        qos = {}
        qos['history'] = int(qos_profile.history)
        qos['depth'] = int(qos_profile.depth)
        qos['reliability'] = int(qos_profile.reliability)
        qos['durability'] = int(qos_profile.durability)
        qos['lifespan'] = duration_to_node(qos_profile.lifespan)
        qos['deadline'] = duration_to_node(qos_profile.deadline)
        qos['liveliness'] = int(qos_profile.liveliness)
        qos['liveliness_lease_duration'] = duration_to_node(qos_profile.liveliness_lease_duration)
        qos['avoid_ros_namespace_conventions'] = qos_profile.avoid_ros_namespace_conventions
        profiles_list.append(qos)

    return yaml.dump(profiles_list, sort_keys=False)


def yaml_to_qos_profiles(profiles_yaml):
    qos_profiles = []
    nodes = yaml.safe_load(profiles_yaml)
    if nodes is None:
        return []
    for node in nodes:
        qos_profile = QoSProfile(depth=int(node['depth']))
        qos_profile.history = int(node['history'])
        qos_profile.reliability = int(node['reliability'])
        qos_profile.durability = int(node['durability'])
        qos_profile.lifespan = node_to_duration(node['lifespan'])
        qos_profile.deadline = node_to_duration(node['deadline'])
        qos_profile.liveliness = int(node['liveliness'])
        qos_profile.liveliness_lease_duration = node_to_duration(node['liveliness_lease_duration'])
        qos_profile.avoid_ros_namespace_conventions = node['avoid_ros_namespace_conventions']
        qos_profiles.append(qos_profile)

    return qos_profiles


def gen_publisher_qos_profile(qos_profiles):
    """Generate a single QoS profile for a publisher from a set of QoS profiles (typically
       read from the offered_qos_profiles in the rosbag, which records the QoS settings used
       by the various publishers on that topic)."""
    if not qos_profiles:
        return QoSProfile(depth=10)

    # Simply use the first one (should have a more sophisticated strategy)
    result = qos_profiles[0]

    # UNKNOWN isn't a valid QoS history policy for a publisher
    if result.history == rclpy.qos.HistoryPolicy.UNKNOWN:
        result.history = rclpy.qos.HistoryPolicy.SYSTEM_DEFAULT
        result.depth = 10

    return result


def gen_subscriber_qos_profile(qos_profiles):
    """Generate a single QoS profile for a subscriber from a set of QoS profiles (typically
       acquired from an active set of publishers for a particular topic)."""
    if not qos_profiles:
        return QoSProfile(depth=10)

    # Simply use the first one (should have a more sophisticated strategy)
    result = qos_profiles[0]

    # UNKNOWN isn't a valid QoS history policy for a subscriber
    if result.history == rclpy.qos.HistoryPolicy.UNKNOWN:
        result.history = rclpy.qos.HistoryPolicy.SYSTEM_DEFAULT
        result.depth = 10

    return result


def get_qos_profiles_for_topic(node, topic):
    """Get the QoS profiles used by current publishers on a specific topic"""
    publishers_info = node.get_publishers_info_by_topic(topic)
    if publishers_info:
        # Get the QoS info for each of the current publishers on this topic
        qos_profiles = []
        for pinfo in publishers_info:
            qos_profiles.append(pinfo.qos_profile)
        return qos_profiles

    return None
