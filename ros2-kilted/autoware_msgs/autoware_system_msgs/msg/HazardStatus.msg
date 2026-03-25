uint8 NO_FAULT = 0
uint8 SAFE_FAULT = 1
uint8 LATENT_FAULT = 2
uint8 SINGLE_POINT_FAULT = 3
uint8 level
bool emergency
bool emergency_holding
diagnostic_msgs/DiagnosticStatus[] diag_no_fault
diagnostic_msgs/DiagnosticStatus[] diag_safe_fault
diagnostic_msgs/DiagnosticStatus[] diag_latent_fault
diagnostic_msgs/DiagnosticStatus[] diag_single_point_fault
