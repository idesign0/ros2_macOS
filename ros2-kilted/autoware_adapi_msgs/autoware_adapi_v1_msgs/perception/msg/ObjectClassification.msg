# Object classification labels
uint8 UNKNOWN = 0   # Unknown or unclassified object
uint8 CAR = 1       # Passenger car
uint8 TRUCK = 2     # Truck or large vehicle
uint8 BUS = 3       # Bus
uint8 TRAILER = 4   # Trailer
uint8 MOTORCYCLE = 5  # Motorcycle
uint8 BICYCLE = 6   # Bicycle
uint8 PEDESTRIAN = 7  # Pedestrian

# Classification label (one of the constants above)
uint8 label
# Classification probability. Range: 0.0 to 1.0
float64 probability
