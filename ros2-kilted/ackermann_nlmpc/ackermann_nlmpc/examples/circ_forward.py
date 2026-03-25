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

from ackermann_nlmpc_msgs.msg import Trajectory, TrajectoryNode


class TrajectoryGeneratorNode(Node):
    def __init__(self):
        super().__init__('traj_generator_node')
        self.traj_pub = self.create_publisher(Trajectory, 'ackermann_nlmpc/trajectory', 10)

    def exec(self):
        traj = self.generate_trajectory()
        self.traj_pub.publish(traj)

    def generate_trajectory(self) -> Trajectory:
        t0 = self.get_clock().now()

        # Parameters
        Nn = 200  # number of trajectory nodes
        vref = 10.0  # Reference velocity [m/s]
        pwidth = 2.0  # Width of the trajectory [m]

        # Vehicle parameters
        lf = 1.105
        lr = 1.738

        # Trajectory parameters
        radius = 30.0 # circle radius [m]
        alpha = 10 * math.pi/180 # angular size of obstacles [rad]
        beta = 20 * math.pi/180 # angular size of transition region [rad]
        Oin = 3.0 # protrusion into path, for inside passing [m]
        Oout = 2.0 # protrusion into path, for outside passing [m]

        frame_id = 'base_link'

        # Initialize trajectory
        traj = Trajectory()
        traj.header.frame_id = frame_id
        traj.header.stamp = t0.to_msg()
        traj.type = 2 # 0: trajectory, 1: path, 2: circular path
        traj.nodes = []

        # Generate trajectory nodes
        for i in range(0, Nn):
            phi = i * 2 * math.pi / Nn
            x = radius * math.sin(phi)
            y = radius * (1 - math.cos(phi))
            delta = math.atan((lf + lr) / radius)

            # Obstacle 1
            if phi > math.pi/2 - alpha/2 - beta and phi < math.pi/2 - alpha/2:
                meanp = (phi - (math.pi/2 - alpha/2 - beta)) / beta * Oout
            elif phi >= math.pi/2 - alpha/2 and phi <= math.pi/2 + alpha/2:
                meanp = Oout
            elif phi > math.pi/2 + alpha/2 and phi < math.pi/2 + alpha/2 + beta:
                meanp = (math.pi/2 + alpha/2 + beta - phi) / beta * Oout

            # Obstacle 2
            elif phi > math.pi - alpha/2 - beta and phi < math.pi - alpha/2:
                meanp = - (phi - (math.pi - alpha/2 - beta)) / beta * Oin
            elif phi >= math.pi - alpha/2 and phi <= math.pi + alpha/2:
                meanp = - Oin
            elif phi > math.pi + alpha/2 and phi < math.pi + alpha/2 + beta:
                meanp = - (math.pi + alpha/2 + beta - phi) / beta * Oin

            # Obstacle 3
            elif phi > 3 * math.pi/2 - alpha/2 - beta and phi < 3 * math.pi/2 - alpha/2:
                meanp = (phi - (3 * math.pi/2 - alpha/2 - beta)) / beta * Oout
            elif phi >= 3 * math.pi/2 - alpha/2 and phi <= 3 * math.pi/2 + alpha/2:
                meanp = Oout
            elif phi > 3 * math.pi/2 + alpha/2 and phi < 3 * math.pi/2 + alpha/2 + beta:
                meanp = (3 * math.pi/2 + alpha/2 + beta - phi) / beta * Oout

            else:
                meanp = 0.0

            node = TrajectoryNode()
            node.header.frame_id = frame_id
            node.header.stamp = t0.to_msg()
            node.x = x
            node.y = y
            node.heading = phi
            node.v = vref
            node.delta = delta
            node.a = 0.0
            node.beta = 0.0
            node.mode = 1 # forward
            node.pw_l = meanp + pwidth / 2.0
            node.pw_r = - meanp + pwidth / 2.0
            traj.nodes.append(node)

        return traj


if __name__ == '__main__':
    rclpy.init()
    node = TrajectoryGeneratorNode()
    
    try:
        node.exec()
        rclpy.spin_once(node, timeout_sec=1.0)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()