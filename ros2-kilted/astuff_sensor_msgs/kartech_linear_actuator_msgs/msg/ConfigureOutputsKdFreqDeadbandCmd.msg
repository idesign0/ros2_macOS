std_msgs/Header header
bool confirm
uint16 kd                   # The differential term of the closed-loop control. Default is 10.
uint8 closed_loop_freq      # The frequency of closed-loop corrections in Hz. The default is 60Hz.
float64 error_dead_band     # The size of the dead-band for error correction in units of 0.001". The default is 0.05"
