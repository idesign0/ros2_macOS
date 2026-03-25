# GNSS(Global Navigation Satellite System) INS(Inertial Navigation System) orientation message
#
# Represents a 3D orientation in a global coordinate system.
#
# ROS uses ENU axes convention (East, North, Up) corresponds to (x, y, z).

# Quaternion representation of the orientation.
geometry_msgs/Quaternion orientation

# Root mean square error of the rotation [rad]
# (x, y, z) axes corresponding to (roll, pitch, yaw)
float32 rmse_rotation_x
float32 rmse_rotation_y
float32 rmse_rotation_z
