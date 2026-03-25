# Common autoware classes
uint8 UNKNOWN = 0
uint8 CAR = 1
uint8 TRUCK = 2
uint8 BUS = 3
uint8 TRAILER = 4
uint8 MOTORCYCLE = 5
uint8 BICYCLE = 6
uint8 PEDESTRIAN = 7
uint8 ANIMAL = 8

# Radar specific classes
uint8 HAZARD = 9 # Defined as an object that can cause danger to autonomous driving
uint8 OVER_DRIVABLE = 10 # Defined as an object that can be safely driven over (e.g., leaf)
uint8 UNDER_DRIVABLE = 11 # Defined as an object that can be safely driven under (e.g., overpass)

uint8 label
float32 probability
