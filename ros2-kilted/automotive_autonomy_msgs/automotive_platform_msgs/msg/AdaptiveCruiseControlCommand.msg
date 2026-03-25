# Adaptive Cruise Control Command Message
# Contains commands to engage/disengage ACC or adjust the set points

std_msgs/Header header

uint8 msg_counter   # Increments each time a command is sent
                    # An acknowledge message should be published with this value

float32 set_speed   # Speed setpoint (m/sec)
uint16 set          # Engage ACC at the above speed set point
uint16 resume       # Resume ACC at previous speed set point
uint16 cancel       # Disengage ACC
uint16 speed_up     # Increase speed set point
uint16 slow_down    # Decrease speed set point
uint16 further      # Increase distance set point
uint16 closer       # Decrease distance set point
