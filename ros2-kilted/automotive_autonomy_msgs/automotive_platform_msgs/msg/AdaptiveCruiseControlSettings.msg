# Adaptive Cruise Control Settings Message
# Contains the current settings/status of ACC

std_msgs/Header header

float32 set_speed       # Current speed set point (m/sec)

uint16 following_spot   # Discrete following location
                        # 0 = following distance is the minimum allowed distance
                        # n = following distance is the maximum allowed distance
                        #      where (n + 1) is the number of allowed distances

float32 min_percent     # Minimum distance relative to maximum allowed distance
                        # 0 to 1.0

float32 step_percent    # Distance between spots relative to maximum allowed
                        # distance
                        # 0 to 1.0

float32 cipv_percent    # Where CIPV is relative to the maximum allowed distance
                        # 0 = there isn't a detected CIPV
                        # 1.0 = the CIPV is at the maximum allowed distance
                        # The maximum allowed distance varies with speed

float32 max_distance    # The distance corresponding to 1.0 percent (meters)

# So if the driver wants to maintain the greatest separation behind the CIPV,
# he can press the increase distance button over and over until the distance
# set point reaches the maximum allowed distance.  If there are 5 allowed
# following distance set points, then following_spot will equal 4.

# If a Closest In Path Vehicle is detected at this distance then the
# cipv_percent will be 1.0.
