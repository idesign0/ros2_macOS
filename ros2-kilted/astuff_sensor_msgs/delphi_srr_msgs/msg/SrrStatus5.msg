# Message file for srr_status5

std_msgs/Header header

bool      disable_auto_align

uint8     can_tx_yaw_rate_ref_qf
uint8     CAN_TX_YAW_RATE_REF_QF_UNDEFINED=0
uint8     CAN_TX_YAW_RATE_REF_QF_TEMP_UNDEFINED=1
uint8     CAN_TX_YAW_RATE_REF_QF_NOT_ACCURATE=2
uint8     CAN_TX_YAW_RATE_REF_QF_ACCURATE=3

uint8     can_tx_yaw_rate_raw_qf
uint8     CAN_TX_YAW_RATE_RAW_QF_UNDEFINED=0
uint8     CAN_TX_YAW_RATE_RAW_QF_TEMP_UNDEFINED=1
uint8     CAN_TX_YAW_RATE_RAW_QF_NOT_ACCURATE=2
uint8     CAN_TX_YAW_RATE_RAW_QF_ACCURATE=3

float32   can_tx_yaw_rate_reference                # deg/s
float32   can_tx_yaw_rate_raw                      # deg/s

uint8     can_tx_system_status
uint8     CAN_TX_SYSTEM_STATUS_CONFIGURATION=0
uint8     CAN_TX_SYSTEM_STATUS_STARTUP=1
uint8     CAN_TX_SYSTEM_STATUS_RUNNING=2
uint8     CAN_TX_SYSTEM_STATUS_BLOCKED=3
uint8     CAN_TX_SYSTEM_STATUS_FAULTY=4
uint8     CAN_TX_SYSTEM_STATUS_SHUTDOWN=5
uint8     CAN_TX_SYSTEM_STATUS_HOT=6

int16     can_tx_outside_temperature               # degc
bool      can_blockage_mnr_blocked
bool      can_blockage_bb_blocked
bool      can_blockage_radar_blocked
bool      can_td_blocked
bool      radar_tx_power_error
bool      radar_lo_power_error
bool      radar_data_sync_error
bool      linearizer_spi_transfer_error
bool      saturated_tuning_freq_error
bool      rtn_spi_transfer_error
bool      rrn_spi_transfer_error
bool      video_port_capture_error
bool      vertical_misalignment_error
bool      tx_temperature_fault
bool      transmitter_id_error
bool      dsp_unit_cal_checksum_error
bool      dsp_unit_cal_block_chcksm_error
bool      dsp_tuning_sensitivity_error
bool      dsp_loop_overrun_error
bool      adc_spi_transfer_error
