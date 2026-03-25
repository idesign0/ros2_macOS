# This message holds metadata information about a source point cloud segment
# within a concatenated point cloud, including its spatial boundaries and structural properties.

# Available status codes for source point cloud
uint8 STATUS_OK = 0       # Cloud received (may have 0 or more points)
uint8 STATUS_TIMEOUT = 1  # Cloud not received due to timeout
uint8 STATUS_INVALID = 2  # Cloud arrived but was malformed/corrupt

# Time of sensor data acquisition, and the coordinate frame ID (for 3d points).
std_msgs/Header header

string topic              # Source topic
uint8 status              # Status of the source point cloud

# The starting index (inclusive) of the point cloud segment within the concatenated point cloud
uint32 idx_begin

# The length of the point cloud segment in the concatenated point cloud
uint32 length
