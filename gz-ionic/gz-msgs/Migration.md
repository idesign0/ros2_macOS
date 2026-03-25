# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Msgs 10.X to 11.X
### Deprecations

1. **spherical_coordinates.proto**
    + The `LOCAL2` field in `SphericalCoordinatesType` is deprecated, use `LOCAL` instead. See https://github.com/gazebosim/gz-math/pull/616 for more details.
      When converting and passing the value to `gz::math::SphericalCoordinates` class for processing, beware that in gz-math 9, some methods yield different
      results when they are passed a `gz::math::Vector3d` in `LOCAL` frame (wrong, but backwards-compatible ones) and different ones when passed a
      `gz::math::CoordinateVector3` (the correct ones). Passing `Vector3d` to the `SphericalCoordinates` methods will be removed in gz-math 10.
      You need to carefully consider this during migration. This table should help with the migration decisions:

      | Old frame | Old vector type | Migrated frame | Migrated vector type | Behavior |
      |-----------|-----------------|----------------|----------------------|----------|
      | `LOCAL2`  | `Vector3d`       | `LOCAL` | `CoordinateVector3` | no behavior change, correct computation |
      | `LOCAL2`  | `Vector3d`       | `LOCAL` | `Vector3d` |  **behavior change**, wrong computation (before was correct computation), deprecated call (removed in gz-math 10)  |
      | `LOCAL2`  | `Vector3d`       | `LOCAL2` | `CoordinateVector3` | no behavior change, correct computation, deprecated constant (removed in gz-math 10) |
      | `LOCAL2`  | `Vector3d`       | `LOCAL2` | `Vector3d` | no behavior change, correct computation, deprecated constant and call (removed in gz-math 10) |
      | `LOCAL`  | `Vector3d`       | `LOCAL` | `CoordinateVector3` | **behavior change**, correct computation (before was wrong computation) |
      | `LOCAL`  | `Vector3d`       | `LOCAL` | `Vector3d` | no behavior change, wrong computation, deprecated call (removed in gz-math 10) |

      When the table says behavior change, it means that the wrong computation incorrectly worked in West-South-Up frame, even though it told it works in East-North-Up frame.
      The old code using the wrong computation had to either (i) flip the sign of latitude and longitude, or (ii) rotate the results by 180 degrees in heading. When you
      migrate away from the wrong computations, you should drop these corrections.

## Gazebo Msgs 9.X to 10.X

### Modifications 

1. Th message generation pipeline is now accessible in downstream packages .
  * For more information, consult the `using_gz_msgs` example.
  * Note that there will no longer be Ruby generated messages, this support will be restored as-needed. 

## Gazebo Msgs 8.X to 9.X

1. **SuppressWarnings.hh** is deprecated and isn't part of `msgs.hh` anymore.
   Use gz-utils instead.
2. Header files under `ignition/...` are deprecated and will be removed in future versions.
   Use `gz/...` instead.
3. Protobuf messages and packages will no longer use `ignition.msgs`, use `gz.msgs` instead
4. `INSTALL_IGN_MSGS_GEN_EXECUTABLE` and `IGN_MSGS_GEN_EXECUTABLE` are deprecated and will be removed. Use `INSTALL_GZ_MSGS_GEN_EXECUTABLE` and `GZ_MSGS_GEN_EXECUTABLE` instead.
5. `IGN_DESCRIPTOR_PATH` is deprecated and will be removed. Use `GZ_DESCRIPTOR_PATH` instead.
6. `camerasensor.proto` has deprecated the `string image_format = 4`. Please
   use `PixelFormatType pixel_format = 21;`
### Breaking Changes

1. The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!
  * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
  * Some non-exhaustive examples of this include:
    * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
    * CMake `-config` files
    * Paths that depend on the project name

## Gazebo Msgs 8.1 to 8.2

### Modifications

1. **gps.proto**
    + Recommend the use of **navsat.proto** instead. The GPS message will be deprecated in the future.

1. **gps_sensor.proto**
    + Recommend the use of **navsat_sensor.proto** instead. The GPS message will be deprecated in the future.

## Gazebo Msgs 6.X to 7.X

### Removals

1. `bool use_parent_model_frame` from `axis.proto`.
    * [Pull Request 112](https://github.com/gazebosim/gz-msgs/pull/112/files)

1. `double force` from `joint_cmd.proto`.
    * [Pull Request 112](https://github.com/gazebosim/gz-msgs/pull/112/files)

1. `double target`, `double p_gain`, `double i_gain`, `double d_gain`,
   `double i_max`, `double i_min`, `double limit` from `pid.proto`.
    * [Pull Request 112](https://github.com/gazebosim/gz-msgs/pull/112/files)

## Gazebo Msgs 6.3 to 6.4

### Modifications

1. **particle_emitter.proto**
   + Fields have changed from plain data types to messages.
       * See [PR 137](https://github.com/gazebosim/gz-msgs/pull/137)

## Gazebo Msgs 4.X to 5.X

### Modifications


### Deprecations

1. **axis.proto**
    + The `use_parent_model_frame` field is deprecated, use `xyz_expressed_in` instead. Setting `xyz_expressed_in = "__model__"` is equivalent to`use_parent_model_frame = true` and leaving `xyz_expressed_in` empty is equivalent to `use_parent_model_frame = false`

## Gazebo Msgs 3.X to 4.X

### Modifications

1. **image.proto**
    + Corrected `BAYER_RGGR8` to `BAYER_BGGR8` in `PixelFormatType`.

1. **joint.proto**
    + The `axis.proto` message contains joint position, velocity, force, and
    acceleration information.

1. **serialized.proto**
    + `SerializedComponent`'s `component` field type changed from `string` to `bytes`.

1. **sensor.proto**
    + Deprecating `RaySensor`, and replacing with `LidarSensor`.

### Deprecations

1. **joint.proto**
    + The `angle` field is deprecated, use `position` in the axis.proto
    message instead.
    + The `velocity` field is deprecated, use `velocity` in the axis.proto
    message instead.

1. **image.proto**
    + `pixel_format` field is deprecated, use `pixel_format_type` with enum `PixelFormatType` instead.

### Removals

1.  **include/gz/msgs/gz.hh** is no longer installed.

## Gazebo Msgs 2.X to 3.X

### Additions

1. **axis\_aligned\_box.proto**
    + New message for axis-aligned bounding boxes.

1. **entity.proto**
    + New message to uniquely identify an entity.

1. **entity\_factory.proto**
    + Add `Model` field to allow model insertion.

1. **light.proto**
    + Add `id`, `parent_id` fields.

1. **link.proto**
    + Add repeated `light` field.

1. **twist.proto**
    + New message for storing linear and angular velocity.

1. **Utility.hh**
    + Conversion functions for `axis_aligned_box.proto`.

### Modifications

1. Use ignition-cmake2 and ignition-math6.

1. Use protobuf3 for all messages.

1. **contacts.proto**
    + Use `Entity` instead of string to specify entities in contact.

1. **entity\_factory.proto**
    + Rename `clone_model_name` field to `clone_name`.
    + Remove `edit_name` field.
    + Use `oneof` to indicate only one method can be used at a time.

### Deprecations

### Removals


## Gazebo Msgs 2.0 to 2.1

### Modifications

1. Use `gz-math6` instead of `gz-math5`.
    This implies an indirect dependency on `gz-cmake2` and cmake 3.10.2,
    which are not available on Ubuntu Xenial, so migration to Ubuntu Bionic may
    be required.

## Gazebo Msgs 1.X to 2.0

### Additions

1. **log\_control.proto**
    + Add `record_resources` bool to match osrf/gazebo PR 3008.

1. **log\_status.proto**
    + Add `record_resources` bool to match osrf/gazebo PR 3008.

1. **sonar.proto**
    + Add `geometry` field.

1. **world\_statistics.proto**
    + Add `real_time_factor` field.

### Modifications

1. Use ignition-cmake1 and ignition-math5.

1. Use inline versioned namespace.

### Deprecations

### Removals

1. **Generator.hh**
    + This file is no longer installed. It served only to make an internal
      protc plugin that customized the protobuf compiler output.
