# Point of Interest Request Message
# Contains information needed to request point of interest information

std_msgs/Header header

string name        # Name of the point of interest list

string module_name # module name of the requesting node

uint16 request_id  # Unique id of this request
                   # Can make another request with the same requestId and
                   # different update_num, guid, or tolerance.  New one will
                   # replace the old one.

uint16 cancel      # Set to 1 to cancel the request with this requestId

uint16 update_num  # The update number of the point list to use

uint16 guid_valid  # Request is for a specific point, not all points in list
uint64 guid        # The unique Id for the desired point

float32 tolerance  # How close to the current vehicle's position a point needs to be
