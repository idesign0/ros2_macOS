# Information source values (string constants):
# - V2N: Signal data sent from a server (Internet)
# - V2I: Direct from RSU (Roadside Unit)
# - V2V: Signal data from another vehicle
# - LOCAL_PERCEPTION: Information detected by onboard sensors
# - MANUAL_OVERRIDE: Manually set signal data (for testing/debugging)
# - SIMULATION: Signal data from a simulated environment
# - INTERNAL_ESTIMATION: Information that was predicted by an onboard model

string INFORMATION_SOURCE_V2N="V2N"
string INFORMATION_SOURCE_V2I="V2I"
string INFORMATION_SOURCE_V2V="V2V"
string INFORMATION_SOURCE_LOCAL_PERCEPTION="LOCAL_PERCEPTION"
string INFORMATION_SOURCE_MANUAL_OVERRIDE="MANUAL_OVERRIDE"
string INFORMATION_SOURCE_SIMULATION="SIMULATION"
string INFORMATION_SOURCE_INTERNAL_ESTIMATION="INTERNAL_ESTIMATION"

# Absolute time this state is expected
builtin_interfaces/Time predicted_stamp

# Valid signals at this time
autoware_perception_msgs/TrafficLightElement[] simultaneous_elements

# Confidence level [0.0–1.0]
float32 reliability

# Source of information, using predefined string constants
string information_source
