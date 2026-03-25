# autoware_control_msgs

## Design

Vehicle dimensions and axes: <https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-interfaces/components/vehicle-dimensions/>

### Lateral.msg

Defines a lateral control command message with a timestamp.

The message conveys the expectation for vehicle's lateral state to be in the given configuration in the time point: `control_time`.

- The field `steering_tire_angle` is required.
- The field `steering_tire_rotation_rate` is optional but may be required by some nodes.
  - If this field is used, `is_defined_steering_tire_rotation_rate` must be set `true`.

### Longitudinal.msg

Defines a longitudinal control command message with a timestamp.

The message conveys the expectation for vehicle's longitudinal state to be in the given configuration in the time point: `control_time`.

- The field `velocity` is required.
- The field `acceleration` is optional but may be required by some nodes.
  - If this field is used, `is_defined_acceleration` must be set `true`.
- The field `jerk` is optional but may be required by some nodes.
  - If this field is used, `is_defined_jerk` must be set `true`.

### Control.msg

Defines a control command message, combining `Lateral.msg` and `Longitudinal.msg`.

The message conveys the expectation for vehicle's combined control state to be in the given configuration in the time point: `control_time`.

If the `control_time` is defined, the `control_time` field in the `lateral` and `longitudinal` fields are ignored.

### ControlHorizon.msg

Defines a control commands array calculated for a future horizon.

- Control messages are ordered from near to far future `[0 to N)` with `time_step_ms` increments.
- First element of the array contains the control signals at the `control_time` of this message.
- The `control_time` field in each element of the controls array can be ignored.
