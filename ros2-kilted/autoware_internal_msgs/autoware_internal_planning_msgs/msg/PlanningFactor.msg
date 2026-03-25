# constants for behavior
uint16 UNKNOWN = 0
uint16 NONE = 1
uint16 SLOW_DOWN = 2
uint16 STOP = 3
uint16 SHIFT_LEFT = 4
uint16 SHIFT_RIGHT = 5
uint16 TURN_LEFT = 6
uint16 TURN_RIGHT = 7

# Module name that generates this planning factor (e.g., "obstacle_stop_planner")
string module

bool is_driving_forward

autoware_internal_planning_msgs/ControlPoint[] control_points

# Behavior type (use constants above)
uint16 behavior

# Detailed description of the planning factor
string detail

autoware_internal_planning_msgs/SafetyFactorArray safety_factors
