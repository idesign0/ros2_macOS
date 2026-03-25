# Message file for srr_track

std_msgs/Header header

uint8     can_tx_detect_valid_level
uint8     CAN_TX_DETECT_VALID_LEVEL_SUSPECT_DETECTION=0
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_1=1
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_2=2
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_3=3
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_4=4
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_5=5
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_6=6
uint8     CAN_TX_DETECT_VALID_LEVEL_LEVEL_7=7

bool      can_tx_detect_status
bool      CAN_TX_DETECT_STATUS_NO_DATA=0
bool      CAN_TX_DETECT_STATUS_VALID_DATA_PRESENT=1

float32   can_tx_detect_range_rate                 # m/s
float32   can_tx_detect_range                      # m
float32   can_tx_detect_angle                      # deg
float32   can_tx_detect_amplitude                  # dbsm
