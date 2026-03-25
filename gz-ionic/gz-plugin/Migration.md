# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Plugin 1.X to 2.X

* The `ignition` namespace is deprecated and will be removed in future versions.
  Use `gz` instead.

* Header files under `ignition/...` are deprecated and will be removed in future versions.
  Use `gz/...` instead.

* The following `IGN_` / `IGNITION_` prefixed macros are deprecated and will be removed in future versions.
  Additionally, they will only be available when including the corresponding `ignition/...` header.
  Use the `GZ_` prefix instead.
  * `IGN_PLUGIN_REGISTER_MORE_TRANS_UNITS`

  * `IGNITION_ADD_PLUGIN`
  * `IGNITION_ADD_PLUGIN_ALIAS`
  * `IGNITION_ADD_FACTORY`
  * `IGNITION_ADD_FACTORY_ALIAS`
  *  `IGN_CREATE_INTERFACE`
  *  `IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS`

* `ignitionVersion()` is deprecated and will be removed in future versions.
  Use `gzVersion()` instead.

* The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!
  * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
  * Some non-exhaustive examples of this include:
    * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
    * CMake `-config` files
    * Paths that depend on the project name
