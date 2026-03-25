# constants for common use
uint16 UNKNOWN = 0

# constants for status
uint16 APPROACHING = 1  # Vehicle is approaching the factor
uint16 STOPPED = 2      # Vehicle has stopped due to the factor

# fields
# Pose of the velocity factor in VelocityFactorArray.header.frame_id
geometry_msgs/Pose pose
# Distance to the velocity factor [m]
float32 distance
# Current status (use status constants above)
uint16 status
# Behavior type (see PlanningBehavior.msg for typical values, any name is available for user extensions)
string behavior
# Planning sequence identifier
string sequence
# Detailed description of the factor
string detail
# Cooperation status (optional, max 1 element)
autoware_adapi_v1_msgs/CooperationStatus[<=1] cooperation
