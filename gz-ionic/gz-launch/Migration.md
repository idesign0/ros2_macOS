# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Launch 6.X to 7.X

- Removed Ignition
    - Removed the ignition include directory.
    - Launch scripts must use the `<gz>` element.

## Gazebo Launch 5.X to 6.X

- The `ignition` namespace is deprecated and will be removed in future versions.
  Use `gz` instead.

- Header files under `ignition/...` are deprecated and will be removed in future versions.
  Use `gz/...` instead.

- Migrate `IGN_LAUNCH_PLUGIN_PATH` environment variable to `GZ_LAUNCH_PLUGIN_PATH` for finding
  plugin.
  With tick-tock.

- Migrate `IGN_LAUNCH_CONFIG_PATH` environment variable to `GZ_LAUNCH_CONFIG_PATH` for finding
  configs.
  With tick-tock.

- The default config file path has been hard-tocked to `~/.gz/launch/gui.config`.
  `~/.ignition/launch/gui.config` will no longer work.

- The shared libraries have `gz` where there used to be `ignition`.
  - Using the un-migrated version is still possible due to tick-tocks, but will be removed in future versions.
- Launch files have been hard-tocked to the `.gzlaunch` extension instead of `.ign`.

## Gazebo Launch 2.2.2

- Environment variable `GZ_LAUNCH_CONFIG_PATH` started to be treated as a path
  list (colon-separated on Linux, semicolon-separated on Windows). Before, only
  a single path could be set here, and setting a path list would break the whole
  launch file lookup functionality.

## Gazebo Launch 0.X to N.M
