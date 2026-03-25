# -----------------------------------------------------------------------------
# Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
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
#

import launch
from launch.actions import DeclareLaunchArgument as LaunchArg
from launch.actions import OpaqueFunction
from launch.substitutions import LaunchConfiguration as LaunchConfig
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def launch_setup(context, *args, **kwargs):
    """Create composable node."""
    trans = LaunchConfig('image_transport').perform(context)
    trans_suffix = '' if trans == 'raw' else '/' + trans
    img_src = 'image' + trans_suffix
    img_trg = LaunchConfig('image').perform(context) + trans_suffix
    container = ComposableNodeContainer(
        name='apriltag_draw_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='apriltag_draw',
                plugin='apriltag_draw::ApriltagDraw',
                namespace=LaunchConfig('camera'),
                parameters=[{'image_transport': trans, 'max_queue_size': 200}],
                remappings=[(img_src, img_trg), ('tags', LaunchConfig('tags'))],
                extra_arguments=[{'use_intra_process_comms': True}],
            )
        ],
        output='screen',
    )
    return [container]


def generate_launch_description():
    """Create composable node by calling opaque function."""
    return launch.LaunchDescription(
        [
            LaunchArg(
                'camera',
                default_value=['camera'],
                description='camera name (base topic w/o image)',
            ),
            LaunchArg(
                'image',
                default_value=['image_raw'],
                description="image topic name, e.g. 'image_raw'",
            ),
            LaunchArg('tags', default_value=['tags'], description="tag topic name, e.g. 'tags'"),
            LaunchArg(
                'image_transport',
                default_value=['raw'],
                description="image transport to use, e.g. 'raw'",
            ),
            OpaqueFunction(function=launch_setup),
        ]
    )
