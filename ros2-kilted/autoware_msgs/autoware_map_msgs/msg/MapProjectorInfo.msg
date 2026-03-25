# Projector type
string LOCAL = "Local"
string LOCAL_CARTESIAN_UTM = "LocalCartesianUTM"
string LOCAL_CARTESIAN = "LocalCartesian"
string MGRS = "MGRS"
string TRANSVERSE_MERCATOR = "TransverseMercator"
string projector_type

# Vertical datum
string WGS84 = "WGS84"
string EGM2008 = "EGM2008"
string vertical_datum

# Used for MGRS map
string mgrs_grid

# Used for some map projection types
# Altitude is not used, so there is no need to specify it.
# Even if altitude is specified, it will be overwritten with 0.0.
geographic_msgs/GeoPoint map_origin

# Scale factor of the map
# Used for TransverseMercator map (default: 0.9996)
# In the case of LocalCartesianUTM and MGRS, it will be overwritten with 0.9996.
# In the case of Local and LocalCartesian, it will be overwritten with 1.0.
float32 scale_factor
