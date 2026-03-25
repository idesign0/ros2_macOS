uint8 device_id
uint8 scanner_type
uint16 scan_number
float32 start_angle
float32 end_angle
builtin_interfaces/Time scan_start_time
builtin_interfaces/Time scan_end_time
builtin_interfaces/Time scan_start_time_from_device
builtin_interfaces/Time scan_end_time_from_device
float32 scan_frequency
float32 beam_tilt
uint32 scan_flags
ibeo_msgs/MountingPositionF mounting_position
ibeo_msgs/ResolutionInfo[8] resolutions

# scanner_type values
uint8 ALASCA_XT=3
uint8 LUX_ECU=4
uint8 LUX_PROTOTYPE=5
uint8 LUX=6
uint8 SCALA_B1=96
