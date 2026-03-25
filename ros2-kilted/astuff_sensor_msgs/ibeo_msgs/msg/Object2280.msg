uint16 id
uint8 tracking_model
bool mobility_of_dyn_object_detected
bool motion_model_validated
uint32 object_age
builtin_interfaces/Time timestamp
uint16 object_prediction_age
uint8 classification
uint8 classification_certainty
uint32 classification_age
ibeo_msgs/Point2Df object_box_center
ibeo_msgs/Point2Df object_box_center_sigma
ibeo_msgs/Point2Df object_box_size
float32 object_box_orientation_angle
float32 object_box_orientation_angle_sigma
ibeo_msgs/Point2Df relative_velocity
ibeo_msgs/Point2Df relative_velocity_sigma
ibeo_msgs/Point2Df absolute_velocity
ibeo_msgs/Point2Df absolute_velocity_sigma
uint8 number_of_contour_points
uint8 closest_point_index
uint16 reference_point_location
ibeo_msgs/Point2Df reference_point_coordinate
ibeo_msgs/Point2Df reference_point_coordinate_sigma
float32 reference_point_position_correction_coefficient
uint16 object_priority
float32 object_existence_measurement
ibeo_msgs/Point2Df[] contour_point_list

# tracking_model values
uint8 DYNAMIC_MODEL=0
uint8 STATIC_MODEL=1

# classification values
uint8 UNCLASSIFIED=0
uint8 UNKNOWN_SMALL=1
uint8 UNKNOWN_BIG=2
uint8 PEDESTRIAN=3
uint8 BIKE=4
uint8 CAR=5
uint8 TRUCK=6
uint8 UNDER_DRIVABLE=12
uint8 OVER_DRIVABLE=13

# reference_point_location values
uint16 CENTER_OF_GRAVITY=0
uint16 FRONT_LEFT=1
uint16 FRONT_RIGHT=2
uint16 REAR_RIGHT=3
uint16 REAR_LEFT=4
uint16 FRONT_CENTER=5
uint16 RIGHT_CENTER=6
uint16 REAR_CENTER=7
uint16 LEFT_CENTER=8
uint16 OBJECT_CENTER=9
uint16 UNKNOWN=255

