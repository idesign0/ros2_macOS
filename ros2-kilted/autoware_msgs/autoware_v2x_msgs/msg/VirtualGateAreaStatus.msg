# constants for status
uint16 RESERVED = 1
uint16 ACQUIRED = 2
uint16 RELEASED = 3

# variables for lock
builtin_interfaces/Time stamp
uint16   status
uint16   sequence_id  # Used to check correspondence with commands.
string   area_id      # Target area ID.
string[] gate_ids     # Entry and exit gate ID.

# variables for scheduling
builtin_interfaces/Time[<=1] expected_time_arrival
