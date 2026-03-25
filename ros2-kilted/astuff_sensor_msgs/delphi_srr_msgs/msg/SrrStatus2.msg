# Message file for srr_status2

std_msgs/Header header

uint8     can_tx_alignment_status
uint8     CAN_TX_ALIGNMENT_STATUS_UNKNOWN=0
uint8     CAN_TX_ALIGNMENT_STATUS_CONVERGED=1
uint8     CAN_TX_ALIGNMENT_STATUS_FAILED=2
uint8     CAN_TX_ALIGNMENT_STATUS_RESERVED=3

bool      can_tx_comm_error
bool      can_tx_steering_angle_sign
float32   can_tx_yaw_rate_bias
float32   can_tx_veh_spd_comp_factor
uint16    can_tx_sw_version_dsp
int16     can_tx_temperature                       # degc
bool      can_tx_range_perf_error
bool      can_tx_overheat_error
bool      can_tx_internal_error
bool      can_tx_xcvr_operational
uint16    can_tx_steering_angle                    # deg
uint8     can_tx_rolling_count_2

