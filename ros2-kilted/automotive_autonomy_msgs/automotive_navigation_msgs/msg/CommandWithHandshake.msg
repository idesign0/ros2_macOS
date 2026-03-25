# General Command Message
# Contains a command and a message counter that can be used for handshaking

std_msgs/Header header

uint8 msg_counter   # Increments each time a command is sent
                    # An acknowledge message should be published with this value

int16 command       # The command value
