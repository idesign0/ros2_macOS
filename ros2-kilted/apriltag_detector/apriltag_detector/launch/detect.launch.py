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
        name='apriltag_detector_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='apriltag_detector',
                plugin='apriltag_detector::DetectorComponent',
                namespace=LaunchConfig('camera'),
                parameters=[
                    {
                        'type': LaunchConfig('type'),
                        'black_border_width': LaunchConfig('black_border_width'),
                        'blur': LaunchConfig('blur'),
                        'decimate_factor': LaunchConfig('decimate_factor'),
                        'image_transport': trans,
                        'max_allowed_hamming_distance': LaunchConfig(
                            'max_allowed_hamming_distance'
                        ),
                        'num_threads': LaunchConfig('num_threads'),
                        'tag_family': LaunchConfig('tag_family'),
                    }
                ],
                remappings=[(img_src, img_trg), ('tags', LaunchConfig('tags'))],
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
            ComposableNode(
                package='apriltag_draw',
                plugin='apriltag_draw::ApriltagDraw',
                namespace=LaunchConfig('camera'),
                parameters=[{'image_transport': trans, 'max_queue_size': 200}],
                remappings=[(img_src, img_trg), ('tags', LaunchConfig('tags'))],
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
        ],
        output='screen',
    )
    return [container]


def generate_launch_description():
    """Create composable node by calling opaque function."""
    return launch.LaunchDescription(
        [
            LaunchArg(
                'black_border_width',
                default_value=['1'],
                description='width of black border (MIT only)',
            ),
            LaunchArg('blur', default_value=['0.0'], description='gaussian blur'),
            LaunchArg('camera', default_value=['camera'], description='camera name'),
            LaunchArg('decimate_factor', default_value=['1.0'], description='decimation factor'),
            LaunchArg(
                'image',
                default_value=['image_raw'],
                description="image topic name, e.g. 'image_raw'",
            ),
            LaunchArg(
                'image_transport', default_value=['raw'], description='input image transport'
            ),
            LaunchArg(
                'max_allowed_hamming_distance',
                default_value=['0'],
                description='maximum allowed hamming distance',
            ),
            LaunchArg('tags', default_value=['tags'], description="tag topic name, e.g. 'tags'"),
            LaunchArg('num_threads', default_value=['1'], description='number of threads to use'),
            LaunchArg('tag_family', default_value=['tf36h11'], description='tag family'),
            LaunchArg(
                'type', default_value=['umich'], description='type of detector (umich, mit)'
            ),
            OpaqueFunction(function=launch_setup),
        ]
    )
