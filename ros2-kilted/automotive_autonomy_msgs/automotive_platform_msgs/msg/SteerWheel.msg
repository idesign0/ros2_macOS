# Steering Wheel Control Message with Mode
# Contains steering wheel command and desired autonomy active mode flag

std_msgs/Header header

uint16 mode             # Is 1 if autonomy mode should be active

float32 angle           # Desired steering wheel angle (rad)
float32 angle_velocity  # Desired steering wheel angle max velocity (rad/sec)
