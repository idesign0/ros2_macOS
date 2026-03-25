# Control messages calculated for a future horizon
#
# Control messages are ordered from near to far future [0 to N) with time_step_ms increments.
#
# First element of the array contains the control signals at the control_time of this message.
#
# The control_time field in each element of the controls array can be ignored.

# Time this message was created
builtin_interfaces/Time stamp

# Time when controls[0] configuration state is expected to be achieved in
builtin_interfaces/Time control_time

# Time difference between consecutive elements of the controls array in milliseconds
float32 time_step_ms

Control[] controls
