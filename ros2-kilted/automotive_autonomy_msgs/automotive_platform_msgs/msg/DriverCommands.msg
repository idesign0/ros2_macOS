# Driver Commands Message
# Contains commands from the driver to the autonomy system

uint8 msg_counter   # Increments each time a command is sent
                    # An acknowledge message should be published with this value

uint16 engage       # Engage auto
uint16 disengage    # Disengage auto
uint16 speed_up     # Increase speed set point
uint16 slow_down    # Decrease speed set point
uint16 further      # Increase distance set point
uint16 closer       # Decrease distance set point
uint16 right_turn   # Right turn signal
uint16 left_turn    # Left turn signal
