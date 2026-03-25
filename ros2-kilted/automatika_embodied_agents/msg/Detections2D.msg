std_msgs/Header header

float64[] scores
string[] labels
Bbox2D[]  boxes

# Either an image or compressed image
sensor_msgs/Image image
sensor_msgs/CompressedImage compressed_image

# Depth image if available
sensor_msgs/Image depth
