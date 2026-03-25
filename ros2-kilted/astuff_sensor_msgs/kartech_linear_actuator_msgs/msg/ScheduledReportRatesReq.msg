std_msgs/Header header
bool confirm
kartech_linear_actuator_msgs/ReportIndex index_1
uint16 index_1_report_time                       # How often to publish the requested report in ms.
kartech_linear_actuator_msgs/ReportIndex index_2 # If this is set to REPORT_NONE_INDEX then only the first index will be reported.
uint16 index_2_report_time                       # Ignored if index_2 is set to REPORT_NONE_INDEX.
