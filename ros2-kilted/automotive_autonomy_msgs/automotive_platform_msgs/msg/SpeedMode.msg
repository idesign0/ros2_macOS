# Speed Control Message with Mode
# Contains speed commands and desired autonomy active mode flag

std_msgs/Header header

uint16 mode # Is 1 if autonomy mode should be active

float32 speed                # Desired speed (m/sec)
float32 acceleration_limit   # Max acceleration limit (m/sec^2)
float32 deceleration_limit   # Max deceleration limit (m/sec^2)
