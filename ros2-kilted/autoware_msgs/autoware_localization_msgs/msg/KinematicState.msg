# KinematicState represents an estimate of a position, velocity and acceleration in free space.
# The pose_with_covariance is in the coordinate frame given by header.frame_id
# The twist_with_covariance and accel_with_covariance are in the coordinate frame given by the child_frame_id

# Includes the frame id of the pose parent.
std_msgs/Header header

# Frame id the pose points to.
string child_frame_id

# Estimated pose that is relative to the header.frame_id.
geometry_msgs/PoseWithCovariance pose_with_covariance

# Estimated linear and angular velocity relative to child_frame_id.
geometry_msgs/TwistWithCovariance twist_with_covariance

# Estimated linear and angular acceleration relative to child_frame_id.
geometry_msgs/AccelWithCovariance accel_with_covariance
