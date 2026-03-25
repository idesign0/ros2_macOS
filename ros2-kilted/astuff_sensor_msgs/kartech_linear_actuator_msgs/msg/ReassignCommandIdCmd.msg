std_msgs/Header header
bool confirm
uint8 command_id_index            # The index of the user-defined command ID (1-4) to change.
uint32 user_command_id            # The new user-defined command ID to set. 0xFFFEXX and 0xFF00XX are reserved.
                                  # Setting this to 0xFFFFFFFF will change the disable_default_command_id flag without affecting any others.
bool disable_default_command_id   # Disables (true) or enables (false) the default command ID.
