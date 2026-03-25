builtin_interfaces/Time stamp

# For common use
uint16 UNKNOWN = 0

# For state
uint16 NORMAL = 1
uint16 MRM_OPERATING = 2
uint16 MRM_SUCCEEDED = 3
uint16 MRM_FAILED = 4

# For behavior. Deprecated: use description API.
# https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-interfaces/ad-api/features/fail-safe/
uint16 NONE = 1
uint16 EMERGENCY_STOP = 2
uint16 COMFORTABLE_STOP = 3
uint16 PULL_OVER = 4

uint16 state
uint16 behavior
