std_msgs/Header header

ibeo_msgs/IbeoDataHeader ibeo_header
uint16 scan_number
uint16 scanner_type
bool motor_on
bool laser_on
bool frequency_locked
uint8 motor_rotating_direction
uint16 angle_ticks_per_rotation
uint32 scan_flags
int16 mounting_yaw_angle_ticks
int16 mounting_pitch_angle_ticks
int16 mounting_roll_angle_ticks
int16 mounting_position_x
int16 mounting_position_y
int16 mounting_position_z
uint8 device_id
builtin_interfaces/Time scan_start_time
builtin_interfaces/Time scan_end_time
int16 start_angle_ticks
int16 end_angle_ticks
uint8 mirror_side
int16 mirror_tilt
ibeo_msgs/ScanPoint2208[] scan_point_list

# scanner_type values
uint16 SCALA_B2=11520

# motor_rotating_direction values
uint8 CLOCKWISE=0
uint8 COUNTER_CLOCKWISE=1

# mirror_side values
uint8 FRONT_MIRROR=0
uint8 REAR_MIRROR=1
