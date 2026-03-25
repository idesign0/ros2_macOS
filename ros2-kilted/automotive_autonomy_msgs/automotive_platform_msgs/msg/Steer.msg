# Steering Control Message
# Contains steering commands

std_msgs/Header header

string module_name # module name of the source node of this message

float32 curvature           # Desired steering curvature (1/m)
float32 max_curvature_rate  # Max rate of change for curvature (1/m/sec)
