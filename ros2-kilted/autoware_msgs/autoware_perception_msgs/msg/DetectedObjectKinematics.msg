# Only position is available, orientation is empty. Note that the shape can be an oriented
# bounding box but the direction the object is facing is unknown, in which case
# orientation should be empty.
uint8 UNAVAILABLE=0

# The orientation is determined only up to a sign flip. For instance, assume that cars are
# longer than they are wide, and the perception pipeline can accurately estimate the
# dimensions of a car. It should set the orientation to coincide with the major axis, with
# the sign chosen arbitrarily, and use this tag to signify that the orientation could
# point to the front or the back.
uint8 SIGN_UNKNOWN=1

# The full orientation is available. Use e.g. for machine-learning models that can
# differentiate between the front and back of a vehicle.
uint8 AVAILABLE=2

geometry_msgs/PoseWithCovariance pose_with_covariance

bool has_position_covariance
uint8 orientation_availability

geometry_msgs/TwistWithCovariance twist_with_covariance

bool has_twist
bool has_twist_covariance
