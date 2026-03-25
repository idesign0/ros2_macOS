std_msgs/Header header

uint16 obstacle_id
float64 obstacle_pos_x
float64 obstacle_pos_y

uint8 BLINKER_INFO_UNAVAILABLE = 0
uint8 BLINKER_INFO_OFF = 1
uint8 BLINKER_INFO_LEFT = 2
uint8 BLINKER_INFO_RIGHT = 3
uint8 BLINKER_INFO_BOTH = 4
uint8 blinker_info

uint8 CUT_IN_AND_OUT_UNDEFINED = 0
uint8 CUT_IN_AND_OUT_IN_HOST_LANE = 1
uint8 CUT_IN_AND_OUT_OUT_HOST_LANE = 2
uint8 CUT_IN_AND_OUT_CUT_IN = 3
uint8 CUT_IN_AND_OUT_CUT_OUT = 4
uint8 cut_in_and_out

float64 obstacle_rel_vel_x

uint8 OBSTACLE_TYPE_VEHICLE = 0
uint8 OBSTACLE_TYPE_TRUCK = 1
uint8 OBSTACLE_TYPE_BIKE = 2
uint8 OBSTACLE_TYPE_PED = 3
uint8 OBSTACLE_TYPE_BICYCLE = 4
uint8 obstacle_type

uint8 OBSTACLE_STATUS_UNDEFINED = 0
uint8 OBSTACLE_STATUS_STANDING = 1
uint8 OBSTACLE_STATUS_STOPPED = 2
uint8 OBSTACLE_STATUS_MOVING = 3
uint8 OBSTACLE_STATUS_ONCOMING = 4
uint8 OBSTACLE_STATUS_PARKED = 5
uint8 obstacle_status

bool obstacle_brake_lights

uint8 OBSTACLE_VALID_INVALID = 0
uint8 OBSTACLE_VALID_NEW = 1
uint8 OBSTACLE_VALID_OLDER = 2
uint8 obstacle_valid

float32 obstacle_length
float32 obstacle_width
uint16 obstacle_age

uint8 OBSTACLE_LANE_NOT_ASSIGNED = 0
uint8 OBSTACLE_LANE_EGO_LANE = 1
uint8 OBSTACLE_LANE_NEXT_LANE = 2
uint8 OBSTACLE_LANE_INVALID = 3
uint8 obstacle_lane

bool cipv_flag
float32 radar_pos_x
float32 radar_vel_x

uint8 RADAR_MATCH_CONFIDENCE_NO_MATCH = 0
uint8 RADAR_MATCH_CONFIDENCE_MULTI_MATCH = 1
uint8 RADAR_MATCH_CONFIDENCE_BOUNDED_LOW = 2
uint8 RADAR_MATCH_CONFIDENCE_BOUNDED_MED = 3
uint8 RADAR_MATCH_CONFIDENCE_BOUNDED_HIGH = 4
uint8 RADAR_MATCH_CONFIDENCE_HIGH = 5
uint8 radar_match_confidence

uint16 matched_radar_id
float32 obstacle_angle_rate
float64 obstacle_scale_change
float32 object_accel_x
bool obstacle_replaced
float32 obstacle_angle
