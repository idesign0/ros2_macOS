#!/usr/bin/env python3
# Copyright (c) 2024-2026 Carologistics
# SPDX-License-Identifier: Apache-2.0
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

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.conditions import IfCondition
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
    PythonExpression,
    TextSubstitution,
)
from launch_ros.actions import ComposableNodeContainer, LoadComposableNodes, Node
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():

    # -----------------------------
    # Declare launch arguments
    # -----------------------------
    declare_log_level = DeclareLaunchArgument(
        'log_level', default_value='info', description='Logging level'
    )

    declare_namespace = DeclareLaunchArgument(
        'namespace', default_value='', description='Namespace of started nodes'
    )

    declare_manager_config = DeclareLaunchArgument(
        'manager_config',
        default_value='plugin_examples/file_load.yaml',
        description='Name of the CLIPS environment manager configuration',
    )

    declare_use_composition = DeclareLaunchArgument(
        'use_composition',
        default_value='False',
        description='Use composed bringup if True',
    )

    declare_start_container = DeclareLaunchArgument(
        'start_container',
        default_value='False',
        description='Start the composable node container if True and composition is used',
    )

    declare_container_name = DeclareLaunchArgument(
        'container_name',
        default_value='cx_container',
        description='The name of container that nodes will load in if use composition',
    )

    declare_package = DeclareLaunchArgument(
        'package',
        default_value='cx_bringup',
        description='The name of package where to look for the manager config',
    )

    # -----------------------------
    # Launch configurations
    # -----------------------------
    namespace = LaunchConfiguration('namespace')
    log_level = LaunchConfiguration('log_level')
    manager_config = LaunchConfiguration('manager_config')
    use_composition = LaunchConfiguration('use_composition')
    start_container = LaunchConfiguration('start_container')
    container_name = LaunchConfiguration('container_name')
    package = LaunchConfiguration('package')

    # Resolve full path to manager config
    manager_config_file = PathJoinSubstitution(
        [FindPackageShare(package), TextSubstitution(text='params/'), manager_config]
    )

    container_name_full = PathJoinSubstitution(
        [namespace, TextSubstitution(text='/'), container_name]
    )

    # -----------------------------
    # Nodes
    # -----------------------------
    load_node = Node(
        condition=IfCondition(PythonExpression(['not ', use_composition])),
        package='cx_clips_env_manager',
        executable='cx_node',
        output='screen',
        emulate_tty=True,
        namespace=namespace,
        parameters=[manager_config_file, {'autostart_node': True}],
        arguments=['--ros-args', '--log-level', log_level],
    )

    load_composable_node = GroupAction(
        condition=IfCondition(use_composition),
        actions=[
            ComposableNodeContainer(
                condition=IfCondition(start_container),
                name=container_name,
                namespace=namespace,
                package='rclcpp_components',
                executable='component_container_mt',
                emulate_tty=True,
                output='screen',
            ),
            LoadComposableNodes(
                target_container=container_name_full,
                composable_node_descriptions=[
                    ComposableNode(
                        package='cx_clips_env_manager',
                        plugin='cx::CLIPSEnvManager',
                        name='clips_manager',
                        namespace=namespace,
                        parameters=[
                            manager_config_file,
                            {'namespace': namespace, 'autostart_node': True},
                        ],
                    ),
                ],
            ),
        ],
    )

    # -----------------------------
    # Launch description
    # -----------------------------
    ld = LaunchDescription()

    ld.add_action(declare_log_level)
    ld.add_action(declare_namespace)
    ld.add_action(declare_manager_config)
    ld.add_action(declare_use_composition)
    ld.add_action(declare_start_container)
    ld.add_action(declare_container_name)
    ld.add_action(declare_package)

    ld.add_action(load_node)
    ld.add_action(load_composable_node)

    return ld
