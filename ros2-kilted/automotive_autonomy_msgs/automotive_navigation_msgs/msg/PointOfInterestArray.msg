# Point of Interest List Message
# Contains an array of points of interest
# update_num is incremented each time a new list is sent

std_msgs/Header header
uint16 update_num
automotive_navigation_msgs/PointOfInterest[] point_list
