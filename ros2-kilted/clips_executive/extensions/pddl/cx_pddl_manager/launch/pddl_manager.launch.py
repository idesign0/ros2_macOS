#!/bin/env/python
# Copyright (c) 2025-2026 Carologistics
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

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import LifecycleNode
from rclpy.logging import get_logger


def launch_with_context(context, *args, **kwargs):
    bringup_dir = get_package_share_directory('cx_pddl_manager')

    LaunchConfiguration('namespace')
    config = LaunchConfiguration('config')
    config_file = os.path.join(bringup_dir, '', config.perform(context))
    # re-issue warning as it is not colored otherwise ...
    if not os.path.isfile(config_file):
        logger = get_logger('cx_pddl_manager_launch')
        logger.warning(f'Parameter file path is not a file: {config_file}')

    LaunchConfiguration('log_level')
    pddl_manager_node = LifecycleNode(
        package='cx_pddl_manager',
        executable='pddl_manager',
        name='pddl_manager',
        namespace='',
        emulate_tty=True,
        output='screen',
        parameters=[config_file],
    )
    return [pddl_manager_node]


def generate_launch_description():

    declare_log_level_ = DeclareLaunchArgument(
        'log_level',
        default_value='info',
        description='Logging level for cx_node executable',
    )

    declare_namespace_ = DeclareLaunchArgument(
        'namespace', default_value='', description='Default namespace'
    )

    declare_config = DeclareLaunchArgument(
        'config',
        default_value='params/pddl_manager.yaml',
        description='Name of the configuration file.',
    )

    # The lauchdescription to populate with defined CMDS
    ld = LaunchDescription()

    ld.add_action(declare_log_level_)

    ld.add_action(declare_namespace_)
    ld.add_action(declare_config)
    ld.add_action(OpaqueFunction(function=launch_with_context))

    return ld
