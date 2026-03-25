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
from rclpy.time import Duration

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
        t1 = 0.0

        # Parameters
        Nn = 100  # number of trajectory nodes
        Nnps = Nn // 4  # number of nodes per segment
        vref = 3.0  # Reference velocity [m/s]
        pwidth = 2.0  # Width of the trajectory [m]
        amax = 2.5  # Maximum acceleration [m/s^2]
        amin = -2.5  # Minimum acceleration [m/s^2]

        # Vehicle parameters
        lf = 1.105
        lr = 1.738

        # Trajectory segments
        L1 = 10.0  # Length of first straight (forward, accelerate to vref) [m]
        R1 = 20.0  # Radius of first turn (forward, stay at vref, brake at end) [m]
        R2 = 10.0  # Radius of second turn (backward, accelerate to vref) [m]
        L2 = 5.0  # Length of second straight (backward, stay at vref, brake at end) [m]

        
        # If the trajectory segments are too short given the acceleration/deceleration constraints, adjust them
        xacc = abs(0.5 * vref**2 / amax)  # Distance to reach reference velocity
        xdec = abs(0.5 * vref**2 / amin)  # Distance to reach standstill
        if L1 < xacc:                     # Length of first straight vs acceleration distance
            L1 = xacc
        if 0.5 * math.pi * R1 < xdec:     # Distance on the arc vs braking distance
            R1 = xdec / (0.5 * math.pi)
        if 0.5 * math.pi * R2 < xacc:     # Distance on the arc vs acceleration distance
            R2 = xacc / (0.5 * math.pi)
        if L2 < xdec:                     # Length of second straight vs braking distance 
            L2 = xdec

        # Initial conditions
        x = 0.0
        y = 0.0
        phi = 0.0
        v = 0.0

        frame_id = 'base_link'

        # Initialize trajectory
        traj = Trajectory()
        traj.header.frame_id = frame_id
        traj.header.stamp = t0.to_msg()
        traj.x0 = x
        traj.y0 = y
        traj.heading0 = phi
        traj.type = 0 # 0: trajectory, 1: path, 2: circular path
        traj.nodes = []

        # Generate trajectory nodes

        # First straight
        ttot = vref / amax + (L1 - xacc) / vref # Total time for this segment: time to accelerate to vref + time at vref
        tmod = ttot / Nnps                      # Time per node
        for _ in range(Nnps):
            a = min(amax, (vref - v) / tmod)    # Acceleration to vref
            v = min(vref, v + tmod * amax)
            x += v * tmod
            
            node = TrajectoryNode()
            node.header.frame_id = frame_id
            node.header.stamp = (t0 + Duration(seconds=t1)).to_msg()
            node.x = x
            node.y = y
            node.heading = phi
            node.v = v
            node.a = a
            node.delta = 0.0
            node.beta = 0.0
            node.mode = 1 # forward
            node.pw_l = pwidth/2
            node.pw_r = pwidth/2
            traj.nodes.append(node)

            t1 += tmod

        # First turn
        ttot = (0.5 * math.pi * R1 - xacc) / vref + abs(vref / amin)  # Total time for this segment: time on the at vref + time to decelerate to standstill
        tmod = ttot / Nnps
        for _ in range(Nnps):
            if (0.5 * math.pi - abs(phi)) * R1 > xdec:   # While the distance to decelerate is larger than the remaining distance on the arc
                a = 0.0                                  # stay at vref
                v = vref
            else:
                a = amin
                v += tmod * amin
            x += v * tmod * math.cos(phi)
            y += v * tmod * math.sin(phi)
            phi -= v * tmod / R1                         # negative phi -> rotating clockwise
            
            node = TrajectoryNode()
            node.header.frame_id = frame_id
            node.header.stamp = (t0 + Duration(seconds=t1)).to_msg()
            node.x = x
            node.y = y
            node.heading = phi
            node.v = v
            node.a = a
            node.delta = -math.atan((lf + lr) / R1)      # negative delta + going forward -> turning right
            node.beta = 0.0
            node.mode = 1 # forward
            node.pw_l = pwidth/2
            node.pw_r = pwidth/2
            traj.nodes.append(node)
            
            t1 += tmod

        # Standstill
        v = 0.0
        a = amin
        phi = -math.pi/2
        tmod = 0.5 # stay still for 0.5 seconds

        node = TrajectoryNode()
        node.header.frame_id = frame_id
        node.header.stamp = (t0 + Duration(seconds=t1)).to_msg()
        node.x = x
        node.y = y
        node.heading = phi
        node.v = v
        node.a = a
        node.delta = 0.0
        node.beta = 0.0
        node.mode = 0 # standstill
        node.pw_l = pwidth/2
        node.pw_r = pwidth/2
        traj.nodes.append(node)
        
        t1 += tmod

        # Second turn
        ttot = vref / amax + (0.5 * math.pi * R2 - xacc) / vref  # Total time for this segment: time to accelerate to vref + time on the arc
        tmod = ttot / Nnps
        for _ in range(Nnps):
            a = min(amax, (vref - v) / tmod)    # Acceleration to vref
            v = min(vref, v + tmod * amax)
            x -= v * tmod * math.cos(phi)
            y -= v * tmod * math.sin(phi)
            phi -= v * tmod / R2
            
            node = TrajectoryNode()
            node.header.frame_id = frame_id
            node.header.stamp = (t0 + Duration(seconds=t1)).to_msg()
            node.x = x
            node.y = y
            node.heading = phi
            node.v = v
            node.a = a
            node.delta = math.atan((lf + lr) / R2)
            node.beta = 0.0
            node.mode = 2 # backward
            node.pw_l = pwidth/2
            node.pw_r = pwidth/2
            traj.nodes.append(node)

            t1 += tmod

        # Second straight
        ttot = (L2 - xdec) / vref + abs(vref / amin)  # Total time for this segment: # time at vref + time to decelerate to standstill
        tmod = ttot / Nnps
        phi = -math.pi
        for _ in range(Nnps):
            if L2 - y > xdec:
                a = 0.0
                v = vref
            else:
                a = amin
                v += tmod * amin
            x += v * tmod
            
            node = TrajectoryNode()
            node.header.frame_id = frame_id
            node.header.stamp = (t0 + Duration(seconds=t1)).to_msg()
            node.x = x
            node.y = y
            node.heading = phi
            node.v = v
            node.a = a
            node.delta = 0.0
            node.beta = 0.0
            node.mode = 2 # backward
            node.pw_l = pwidth/2
            node.pw_r = pwidth/2
            traj.nodes.append(node)

            t1 += tmod

        # Final standstill
        v = 0.0
        a = amin

        node = TrajectoryNode()
        node.header.frame_id = frame_id
        node.header.stamp = (t0 + Duration(seconds=t1)).to_msg()
        node.x = x
        node.y = y
        node.heading = phi
        node.v = 0.0
        node.a = a
        node.delta = 0.0
        node.beta = 0.0
        node.mode = 0 # standstill
        node.pw_l = pwidth/2
        node.pw_r = pwidth/2
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