std_msgs/Header header
bool confirm

uint8 RESET_OUTPUTS = 0
uint8 RESET_USER_DEFINED_IDS = 1
uint8 RESET_REPORT_RATES = 2
uint8 RESET_HARDWARE_CONFIGURATIONS = 3
uint8 RESET_USER_CONFIGURATIONS = 4
uint8 RESET_EVERYTHING = 5
uint8 RESET_NONE = 6

uint8 reset_type

bool reset_user_rpt_id
bool reset_user_cmd_id_1
bool reset_user_cmd_id_2
bool reset_user_cmd_id_3
bool reset_user_cmd_id_4
bool disable_user_rpt_id
bool reenable_default_cmd_id
