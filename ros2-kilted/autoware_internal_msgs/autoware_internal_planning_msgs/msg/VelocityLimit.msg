# constants for system-defined sender
string SENDER_API = "api"

# fields
builtin_interfaces/Time stamp
# Maximum velocity [m/s]. Range: >= 0.0
float32 max_velocity

# Whether to use constraints
bool use_constraints false
# Velocity limit constraints (only used if use_constraints is true)
autoware_internal_planning_msgs/VelocityLimitConstraints constraints

# Identifier of the module that set this limit (use SENDER_* constants above or any user-defined sender)
string sender
