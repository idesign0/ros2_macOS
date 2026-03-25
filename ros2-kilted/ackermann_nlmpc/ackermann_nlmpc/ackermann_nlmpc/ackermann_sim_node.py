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

from ackermann_msgs.msg import AckermannDriveStamped
from geometry_msgs.msg import TransformStamped
from nav_msgs.msg import Odometry
from std_msgs.msg import Float32

from tf2_ros import TransformBroadcaster


class AckermannSimNode(Node):
    """
    Simple simulation node for an Ackermann steering vehicle using a kinematic bicycle model.

    This subscribes to AckermannDriveStamped messages for control commands, updates a kinematic bicycle model
    and publishes the resulting odometry and steering angle.
    Only acceleration and steering angle rate from the AckermannDriveStamped message are used.
    """

    def __init__(self):
        super().__init__('ackermann_sim_node')

        # Initialize parameters
        self.declare_parameter('base_link_frame', 'base_link')
        self.declare_parameter('odom_frame', 'odom')
        self.declare_parameter('dt', 0.1)                   # Time step for simulation [s]
        self.declare_parameter('amax', 5.0)                 # Maximum acceleration [m/s^2]
        self.declare_parameter('dmax', math.pi * 1 / 4)     # Maximum steering angle [rad]
        self.declare_parameter('lf', 1.105)                 # Distance from front axle to center of mass [m]
        self.declare_parameter('lr', 1.738)                 # Distance from rear axle to center of mass [m]
        
        self._base_link_frame = self.get_parameter('base_link_frame').get_parameter_value().string_value
        self._odom_frame = self.get_parameter('odom_frame').get_parameter_value().string_value
        self._dt = self.get_parameter('dt').get_parameter_value().double_value
        self._amax = self.get_parameter('amax').get_parameter_value().double_value
        self._dmax = self.get_parameter('dmax').get_parameter_value().double_value
        self._lf = self.get_parameter('lf').get_parameter_value().double_value
        self._lr = self.get_parameter('lr').get_parameter_value().double_value
        self._l = self._lf + self._lr
        
        # Initialize state variables
        self._pos_x = 0.0
        self._pos_y = 0.0
        self._pos_yaw = 0.0
        self._velocity = 0.0
        self._steering_angle = 0.0

        # Create publishers
        self._odom_pub = self.create_publisher(Odometry, 'odom', 10)
        self._steering_angle_pub = self.create_publisher(Float32, 'steering_angle', 10)

        # Create tf broadcaster
        self._tf_broadcaster = TransformBroadcaster(self)

        # Create subscribers
        self._latest_cmd = None
        self._control_sub = self.create_subscription(AckermannDriveStamped, 'control', self._control_callback, 10)

        # Create timer
        self._timer = self.create_timer(self._dt, self._timer_callback)

    def _control_callback(self, msg: AckermannDriveStamped):
        self._latest_cmd = msg.drive

    def _timer_callback(self):
        """Update the vehicle model based on the latest command and publish the odometry and steering angle."""
        if self._latest_cmd is not None:
            # 0 speed and acceleration means brake to standstill
            if self._latest_cmd.speed == 0.0 and self._latest_cmd.acceleration == 0.0:
                a = min(self._amax, abs(self._velocity/self._dt)) * math.copysign(1, -self._velocity)
                self.get_logger().debug('Braking to standstill')
            # use acceleration from message, velocity control is not supported
            else:
                a = self._latest_cmd.acceleration
                if abs(a) > self._amax:
                    a = self._amax * math.copysign(1, a)
            ddelta = self._latest_cmd.steering_angle_velocity
        else:
            a = 0.0
            ddelta = 0.0
        self.get_logger().info(f'Applying control: a={a:.2f}, ddelta={ddelta:.2f}')

        # Update model
        beta = math.atan(math.tan(self._steering_angle) * self._lr / (self._l))
        self._pos_x += self._velocity * math.cos(self._pos_yaw + beta) * self._dt
        self._pos_y += self._velocity * math.sin(self._pos_yaw + beta) * self._dt
        self._pos_yaw += (self._velocity / self._l) * math.cos(beta) * math.tan(self._steering_angle) * self._dt
        self._pos_yaw %= (2 * math.pi)
        self._velocity += a * self._dt
        self._steering_angle += ddelta * self._dt
        self._steering_angle = max(self._steering_angle, -self._dmax)
        self._steering_angle = min(self._steering_angle, self._dmax)

        # Publish odometry
        t = self.get_clock().now().to_msg()
        odom_msg = Odometry()
        odom_msg.header.stamp = t
        odom_msg.header.frame_id = self._odom_frame
        odom_msg.child_frame_id = self._base_link_frame
        odom_msg.pose.pose.position.x = self._pos_x
        odom_msg.pose.pose.position.y = self._pos_y
        odom_msg.pose.pose.position.z = 0.0
        odom_msg.pose.pose.orientation.x = 0.0
        odom_msg.pose.pose.orientation.y = 0.0
        odom_msg.pose.pose.orientation.z = math.sin(self._pos_yaw / 2.0)
        odom_msg.pose.pose.orientation.w = math.cos(self._pos_yaw / 2.0)
        odom_msg.twist.twist.linear.x = self._velocity
        odom_msg.twist.twist.linear.y = 0.0
        odom_msg.twist.twist.linear.z = 0.0
        odom_msg.twist.twist.angular.x = 0.0
        odom_msg.twist.twist.angular.y = 0.0
        odom_msg.twist.twist.angular.z = 0.0
        self._odom_pub.publish(odom_msg)

        # Publish steering angle
        steering_angle_msg = Float32()
        steering_angle_msg.data = self._steering_angle
        self._steering_angle_pub.publish(steering_angle_msg)

        # Publish tf transform
        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = self._odom_frame
        t.child_frame_id = self._base_link_frame
        t.transform.translation.x = self._pos_x
        t.transform.translation.y = self._pos_y
        t.transform.translation.z = 0.0
        t.transform.rotation.x = 0.0
        t.transform.rotation.y = 0.0
        t.transform.rotation.z = math.sin(self._pos_yaw / 2.0)
        t.transform.rotation.w = math.cos(self._pos_yaw / 2.0)
        self._tf_broadcaster.sendTransform(t)

        self.get_logger().info(f'Published odometry: x={self._pos_x:.2f}, y={self._pos_y:.2f}, yaw={self._pos_yaw:.2f}, v={self._velocity:.2f}, delta={self._steering_angle:.2f}')


def main(args=None):
    rclpy.init(args=args)
    node = AckermannSimNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()


if __name__ == '__main__':
    main()