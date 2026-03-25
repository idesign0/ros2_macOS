# Turn Signal Command with Mode
# Contains turn signal command and desired autonomy active mode flag

std_msgs/Header header

uint16 mode             # Is 1 if autonomy mode should be active

uint8 NONE=0
uint8 LEFT=1
uint8 RIGHT=2
uint8 turn_signal       # Desired turn signal on
