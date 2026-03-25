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

import math

import rclpy
from rclpy.node import Node
from rclpy.time import Time

from ackermann_nlmpc_msgs.msg import Trajectory, TrajectoryNode
from geometry_msgs.msg import PoseStamped
from nav_msgs.msg import Path

from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_geometry_msgs import do_transform_pose

from .utils import pose2yaw


class TrajectoryConverterNode(Node):
    """Node for converting Path and goal pose messages to Trajectory message format. Includes basic direction change detection."""

    def __init__(self):
        super().__init__('trajectory_converter_node')

        self.declare_parameter('base_link_frame', 'base_link')
        self.declare_parameter('max_nodes', 1000)
        self.declare_parameter('default_velocity', 2.0)
        self.declare_parameter('default_pathwidth_l', 1.0)
        self.declare_parameter('default_pathwidth_r', 1.0)

        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, self)

        self.create_subscription(Path, 'path', self._cb_path, 10)
        self.create_subscription(PoseStamped, 'goal_pose', self._cb_goal_pose, 10)

        self.traj_pub = self.create_publisher(Trajectory, 'trajectory', 10)

    def _cb_path(self, msg: Path):
        if msg.poses is None or len(msg.poses) == 0:
            self.get_logger().error('Path does not contain any poses.')
            return

        self._make_trajectory(msg)

    def _cb_goal_pose(self, msg: PoseStamped):
        self.get_logger().debug('Received goal pose data.')

        # Use pose as one-element and update the reference trajectory
        path = Path()
        path.header.stamp = msg.header.stamp
        path.header.frame_id = msg.header.frame_id
        path.poses = [msg]
        self._make_trajectory(path)

    def _make_trajectory(self, path: Path):

        max_nodes = self.get_parameter('max_nodes').get_parameter_value().integer_value
        v = self.get_parameter('default_velocity').get_parameter_value().double_value
        pw_l = self.get_parameter('default_pathwidth_l').get_parameter_value().double_value
        pw_r = self.get_parameter('default_pathwidth_r').get_parameter_value().double_value
        base_link_frame = self.get_parameter('base_link_frame').get_parameter_value().string_value

        transforms = {}
        poses : list[PoseStamped] = []
        for pose in path.poses:
            # Transform pose to base_link frame
            pose_t = PoseStamped()
            pose_t.header.stamp = pose.header.stamp
            pose_t.header.frame_id = base_link_frame
            if pose.header.frame_id == base_link_frame:
                pose_t.pose = pose.pose
            else:
                if pose.header.frame_id not in transforms:
                    # Lookup transform from pose frame to base_link frame
                    transforms[pose.header.frame_id] = self._tf_buffer.lookup_transform(
                        source_frame=pose.header.frame_id,
                        target_frame=base_link_frame,
                        time=Time()
                    )
                pose_t.pose = do_transform_pose(
                    pose.pose,
                    transforms[pose.header.frame_id]
                )
            # Store transformed pose
            poses.append(pose_t)

        # If the first pose is not close enough to the trajectory origin, add a node at the origin
        if abs(poses[0].pose.position.x) > 1e-3 or abs(poses[0].pose.position.y) > 1e-3 or abs(pose2yaw(poses[0].pose)) > 1e-3:
            p = PoseStamped()
            p.header.frame_id = base_link_frame
            p.header.stamp = path.header.stamp if path.header.stamp else self.get_clock().now().to_msg()
            poses.insert(0, p)

        # Use current pose for trajectory header
        traj = Trajectory()
        traj.header.frame_id = self.get_parameter('base_link_frame').get_parameter_value().string_value
        traj.header.stamp = path.header.stamp if path.header.stamp else self.get_clock().now().to_msg()
        traj.x0 = 0.0
        traj.y0 = 0.0
        traj.heading0 = 0.0
        traj.type = 0
        traj.nodes = []

        drd = 1 # driving direction (0: standstill, 1: forward, 2: backward)

        for i in range(len(poses)):
            pose = poses[i]

            # convert pose to trajectory node
            node = TrajectoryNode()
            node.header = pose.header
            node.x = pose.pose.position.x
            node.y = pose.pose.position.y
            node.heading = pose2yaw(pose.pose)
            node.v = v
            node.a = 0.0
            node.delta = 0.0
            node.beta = 0.0
            node.mode = drd if i > 0 else 0
            node.pw_l = pw_l
            node.pw_r = pw_r

            # if there is a next pose, check for direction change
            if i + 1 < len(poses):
                next_pose = poses[i + 1]
                direction_change = self._check_direction_change(pose, next_pose, moving_forward=(drd == 1))

                if direction_change:
                    # set current node to standstill
                    node.mode = 0
                    # and set the next node to the opposite driving direction
                    if drd == 1:
                        drd = 2
                    else:
                        drd = 1

            traj.nodes.append(node)

            if len(traj.nodes) >= max_nodes:
                break

        self.traj_pub.publish(traj)

    def _check_direction_change(self, pose_1: PoseStamped, pose_2: PoseStamped, moving_forward: bool) -> bool:
        """
        Check if a direction change occurs between two poses, i.e., whether the vehicle should change from moving forward to backward or vice versa.

        :param pose_1: The first pose.
        :param pose_2: The second pose.
        :param moving_forward: Whether the vehicle is currently moving forward (True) or backward (False).
        :return: True if a direction change occurs, False otherwise.
        :raises ValueError: If the frames of the poses do not match.
        """
        if pose_1.header.frame_id != pose_2.header.frame_id:
            raise ValueError("Pose frames do not match.")
        
        # Check if both headers point in the same direction
        yaw_1 = pose2yaw(pose_1.pose)
        yaw_2 = pose2yaw(pose_2.pose)
        angle_diff = (yaw_2 - yaw_1 + math.pi) % (2 * math.pi) - math.pi
        # If heading_1 == heading_2 +- 180 deg, count as same direction
        same_direction = abs(angle_diff) < math.pi / 2

        # Check if pose_2 is in front of pose_1 (in 180 deg cone forward from pose_1)
        dx = pose_2.pose.position.x - pose_1.pose.position.x
        dy = pose_2.pose.position.y - pose_1.pose.position.y
        angle_to_pose_2 = math.atan2(dy, dx)
        angle_diff = (angle_to_pose_2 - yaw_1 + math.pi) % (2 * math.pi) - math.pi
        in_front = abs(angle_diff) < math.pi / 2

        if moving_forward:
            return same_direction and not in_front
        else:
            return same_direction and in_front


def main(args=None):
    rclpy.init(args=args)
    node = TrajectoryConverterNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()


if __name__ == '__main__':
    main()