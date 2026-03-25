#!/usr/bin/env python3
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

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution


def generate_launch_description():
    # -----------------------------
    # Launch arguments
    # -----------------------------
    declare_package = DeclareLaunchArgument(
        'package',
        default_value='cx_pddl_bringup',
        description='The name of package where to look for the manager config',
    )
    declare_manager_config = DeclareLaunchArgument(
        'manager_config',
        default_value='pddl_agents/cx_pddl_clips_agent.yaml',
        description='Name of the CLIPS environment manager configuration',
    )

    package = LaunchConfiguration('package')
    manager_config = LaunchConfiguration('manager_config')

    # -----------------------------
    # Paths to other launch files
    # -----------------------------
    pddl_manager_launch = PathJoinSubstitution(
        [get_package_share_directory('cx_pddl_manager'), 'launch', 'pddl_manager.launch.py']
    )

    cx_launch_file = PathJoinSubstitution(
        [get_package_share_directory('cx_bringup'), 'launch', 'cx_launch.py']
    )

    # -----------------------------
    # Include launch descriptions
    # -----------------------------
    include_pddl_manager = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(pddl_manager_launch)
    )

    include_cx_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(cx_launch_file),
        launch_arguments={
            'package': package,
            'manager_config': manager_config,
        }.items(),
    )

    # -----------------------------
    # Build LaunchDescription
    # -----------------------------
    ld = LaunchDescription()
    ld.add_action(declare_package)
    ld.add_action(declare_manager_config)
    ld.add_action(include_pddl_manager)
    ld.add_action(include_cx_launch)

    return ld
