# Message file for srr_status3

std_msgs/Header header

uint8     can_tx_alignment_state
uint8     CAN_TX_ALIGNMENT_STATE_OFF=0
uint8     CAN_TX_ALIGNMENT_STATE_INIT=1
uint8     CAN_TX_ALIGNMENT_STATE_AUTOMATIC_ALIGNMENT=2
uint8     CAN_TX_ALIGNMENT_STATE_FACTORY_ALIGNMENT=3
uint8     CAN_TX_ALIGNMENT_STATE_SERVICE_ALIGNMENT=4

uint8     can_tx_interface_ver_minor
uint32    can_tx_sw_version_arm
uint8     can_tx_hw_version
uint8     can_tx_interface_version
uint32    can_tx_serial_num
