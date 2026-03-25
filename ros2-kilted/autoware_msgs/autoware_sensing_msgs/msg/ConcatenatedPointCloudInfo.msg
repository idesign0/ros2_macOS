# This message holds information about a concatenated point cloud that combines multiple source
# point clouds, including the overall point cloud structure and metadata for each source segment.

# Available matching strategies for source point clouds concatenation
uint8 STRATEGY_NAIVE = 0     # Assigns the oldest stamp of the sensor that was concatenated
uint8 STRATEGY_ADVANCED = 1  # Assigns the oldest stamp of the sensor that was concatenated

# The concatenated point cloud header, where timestamp assignment depends on matching strategy
std_msgs/Header header

bool concatenation_success                       # Whether concatenation was successful
uint8 matching_strategy                          # Strategy used for concatenation

# Raw data of concatenation config, relevant to matching_strategy field.
uint8[] matching_strategy_config

SourcePointCloudInfo[] source_info               # Meta information of input point clouds
