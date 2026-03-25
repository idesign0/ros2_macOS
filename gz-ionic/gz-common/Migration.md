# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Common 5.X to 6.X

### Modifications

1. Logging has been heavily modified as we're relying on
 [spdlog](https://github.com/gabime/spdlog). See
 [example1](https://github.com/gazebosim/gz-utils/blob/gz-utils3/examples/log/main.cc)
 and [example2]((https://github.com/gazebosim/gz-common/blob/gz-common6/examples/console.cc))
 to learn how to access the internal spdlog logger and the global options.

 The `gzdbg`, `gzmsg`, `gzlog`, `gzwarn` and `gzerr` macros should work the
 same as previous versions but you now have two extra macros: `gzcrit` and
 `gztrace` for logging critical errors and traces respectively.

 `Console::SetVerbosity` only accepts values between 0 (critical errors only)
 and 5 (all log messages). Other values don't have any effect.

1. Removed the `graphics` component's dependency on the GTS
   (GNU Triangulated Surface) library which was used for doing triangulation
   and CSG Boolean operation by the `MeshManager`. The Delaunay triangulation
   function now uses the CDT (Constrained Delaunay Triangulation) library.

### Deletions

1. **MeshManager.hh**
    + `void CreateBoolean(const std::string &_name, const Mesh *_m1,
          const Mesh *_m2, const int _operation,
          const gz::math::Pose3d &_offset = gz::math::Pose3d::Zero)`

## Gazebo Common 4.X to 5.X

### Deprecations

1. `Submesh::MaterialIndex` is deprecated. `SubMesh::GetMaterialIndex` should
   be used instead, which properly handles submeshes having no material index
   applied to them.
2. The `ignition` namespace is deprecated and will be removed in future versions.  Use `gz` instead.
3. Header files under `ignition/...` are deprecated and will be removed in future versions.
   Use `gz/...` instead.
4. The logging macros (`ignmsg`, `ignwarn`, `ignerr`, etc.) and logging function macros
   (`ignLogInit()`, etc.) are deprecated and will be removed in future versions. Use `gz` instead
   (e.g. `gzmsg`, `gzwarn`, `gzLogInit()`)
5. All the plugin APIs are deprecated, use the gz-plugin library instead. See
   the [migration guide](https://github.com/ignitionrobotics/ign-plugin/blob/ign-plugin1/MIGRATION.md).
6. The following `IGN_` prefixed environment variables are deprecated and will be removed.
   Please use the `GZ_` prefixed versions instead!
   1. `IGN_VIDEO_ALLOWED_ENCODERS` -> `GZ_VIDEO_ALLOWED_ENCODERS`
   2. `IGN_VIDEO_ENCODER_DEVICE` -> `GZ_VIDEO_ENCODER_DEVICE`
   3. `IGN_VIDEO_USE_HW_SURFACE` -> `GZ_VIDEO_USE_HW_SURFACE`
   4. `IGN_FILE_PATH` -> `GZ_FILE_PATH`
   5. `IGN_LOG_PATH` -> `GZ_LOG_PATH`
   6. `IGN_PLUGIN_PATH` -> `GZ_PLUGIN_PATH`
7. The following `IGN_` / `IGNITION_` prefixed macros are deprecated and will be removed in future versions.
   Additionally, they will only be available when including the corresponding `ignition/...` header.
   Use the `GZ_` prefix instead.
   1. `IGN_ENUM`
   2. `IGN_HOMEDIR`
   3. `IGN_NANO_TO_SEC`, `IGN_SEC_TO_NANO`, `IGN_MS_TO_NANO`, `IGN_US_TO_NANO`
   4. `IGN_SPEED_OF_LIGHT`
   5. `IGN_SLEEP_S`, `IGN_SLEEP_US`, `IGN_SLEEP_MS`, `IGN_SLEEP_NS`
   6. `IGN_SYSTEM_TIME`, `IGN_SYSTEM_TIME_S`, `IGN_SYSTEM_TIME_US`, `IGN_SYSTEM_TIME_MS`, `IGN_SYSTEM_TIME_NS`
   7. `IGN_ASSERT`
   8. `IGNITION_COMMON_TINYOBJLOADER_IMPLEMENTATION` (`src` local, hard-tocked)
   9. `IGN_PROFILER_ENABLE`, `IGN_PROFILE_THREAD_NAME`, `IGN_PROFILE_LOG_TEXT`, `IGN_PROFILE_BEGIN`, `IGN_PROFILE_END`, `IGN_PROFILE_L`, `IGN_PROFILE`, `IGN_PROFILE_VALID`
   10. `IGN_CREATE_SPEC_INTERFACE`
   11. `IGN_DUMMY_PLUGIN_PATH`
   12. `IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS`
8. The `Image::Data(unsigned char**, unsigned int&)` functions that accept a pointer and a size and internally allocate memory are deprecated and will be removed in future versions.
   Use the new `Data` functions that return a `std::vector<unsigned char>` to have automatic memory management.

### Breaking Changes

1. The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!
  * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
  * Some non-exhaustive examples of this include:
    * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
    * CMake `-config` files
    * Paths that depend on the project name

### Additions

1. **geospatial** component that loads heightmap images and DEMs
    + Depends on the gz-common's `graphics` component and the `gdal` library

### Modifications

1. `HeightmapData.hh` and `ImageHeightmap.hh` have been moved out of the
`graphics` component and into the new `geospatial` component
    + To use the heightmap features, users must add the `geospatial` component
      to the `find_package` call and update the include paths to use
      the geospatial subfolder (`#include <ignition/common/geospatial/HeightmapData.hh>`)

1. `HeightmapData::FillHeightmap` method is now `const`.

1. `Image::AvgColor`, `Image::Data` and `Image::RGBData` methods are now `const`.

## Gazebo Common 3.X to 4.X

### Modifications

1. Corrected `BAYER_RGGR8` to `BAYER_BGGR8` in `PixelFormatName` and
   `PixelFormatType` located in `graphics/include/gz/common/Image.hh`.

1. URI parsing has updated to follow the specification more closely when
   `URI::Authority` is set. Changes include:
    * An empty URI Path is valid.
    * Double forward slashes, `//`, are valid in a URI Path.
    * A URI Query does not require a `key=value` format. For example
    a valid query can be "?aquery", "?aquery?", and `??`.
    * A URI authority is optional. If present, then a URI authority begins
    with two forward slashes and immediately follows the URI scheme. A host must be present if an authority is present and the scheme != 'file'.

## Gazebo Common 2.X to 3.X

### Additions

1. **Event.hh**
    + Added second template argument to `EventT` to disambiguate events
      with matching signatures.

1. **profiler** component that helps measure software performance.

1. **SystemPaths.hh**
    + Search paths specified in `GZ_FILE_PATH` environment variable when
      finding files.

1. **Util.hh**
    + Added `constexpr uint64_t hash64(std::string_view)` to generate a
      hash at compile time if possible.

### Modifications

1. Depends on **gz-cmake2**
    + gz-common now depends on gz-cmake2.

1. Requires c++17.

1. (New in 3.8.0) On Windows, the value of C++ macro `GZ_HOMEDIR` changed from `HOMEPATH` to `USERPROFILE`. It is usually used to read the path to the user's home from environment. The old value pointed to a path relative to the (a) current drive letter as reported by `pwd`, not the system drive letter. The new value correctly points to an environment variable that contains the full absolute path to the user's profile. If the code did not use the macro in some unexpected way, the new behavior should work either the same or even better (it would work even when the current directory is on a non-system drive). If the code relied on this value to be relative to the current drive letter, it needs to be changed to use `HOMEPATH` directly.

## Gazebo Common 1.X to 2.X

### Modifications

1. Depends on **gz-cmake1**
    + gz-common now depends on gz-cmake1, which provides
      support for Component libraries.

1. Component libraries
    + Some classes have been moved from the main gz-common library
      to component libraries. To use these features, you must
      list them in the `find_package` call and link against them.
      - av: audio and video playback and encoding
      - events: registering and handling event callbacks
      - graphics: animation, images, and triangle meshes

## Gazebo Common 0.X to 1.X

### Added dependencies

1. **gz-cmake**
    + gz-math now has a build dependency on gz-cmake, which
      allows cmake scripts to be shared across all the Gazebo packages.
