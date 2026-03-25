# Steering Control Message with Mode
# Contains steering commands and desired autonomy active mode flag

std_msgs/Header header

uint16 mode # Is 1 if autonomy mode should be active

float32 curvature           # Desired steering curvature (1/m)
float32 max_curvature_rate  # Max rate of change for curvature (1/m/sec)
