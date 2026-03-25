# constants for common use
uint8 UNKNOWN = 0

# constants for color
uint8 RED = 1
uint8 AMBER = 2
uint8 GREEN = 3
uint8 WHITE = 4

# constants for shape
uint8 CIRCLE = 1
uint8 LEFT_ARROW = 2
uint8 RIGHT_ARROW = 3
uint8 UP_ARROW = 4
uint8 UP_LEFT_ARROW=5
uint8 UP_RIGHT_ARROW=6
uint8 DOWN_ARROW = 7
uint8 DOWN_LEFT_ARROW = 8
uint8 DOWN_RIGHT_ARROW = 9
uint8 CROSS = 10

# constants for status
uint8 SOLID_OFF = 1
uint8 SOLID_ON = 2
uint8 FLASHING = 3

# fields
# Traffic light color (use color constants above, or UNKNOWN)
uint8 color
# Traffic light shape (use shape constants above, or UNKNOWN)
uint8 shape
# Traffic light status (use status constants above, or UNKNOWN)
uint8 status
# Detection confidence. Range: 0.0 to 1.0
float32 confidence
