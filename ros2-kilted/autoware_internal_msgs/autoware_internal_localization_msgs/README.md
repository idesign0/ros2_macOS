# autoware_internal_localization_msgs

This package contains message, service, and action definitions used by the localization system in Autoware.

## Description

The package includes service definitions for:

- `PoseWithCovarianceStamped`: Service to estimate initial pose with covariance information
- `InitializeLocalization` : Service to initialize the localization system with different methods

## Services

### PoseWithCovarianceStamped

This service is used to estimate the initial pose of the vehicle.

**Request:**

- `geometry_msgs/PoseWithCovarianceStamped pose_with_covariance`: Input pose with covariance

**Response:**

- `bool success`: Whether the service execution was successful
- `bool reliable`: Whether the estimated pose is reliable
- `geometry_msgs/PoseWithCovarianceStamped pose_with_covariance`: Estimated pose with covariance

### InitializeLocalization

This service is used to initialize the localization system with different methods.

**Request:**

- `geometry_msgs/PoseWithCovarianceStamped[<=1] pose_with_covariance`: Optional input pose for initialization
- `uint8 method`: Initialization method
  - `AUTO (0)`: The initial position is automatically estimated with localization algorithm. The input pose will be used as an initial guess if provided.
  - `DIRECT (1)`: The initial position is set directly by the input pose without going through localization algorithm.

**Response:**

- `autoware_common_msgs/ResponseStatus status`: Response status containing error codes in case of failure:
  - `ERROR_UNSAFE (1)`: Unsafe initialization
  - `ERROR_GNSS_SUPPORT (2)`: GNSS support error
  - `ERROR_GNSS (3)`: GNSS error
  - `ERROR_ESTIMATION (4)`: Estimation error
