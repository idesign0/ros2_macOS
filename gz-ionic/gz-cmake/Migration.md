# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo CMake 3.X to 4.X

1. The minimum required cmake version is now 3.22.1.

1. **Breaking**: C/C++ projects enable the `visibility=hidden` compiler flag by default.
   gz-cmake4 changes gz-cmake projects to use C/C++ visibility hidden
   by default. This is a potential breaking changed for projects using
   gz-cmake but the benefits in terms of creating portable code and
   time spend by the loader could be relevant.
   To avoid this change, the EXPOSE_SYMBOLS_BY_DEFAULT flag can be used in
   the gz_configure_project call.
   The change deprecates the HIDDEN_SYMBOLS_BY_DEFAULT flag that can be
   removed.

1. **Breaking**: Now the code generates always a `doc` target that can be
   called to generate the documentation. The `doc` target is excluded from
   the ALL target so it needs to be explicitly triggered.

1. **Deprecated**: `BUILD_DOCS` CMake arguments is deprecated.
    **Replacement**: building docs is excluded from the default build and needs
    to be explicitly triggered by calling the `doc` target.

1. **Deprecated**: `GzPython.cmake`
    **Replacement**: Use `find_package(Python3)` to find Python3 and the
              `Python3_EXECUTABLE` variable instead of `PYTHON_EXECUTABLE`.

## Gazebo CMake 2.X to 3.X

1. **Breaking**: Examples are now built using native cmake.
  Two targets will be generated for each set of examples: `EXAMPLES_Build_TEST` and `EXAMPLES_Configure_TEST`
  Examples are not built by default, but instead require `BUILD_EXAMPLES:bool=True` to be set.
  This is because examples require the package of interest to be installed via `make install`.

1. **Breaking**: The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!
  * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
  * Some non-exhaustive examples of this include:
    * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
    * CMake `-config` files
    * Paths that depend on the project name
:
1. **Deprecated**: include/ignition/utilities/SuppressWarning.hh
    **Replacement**: include/ignition/utils/SuppressWarning.hh
                     (in the gz-utils package)
1. **Deprecated**: include/ignition/utilities/ExtraTestMacros.hh
    **Replacement**: include/ignition/utils/ExtraTestMacros.hh
                     (in the gz-utils package)
1. **Deprecated**: CMake functions and macros starting with `ign_`
    **Replacement**: CMake functions and macros starting with `gz_`
1. **Deprecated**: `ignition` namespaces
    **Replacement**: `gz` namespaces
1. **Deprecated**: `Ign` prefixed CMake files
    **Replacement**: `Gz` prefixed CMake files
1. **Deprecated**: `IGN/IGNITION` prefixed CMake variables and options
    **Replacement**: `GZ` prefixed CMake variables and options

