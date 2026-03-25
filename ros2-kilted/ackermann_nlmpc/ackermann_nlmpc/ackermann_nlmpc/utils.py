# Copyright (C) 2025, Georg Schildbach, Jasper Pflughaupt
# --------------------------------------------------------------------------------------------------
# This program is free software: you can redistribute it and/or modify it under the terms of the 
# GNU General Public License as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
# You should have received a copy of the GNU General Public License along with this program. If
# not, see <http://www.gnu.org/licenses/>.
# --------------------------------------------------------------------------------------------------
# Your feedback or questions or bug reports are highly welcome! Contact me if you are interested in
# using this product commercially. Please address all correspondance to: gschildbach(at)gmail.com
# --------------------------------------------------------------------------------------------------

from geometry_msgs.msg import Point, Quaternion, Pose, PoseStamped, TransformStamped
from transforms3d.euler import quat2euler, euler2quat
from tf2_geometry_msgs import do_transform_pose


def pose2yaw(pose: Pose | PoseStamped) -> float:
    """
    Extract the yaw angle from a Pose or PoseStamped message.

    :param pose: The Pose or PoseStamped message.
    :return: The yaw angle in radians.
    """
    if isinstance(pose, Pose):
        q = [pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z]
    elif isinstance(pose, PoseStamped):
        q = [pose.pose.orientation.w, pose.pose.orientation.x, pose.pose.orientation.y, pose.pose.orientation.z]
    else:
        raise TypeError(f"pose must be of type Pose or PoseStamped, got {type(pose)}")
    return quat2euler(q)[2]


def transform_2d_pose(x: float, y: float, yaw: float, transform: TransformStamped) -> tuple[float, float, float]:
    """
    Transform a 2D pose (x, y, yaw) using a TransformStamped message.

    :param x: The x coordinate of the pose.
    :param y: The y coordinate of the pose.
    :param yaw: The yaw angle of the pose in radians.
    :param transform: The TransformStamped message containing the transformation.
    :return: A tuple (x', y', yaw') representing the transformed pose.
    """
    # Convert yaw to quaternion
    q = euler2quat(0.0, 0.0, yaw)
    # Create Pose from the input parameters
    pose = Pose(
        position = Point(x=x, y=y, z=0.0),
        orientation = Quaternion(w=q[0], x=q[1], y=q[2], z=q[3])
    )
    # Transform the pose using the provided transform
    transformed_pose = do_transform_pose(pose, transform)
    # Return the transformed x, y, and yaw
    return transformed_pose.position.x, transformed_pose.position.y, pose2yaw(transformed_pose)