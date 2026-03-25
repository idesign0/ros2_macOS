# A reference trajectory for the ackermann_nlmpc_node in the form of a piecewise linear path with optional
# desired velocity and acceleration profile.
# The pose described in this message (x0, y0, heading0) is the origin of the trajectory. The nodes in the
# nodes[] list describe the path. Both origin and nodes may be provided in any coordinate frame, as long
# as a transformation to the odom frame is available via tf.

std_msgs/Header header  # Time and coordinate frame information for trajectory origin.
                        # Set header.stamp to current ROS time to start trajectory execution immediately.
                        # Set header.stamp > current ROS time for delayed start of trajectory execution.
float32 x0              # Trajectory origin x coordinate [m] in the frame set by header.frame_id.
float32 y0              # Trajectory origin y coordinate [m] in the frame set by header.frame_id.
float32 heading0        # Trajectory origin heading [rad] in the frame set by header.frame_id.
uint8 type              # Trajectory type:
                        # 0: "trajectory": Follow the provided trajectory, including time based
                        #                  longitudinal adjustment, i.e. if vehicle is lagging
                        #                  behind according to node timestamps, exceed reference
                        #                  velocity to try and catch up.
                        # 1: "path": Follow the provided trajectory, without time based longitudinal
                        #            adjustment.
                        # 2: "circular path": Same as path, but when reaching the end of the
                        #                     trajectory, start again from the beginning.
ackermann_nlmpc_msgs/TrajectoryNode[] nodes # trajectory nodes