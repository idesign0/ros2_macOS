# Geographic point, using the WGS 84 reference ellipsoid.
# This data will be invalid If Autoware does not provide projection information between geographic coordinates and local coordinates.
geographic_msgs/GeoPointStamped geographic_pose

# Local coordinate from the autoware
geometry_msgs/PoseWithCovarianceStamped pose
geometry_msgs/TwistWithCovarianceStamped twist
geometry_msgs/AccelWithCovarianceStamped accel
