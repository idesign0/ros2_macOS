# Copyright 2023 Nick Morales.
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

"""
An example launch file for running an integration test with catch_ros2.

It runs an auxiliary test node (the node under test), and an integration
test node (the node which performs the test).
"""

from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_catch_ros2 import Catch2IntegrationTestNode, Catch2LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    """Launch example integration test."""
    # Catch2LaunchDescription:
    # a wrapper around LaunchDescription which adds a required "result_file"
    # argument for the launch file. This file will hold the results of the test.
    return Catch2LaunchDescription([
        # Specific launch arguments can also be included at the user's discretion
        DeclareLaunchArgument(
            name='test_duration',
            default_value='2.0',
            description='Max length of test in seconds.',
        ),
        # Auxiliary nodes can be run like normal to test integration between nodes
        Node(
            package='catch_ros2',
            executable='integration_aux_node',
        ),
        # Catch2IntegrationTestNode:
        # a wrapper around Node which passes the "result_file" argument to Catch2.
        # There should only be one integration test node. This node will shutdown
        # the entire launch file when it exits.
        # Specific parameters and other arguments can also be passed, like the
        # "test_duration" example below.
        Catch2IntegrationTestNode(
            package='catch_ros2',
            executable='integration_test_node',
            parameters=[{
                'test_duration': LaunchConfiguration('test_duration'),
            }],
        ),
    ])
