# GNSS(Global Navigation Satellite System) INS(Inertial Navigation System) orientation message
#
# Represents a 3D orientation in a global coordinate system with coordinate frame and timestamp.

# Timestamp this reading was received by the INS sensor
# Frame is expected to be of the INS device
std_msgs/Header header

GnssInsOrientation orientation
