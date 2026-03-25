std_msgs/Header header

ibeo_msgs/IbeoDataHeader ibeo_header
uint16 image_format
uint32 us_since_power_on
builtin_interfaces/Time timestamp
uint8 device_id
ibeo_msgs/MountingPositionF mounting_position
float64 horizontal_opening_angle
float64 vertical_opening_angle
uint16 image_width
uint16 image_height
uint32 compressed_size
uint8[] image_buffer

# image_format values
uint16 JPEG=0
uint16 MJPEG=1
uint16 GRAY8=2
uint16 YUV420=3
uint16 YUV422=4
