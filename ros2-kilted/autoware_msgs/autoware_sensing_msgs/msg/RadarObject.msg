float32 INVALID_COV_VALUE = -1.0

uint8 MEASUREMENT_STATUS_INVALID = 0
uint8 MEASUREMENT_STATUS_MEASURED = 1
uint8 MEASUREMENT_STATUS_PREDICTED = 2
uint8 MEASUREMENT_STATUS_NEW = 3
uint8 MEASUREMENT_STATUS_UNKNOWN = 4

uint8 MOVEMENT_STATUS_INVALID = 0
uint8 MOVEMENT_STATUS_DYNAMIC = 1
uint8 MOVEMENT_STATUS_STATIC = 2
uint8 MOVEMENT_STATUS_UNKNOWN = 3

uint32 object_id
uint16 age
uint8 measurement_status
uint8 movement_status

geometry_msgs/Point position
geometry_msgs/Vector3 velocity
geometry_msgs/Vector3 acceleration
geometry_msgs/Vector3 size

float32[6] position_covariance
float32[6] velocity_covariance
float32[6] acceleration_covariance
float32[6] size_covariance

float32 orientation
float32 orientation_std
float32 orientation_rate
float32 orientation_rate_std

float32 existence_probability
RadarClassification[] classifications
