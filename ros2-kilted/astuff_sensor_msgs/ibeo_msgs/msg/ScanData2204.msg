std_msgs/Header header

ibeo_msgs/IbeoDataHeader ibeo_header
uint64 scan_start_time
uint32 scan_end_time_offset
bool ground_labeled
bool dirt_labeled
bool rain_labeled
bool fused_scan
uint8 mirror_side
uint8 coordinate_system
uint16 scan_number
uint16 scan_points
uint16 number_of_scanner_infos
ibeo_msgs/ScannerInfo2204[] scanner_info_list
ibeo_msgs/ScanPoint2204[] scan_point_list

# mirror_side values
uint8 FRONT=0
uint8 REAR=1

# coordinate_system values
uint8 SCANNER=0
uint8 VEHICLE=1
