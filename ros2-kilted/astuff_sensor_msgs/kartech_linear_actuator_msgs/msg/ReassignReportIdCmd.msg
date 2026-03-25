std_msgs/Header header
bool confirm
uint32 user_report_id     # The new user report ID to use. Values 0xFFFEXX and 0xFF00XX are reserved. Setting this to 0xFFFFFFFF will only change the use_user_report_id flag.
bool use_user_report_id   # Whether to use the user-defined report ID (true) or the default report ID (false).
