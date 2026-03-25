# Point of Interest Response Message
# Contains status information about the points within the threshold

std_msgs/Header header

string name             # Name of the point of interest list

string module_name      # module name of the requesting node

uint16 request_id       # Unique id of this request

uint16 update_num       # The update number of the point list to use

automotive_navigation_msgs/PointOfInterestStatus[] point_statuses # The status information
