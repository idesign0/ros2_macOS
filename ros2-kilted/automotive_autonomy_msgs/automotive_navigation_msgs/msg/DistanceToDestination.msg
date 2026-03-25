# Distance To Destination Message
# Contains the distance to the desired destination

std_msgs/Header header

uint8 msg_counter   # Increments each time a command is sent
                    # An acknowledge message should be published with this value

float32 distance    # Distance to destination (m)
