# _disabled — packages kept in the tree but excluded from the build

`COLCON_IGNORE` here stops colcon from *discovering* anything under this folder.
That matters for packages which declare `<member_of_group>` for a ROOT group
(`rosidl_generator_packages`, `rmw_implementation_packages`, ...): merely being
discovered makes them an implicit dependency of the rosidl/rmw core, which can
create a dependency cycle and make the whole workspace unorderable
("Unable to order packages topologically").

`--packages-ignore` does NOT help — colcon orders packages *before* applying
package selection, so the offender must not be discovered at all.

The submodules stay tracked and versioned; to re-enable one, `git mv` it back
out of this folder and re-run `ros2pkg check-cycles <distro>`.

| package | why |
|---------|-----|
| rosidlcpp | joins `rosidl_generator_packages`; creates a 22-package cycle with kilted's rmw core (verified: excluding it is the minimal fix) |
