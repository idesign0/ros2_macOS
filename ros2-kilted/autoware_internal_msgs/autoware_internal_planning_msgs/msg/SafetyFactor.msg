# constants for common use
uint16 UNKNOWN = 0

# constants for factor type
uint16 POINTCLOUD = 1
uint16 OBJECT = 2

# variables
uint16 type

# use only for predicted objects
unique_identifier_msgs/UUID object_id

# use only for predicted objects
autoware_perception_msgs/PredictedPath predicted_path

# Time to collision (start) relative to the header time [s]
float32 ttc_begin
# Time to collision (end) relative to the header time [s]
float32 ttc_end

# type == POINTCLOUD: the position of the each points
# type == OBJECT: the object position
geometry_msgs/Point[] points

# module's primitive judgement of the safety for its decision
bool is_safe
