# Point of Interest Status Message
# Contains the distance, heading, a localized position of a point of interest

uint64 guid        # Unique Id for this point

float32 distance   # Great circle distance (meters)
float32 heading    # Heading (radians)
float32 x_position # Distance in front of the vehicle (meters)
float32 y_position # Distance to the left of the vehicle (meters)

string params      # List of parameter:value pairs
