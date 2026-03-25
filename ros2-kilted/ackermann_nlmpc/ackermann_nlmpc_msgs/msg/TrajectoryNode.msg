# Desired vehicle state at one point on the trajectory.
# When making a direction change (mode 1->2 or 2->1) it is advisable to add at least one node
# with standstill mode (0) in between.
# Use the path width constraints for obstacle avoidance, e.g. for 1 m wide obstacle centered
# on the trajectory, set pw_l = -0.5 and pw_r = 1.5 to go around on the right side.

std_msgs/Header header  # Node header containing:
                        # - timestamp [s] at which the vehicle should reach this node,
                        # - coordinate frame id in which the coordinates are provided.
float64 x               # x coordinate of the node [m]
float64 y               # y coordinate of the node [m]
float64 heading         # yaw angle of the node [rad]
float64 v               # desired velocity at this node [m/s]
float64 a               # desired acceleration at this node [m/s^2]
float64 delta           # desired steering angle at this node [rad]
float64 beta            # desired side slip angle at this node [rad]
uint8 mode              # driving mode (0: standstill, 1: forward, 2: backward)
float64 pw_l            # path width left (state constraint) [m]
float64 pw_r            # path width right (state constraint) [m]