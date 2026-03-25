# Message file for srr_feature_alert

std_msgs/Header header

uint8     lcma_blis_ignored_track_id
uint8     lcma_blis_track_id
float32   lcma_cvw_ttc                             # seconds
bool      cta_ttc_alert
float32   cta_selected_track_ttc                   # seconds
uint16    cta_selected_track
uint8     cta_alert                                # binary
bool      cta_active                               # binary
uint8     lcma_cvw_cipv
uint8     lcma_cvw_alert_state
uint8     lcma_blis_alert_state
bool      lcma_active
