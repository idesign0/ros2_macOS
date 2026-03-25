uint16 object_age
uint16 hidden_status_age
uint8 object_phase
uint8 dynamic_property
uint16 relative_time_of_measure
ibeo_msgs/Point2Di position_closest_point
ibeo_msgs/Point2Di relative_velocity
ibeo_msgs/Point2Dui relative_velocity_sigma
uint8 classification
uint16 classification_age
ibeo_msgs/Point2Di object_box_size
ibeo_msgs/Point2Dui object_box_size_sigma
int16 object_box_orientation
uint16 object_box_orientation_sigma
uint8 tracking_point_location
ibeo_msgs/Point2Di tracking_point_coordinate
ibeo_msgs/Point2Dui tracking_point_coordinate_sigma
ibeo_msgs/Point2Di velocity
ibeo_msgs/Point2Dui velocity_sigma
ibeo_msgs/Point2Di acceleration
ibeo_msgs/Point2Dui acceleration_sigma
int16 yaw_rate
uint16 yaw_rate_sigma
uint8 number_of_contour_points
ibeo_msgs/ContourPointSigma[] contour_point_list

# object_phase values
uint8 INITIALIZATION_PHASE=0
uint8 TRACKING_PHASE=1

# dynamic_property values
uint8 DYNAMIC_AND_MOVING=0
uint8 DYNAMIC_AND_STOPPED=1
uint8 A_PRIORI_STATIONARY=2

# classification values
uint8 UNCLASSIFIED=0
uint8 UNKNOWN_SMALL=1
uint8 UNKNOWN_BIG=2
uint8 PEDESTRIAN=3
uint8 BIKE=4
uint8 CAR=5
uint8 TRUCK=6
uint8 OVER_DRIVABLE=10
uint8 UNDER_DRIVABLE=11

# tracking_point_location values
uint8 CENTER_OF_GRAVITY=0
uint8 TOP_FRONT_LEFT_CORNER=1
uint8 TOP_FRONT_RIGHT_CORNER=2
uint8 BOTTOM_REAR_RIGHT_CORNER=3
uint8 BOTTOM_REAR_LEFT_CORNER=4
uint8 CENTER_OF_TOP_FRONT_EDGE=5
uint8 CENTER_OF_RIGHT_EDGE=6
uint8 CENTER_OF_BOTTOM_REAR_EDGE=7
uint8 CENTER_OF_LEFT_EDGE=8
uint8 BOX_CENTER=9
uint8 INVALID=255
