# _disabled — packages kept in the tree but excluded from the build

`COLCON_IGNORE` here stops colcon from *discovering* anything under this folder.

This matters for packages that declare `<member_of_group>` for a ROOT group
(`rosidl_generator_packages`, `rmw_implementation_packages`, ...). Such a package
becomes an implicit dependency of the rosidl/rmw core, so if it fails to build,
**every message package and everything above it is skipped** ("N packages not
processed"), and if it depends back upward it creates an unorderable cycle.

`--packages-ignore` does NOT substitute: colcon orders packages *before* it
applies package selection.

Submodules here stay tracked and versioned. To re-enable one, `git mv` it back
out and run `ros2pkg check-cycles <distro>`.

| package | why |
|---------|-----|
| rosidlcpp | joins `rosidl_generator_packages`; `rosidlcpp_generator_core` fails to compile on macOS, which blocked 1745 packages (rclcpp, nav2, moveit, rviz2 never built) in CI run 82573881332 |
