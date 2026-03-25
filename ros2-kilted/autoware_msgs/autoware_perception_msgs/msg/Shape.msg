# Shape type constants
uint8 BOUNDING_BOX=0  # Use dimensions (x=length, y=width, z=height)
uint8 CYLINDER=1      # Use dimensions (x=diameter, y=diameter, z=height)
uint8 POLYGON=2       # Use footprint as the base polygon with dimensions.z for height

uint8 type
# Footprint polygon (used when type=POLYGON)
geometry_msgs/Polygon footprint
# Dimensions [m] (x, y, z interpretation depends on type)
geometry_msgs/Vector3 dimensions
