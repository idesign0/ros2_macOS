# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Rendering 8.x to 9.x

### Deprecations

1. **Ogre2SelectionBuffer**
    + Deprecated: `bool ExecuteQuery(const int _x, const int _y, Ogre::Item *&_item, math::Vector3d &_point)`
    + Replacement: `bool ExecuteQuery(int _x, int _y, Ogre::MovableObject *&_obj, math::Vector3d &_point)`

### Modifications

1. **Ogre2SelectionBuffer**
    + Removed: `Ogre::OgreItem *OnSelectionClick(const int _x, const int _y)`
    + Replacement: `Ogre::MovableObject *OnSelectionClick(int _x, int _y)`

1. **GpuRays**
    + Made function private: `void Copy(Image &_image)`
    + Use the overloaded function: `void Copy(float *_data)`

1. **RenderPass**
    + Made function private: `void PreRender()`
    + Use the overloaded function: `void PreRender(const CameraPtr &_camera)`

## Gazebo Rendering 7.x to 8.x

### Deprecations
1. The following `HAVE_` prefixed macros that are defined in config.hh are deprecated and will be removed in future versions.
    + Deprecated: `HAVE_OGRE`, `HAVE_OGRE2` `HAVE_OPTIX`
    + Replacement: `GZ_RENDERING_HAVE_OGRE`, `GZ_RENDERING_HAVE_OGRE2` `GZ_RENDERING_HAVE_OPTIX`

### Modifications
1. The BaseStore internal data structure has changed from an std::map to an
   std::vector for performance consideration. This means iterators returned by
   BaseStore APIs such as `IterByIndex` may now be different from before since
   the order of objects stored in maps and vectors are different. The iterators
   returned may also change or become invalid when objects are added or removed
   from the store. This impacts users using APIs to access nodes / visuals by
   index, e.g. `Node::ChildByIndex` and `Scene::VisualByIndex` may now
   return a different node pointer.

## Gazebo Rendering 6.x to 7.x

### Deprecations

1. The CMake project is called `gz-renderingN` instead of `ignition-renderingN`.
1. The shared libraries have `gz` where there used to be `ignition`, i.e. `libgz-rendering-ogre.so`
1. **ogre2/include/ignition/rendering/ogre2/Ogre2RenderEngine.hh**
    + Deprecated: `Ogre2IgnHlmsSphericalClipMinDistance &HlmsCustomizations()`
    + Replacement: `Ogre2GzHlmsSphericalClipMinDistance &SphericalClipMinDistance()`
1. The `ignition` namespace is deprecated and will be removed in future versions.  Use `gz` instead.
1. Header files under `ignition/...` are deprecated and will be removed in future versions.
   Use `gz/...` instead.
4. The environment variable `IGN_RENDERING_PLUGIN_PATH` is deprecated. Use `GZ_RENDERING_PLUGIN_PATH` instead.
5. The environment variable `IGN_RENDERING_RESOURCE_PATH` is deprecated. Use `GZ_RENDERING_RESOURCE_PATH` instead.
6. The following `IGN_` prefixed macros are deprecated and will be removed in future versions.
   Additionally, they will only be available when including the corresponding `ignition/...` header.
   Use the `GZ_` prefix instead.
    1. `IGN_RENDERING_REGISTER_RENDER_PASS`
    2. `IGN_VISIBILITY_ALL`
    3. `IGN_VISIBILITY_SELECTION`
    4. `IGN_VISIBILITY_GUI`
    5. `IGN_VISIBILITY_SELECTABLE`
7. The `IgnRenderingMode` enum is deprecated and will be removed in future versions. Use `GzRenderingMode` instead.
    1. Similarly, the `IORM_` prefixed enum values are deprecated. Use the `GORM_` prefix instead.
    2. Also, `Ogre2RenderEngine->SetIgnOgreRenderingMode` is deprecated and will be removed. Use `Ogre2RenderEngine->SetGzOgreRenderingMode` instead.

### Breaking Changes

1. The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!
  * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
  * Some non-exhaustive examples of this include:
    * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
    * CMake `-config` files
    * Paths that depend on the project name

1. Updated Bayer image format enums in `PixelFormat.h` to match the Bayer
formats used in gz-common and gz-msgs.
  * `PF_BAYER_GBGR8` -> `PF_BAYER_GBRG8`
  * `PF_BAYER_GRGB8` -> `PF_BAYER_GRBG8`

## Gazebo Rendering 6.2.1 to 6.X

### Modifications

1. Ogre 2 heightmaps: the Y position sign was flipped

1. `Scene::SetTime` is often unset. Gazebo's `Ogre2` now defaults to 60hz otherwise rendering won't advance forward.
	+ Mostly affects Particles.
	+ Also may affect gaussian postprocessing and other filters dependent on time.
	+ Previous behavior was using real time instead of simulation time, which is wrong.
	+ See https://github.com/gazebosim/gz-rendering/issues/556 for details.

## Gazebo Rendering 5.x to 6.x

### Modifications

1. **Scene.hh**
    + Added `Scene::PostRender`. The function `Camera::Render` must be executed
      between calls to `Scene::PreRender` and `Scene::PostRender`. Failure to do
      so will result in asserts triggering informing users to correct their code.
      Alternatively calling `Scene::SetCameraPassCountPerGpuFlush( 0 )` avoids
      this strict requirement.
      Users handling only one Camera can call `Camera::Update` or `Camera::Capture`
      and thus do not need to worry.
      However for more than one camera (of any type) the optimum way to handle them is to update them in the following pattern:
      ```
      scene->PreRender();
      for( auto& camera in cameras )
          camera->Render();
      for( auto& camera in cameras )
          camera->PostRender();
      scene->PostRender();
      ```
      This pattern maximizes the chances of improving performance.
      *Note*: Calling instead `Camera::Update` for each camera is a waste of CPU resources.
    + It is invalid to modify the scene between `Scene::PreRender` and `Scene::PostRender` (e.g. add/remove objects, lights, etc)
    + Added `Scene::SetCameraPassCountPerGpuFlush`. Setting this value to 0 forces legacy behavior which eases porting.
    + Systems that rely on Graphics components like particle FXs and postprocessing are explicitly affected by Scene's Pre/PostRender. Once `Scene::PostRender` is called, the particle FXs' simulation is moved forward, as well as time values sent to postprocessing shaders. In previous gz-rendering versions each `Camera::Render` call would move the particle simulation forward, which could cause subtle bugs or inconsistencies when Cameras were rendering the same frame from different angles. Setting SetCameraPassCountPerGpuFlush to 0 will also cause these subtle bugs to reappear.

1. **Visual.hh** and **Node.hh**
    + `*UserData` methods and the `Variant` type alias have been moved from the `Visual` class to the `Node` class.
    `Node::UserData` now returns no data for keys that don't exist (prior to Rendering 6.x, if
    `Visual::UserData` was called with a key that doesn't exist, an `int` was returned by default).

1. **depth_camera_fs.glsl** and **depth_camera_final_fs.glsl**
    + Far clipping changed from clipping by depth to clipping by range, i.e. distance to point, so that the data generated will never exceed the specified max range of the camera.

## Gazebo Rendering 4.0 to 4.1

## ABI break

1. **ogre2/include/ignition/rendering/ogre2/Ogre2DepthCamera.hh**
    + Medium severity ABI break with the addition of the `AddRenderPass` override.

## Gazebo Rendering 3.X to 4.X

### Deprecations

1. **gz::common::Time** deprecated in favor of **std::chrono::steady_clock::duration**
    + Deprecated: `void BaseScene::SetSimTime(const common::Time &_time)`
    + Replacement: `void BaseScene::SetTime(const std::chrono::steady_clock::duration &_time)`
    + Deprecated: `common::Time SimTime() const`
    + Replacement: `std::chrono::steady_clock::duration Time() const`
    + Deprecated: `common::Time simTime`
    + Replacement: `std::chrono::steady_clock::duration time`

## Gazebo Rendering 2.X to 3.X

### Deletions

1. **Ogre2DepthCamera.hh**
    + Removed unused member variables `captureData` and `newData`

## Gazebo Rendering 1.X to 2.X

### Modifications

1. **Visual.hh** and **Node.hh**
    + All functions for setting and getting scale properties in the Visual class
      are moved to the base Node class. These functions are now inherited by
      the Visual class.
