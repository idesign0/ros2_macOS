std_msgs/Header header

uint8 LANE_CONFIDENCE_NONE = 0
uint8 LANE_CONFIDENCE_LOW = 1
uint8 LANE_CONFIDENCE_MED = 2
uint8 LANE_CONFIDENCE_HIGH = 3

uint8 LANE_TYPE_DASHED = 0
uint8 LANE_TYPE_SOLID = 1
uint8 LANE_TYPE_NONE = 2
uint8 LANE_TYPE_ROAD_EDGE = 3
uint8 LANE_TYPE_DOUBLE_LANE_MARK = 4
uint8 LANE_TYPE_BOTTS_DOTS = 5
uint8 LANE_TYPE_INVALID = 6

uint8 lane_confidence_left
bool ldw_available_left
uint8 lane_type_left
float32 distance_to_left_lane
uint8 lane_confidence_right
bool ldw_available_right
uint8 lane_type_right
float32 distance_to_right_lane
