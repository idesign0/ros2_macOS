# Speed Control Message
# Contains speed commands

std_msgs/Header header

string module_name # module name of the source node of this message

float32 speed                # Desired speed (m/sec)
float32 acceleration_limit   # Max acceleration limit (m/sec^2)
float32 deceleration_limit   # Max deceleration limit (m/sec^2)
