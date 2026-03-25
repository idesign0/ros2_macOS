# Desired Destination Message
# Contains the location of a desired destination

std_msgs/Header header

uint8 msg_counter   # Increments each time a command is sent
                    # An acknowledge message should be published with this value

uint16 valid        # 1 if location should be used, 0 to cancel out previous location

float64 latitude    # Latitude of destination (rad)
float64 longitude   # Longitude of destination (rad)
