# Lanelet map message
# This message contains the binary data of a Lanelet map.
# Also contains the map name, version and format.

# Header with timestamp when the message is published
# And frame of the Lanelet Map origin (probably just "map")
std_msgs/Header header

# Version of the map format (optional)
# Example: "1.1.1"
string version_map_format

# Version of the map (encouraged, optional)
# Example: "1.0.0"
string version_map

# Name of the map (encouraged, optional)
# Example: "florence-prato-city-center"
string name_map

# Binary map data
uint8[] data
