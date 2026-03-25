std_msgs/Header header

Point2D[] centroids
string[] labels
Bbox2D[]  boxes
int8[] ids
Point2D[] estimated_velocities

# Either an image or compressed image
sensor_msgs/Image image
sensor_msgs/CompressedImage compressed_image

# Depth image if available
sensor_msgs/Image depth
