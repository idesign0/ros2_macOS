# Status of vehicle cabin sensors
std_msgs/Header header

# The door positions below are from the point of view of someone sitting
# in the vehicle, facing the X positive direction. Not using driver/passenger
# since this is different in left- vs right-hand drive vehicles.
bool door_open_front_right       # Status of the door nearest the front of the vehicle in the Y+ direction
bool door_open_front_left        # Status of the door nearest the front of the vehicle in the Y- direction
bool door_open_rear_right        # Status of the door nearest the rear of the vehicle in the Y+ direction
bool door_open_rear_left         # Status of the door nearest the rear of the vehicle in the Y- direction
bool hood_open                   # Status of the front compartment cover
bool trunk_open                  # Status of the rear compartment cover

bool passenger_present           # Whether or not a passenger is detected in the front non-driving seat
bool passenger_airbag_enabled    # Whether or not the passenger-side airbag is enabled

bool seatbelt_engaged_driver     # Whether or not the seatbelt for the driver's seat is buckled
bool seatbelt_engaged_passenger  # Whether or not the seatbelt for the passenger's seat is buckled
