# constants for command
uint16 ACQUIRE = 1
uint16 RELEASE = 2

# variables for lock
uint16   command
uint16   sequence_id  # Used to check correspondence with status messages.
string   area_id      # Target area ID.
string[] gate_ids     # Entry and exit gate ID.

# variables for scheduling
builtin_interfaces/Time[<=1] expected_time_arrival
