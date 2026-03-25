std_msgs/Header header

bool suppress_sound
bool night_time
bool dusk_time

uint8 SOUND_SILENT = 0
uint8 SOUND_LDWL = 1
uint8 SOUND_LDWR = 2
uint8 SOUND_FAR_HW = 3
uint8 SOUND_NEAR_HW = 4
uint8 SOUND_SOFT_FCW = 5
uint8 SOUND_HARD_FCW = 6
uint8 SOUND_RESERVED = 7
uint8 sound_type

bool headway_valid
float32 headway_measurement
bool lanes_on
bool left_ldw_on
bool right_ldw_on
bool fcw_on
bool left_crossing
bool right_crossing
bool maintenance
bool failsafe
bool ped_fcw
bool ped_in_dz

uint8 HEADWAY_LEVEL_OFF = 0
uint8 HEADWAY_LEVEL_GREEN = 1
uint8 HEADWAY_LEVEL_ORANGE = 2
uint8 HEADWAY_LEVEL_RED = 3
uint8 headway_warning_level
