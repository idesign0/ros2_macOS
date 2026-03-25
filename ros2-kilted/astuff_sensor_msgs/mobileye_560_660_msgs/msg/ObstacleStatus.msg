std_msgs/Header header

uint16 num_obstacles
uint16 timestamp
uint16 application_version
uint16 active_version_number_section
bool left_close_range_cut_in
bool right_close_range_cut_in

uint8 STOP_GO_STOP = 0
uint8 STOP_GO_GO = 1
uint8 STOP_GO_UNDECIDED = 2
uint8 STOP_GO_DRIVER_DECISION_REQUIRED = 3
uint8 STOP_GO_NOT_CALCULATED = 15
uint8 stop_go

uint8 protocol_version
bool close_car

uint8 FAILSAFE_NONE = 0
uint8 FAILSAFE_LOW_SUN = 1
uint8 FAILSAFE_BLUR_IMAGE = 2
uint8 failsafe
