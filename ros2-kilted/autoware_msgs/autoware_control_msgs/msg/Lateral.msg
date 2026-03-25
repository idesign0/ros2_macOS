# Lateral control message for Ackermann-style platforms
#
# Note regarding tires: If the platform has multiple steering tires, the commands
# given here are for a virtual tire at the average lateral position of the steering tires.
#
# Look up Ackermann Bicycle Model for more details
#
# Positive values represents left inclination (if forward)
# Negative values represents right inclination (if forward)

# Time this message was created
builtin_interfaces/Time stamp

# Time this configuration state is expected to be achieved in (optional)
builtin_interfaces/Time control_time

# Desired angle of the steering tire in rad (radians)
float32 steering_tire_angle

# Desired rate of change of the steering tire angle in rad/s
float32 steering_tire_rotation_rate

# Controller has filled in the steering_tire_rotation_rate value
bool is_defined_steering_tire_rotation_rate
