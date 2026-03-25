# Gazebo Rendering

### Gazebo Rendering 9.X

### Gazebo Rendering 9.5.0 (2025-11-14)

1. Increase tolerance for heightmap test
    * [Pull request #1179](https://github.com/gazebosim/gz-rendering/pull/1179)

1. Increase depth camera integration test error tolerance for ogre 1.x on arm based mac
    * [Pull request #1180](https://github.com/gazebosim/gz-rendering/pull/1180)

1. Conditional OGRE2 Overlay Support
  * By default, OGRE2 overlay support is enabled, but it can be disabled using the
    CMake option `USE_OGRE2_OVERLAY=OFF`. This is useful for builds using libc++
    version 19 or greater than are broken, see (<https://github.com/OGRECave/ogre-next/issues/541>).
    * [Pull request #1177](https://github.com/gazebosim/gz-rendering/pull/1177)

1. Prevent material destruction if setting the same material to a submesh
    * [Pull request #1168](https://github.com/gazebosim/gz-rendering/pull/1168)

### Gazebo Rendering 9.4.0 (2025-09-05)

1. Add bazel build for ogre2 engine
    * [Pull request #1157](https://github.com/gazebosim/gz-rendering/pull/1157)

### Gazebo Rendering 9.3.0 (2025-07-25)

1. Guard against bad memory access in RenderPassSystem
    * [Pull request #1153](https://github.com/gazebosim/gz-rendering/pull/1153)

1. Enable loading a render engine plugin from the static plugin registry
    * [Pull request #1152](https://github.com/gazebosim/gz-rendering/pull/1152)

1. Fix user settings of thermal camera being ignored
    * [Pull request #1138](https://github.com/gazebosim/gz-rendering/pull/1138)

1. CppCheck fixes
    * [Pull request #1148](https://github.com/gazebosim/gz-rendering/pull/1148)

### Gazebo Rendering 9.2.0 (2025-06-06)

1. Disable a couple of tests on windows
    * [Pull request #1126](https://github.com/gazebosim/gz-rendering/pull/1126)

1. Enable bazel build (core only)
    * [Pull request #1144](https://github.com/gazebosim/gz-rendering/pull/1144)

1. Hide Ogre and Ogre2 external window on MacOS and Windows
    * [Pull request #1141](https://github.com/gazebosim/gz-rendering/pull/1141)

1. Fix clamp of temperature in thermal camera output (#1133)
    * [Pull request #1135](https://github.com/gazebosim/gz-rendering/pull/1135)

1. Change to std::get_if for gz::rendering::Variant for throwless behaviour
    * [Pull request #1124](https://github.com/gazebosim/gz-rendering/pull/1124)

1. Added missing includes
    * [Pull request #1128](https://github.com/gazebosim/gz-rendering/pull/1128)

1.  Add sanity check for valid submesh indices
    * [Pull request #1123](https://github.com/gazebosim/gz-rendering/pull/1123)

1. Remove unused variable in OgreRenderingEngine.cc
    * [Pull request #1119](https://github.com/gazebosim/gz-rendering/pull/1119)

1. Fix spelling issues in documentation
    * [Pull request #1114](https://github.com/gazebosim/gz-rendering/pull/1114)
    * [Pull request #1115](https://github.com/gazebosim/gz-rendering/pull/1115)

1. Remove ClearMaterialsCache call when a material is destroyed in ogre2
    * [Pull request #1110](https://github.com/gazebosim/gz-rendering/pull/1110)

### Gazebo Rendering 9.1.0 (2025-01-27)

1. Visualize Frustum
    * [Pull request #1095](https://github.com/gazebosim/gz-rendering/pull/1095)
    * [Pull request #1102](https://github.com/gazebosim/gz-rendering/pull/1102)

1. Fix RenderingIface_TEST for ogre2 when there is no display
    * [Pull request #1098](https://github.com/gazebosim/gz-rendering/pull/1098)

1. Extend wide angle cameras to support L8 and L16 image formats
    * [Pull request #1097](https://github.com/gazebosim/gz-rendering/pull/1097)

1. Better approach to use std::variant
    * [Pull request #1093](https://github.com/gazebosim/gz-rendering/pull/1093)

1. Do not create wirebox with empty AABB (ogre)
    * [Pull request #1083](https://github.com/gazebosim/gz-rendering/pull/1083)

1. Check valid camera near far clip distances
    * [Pull request #1082](https://github.com/gazebosim/gz-rendering/pull/1082)

1. Fix Windows github action workflow
    * [Pull request #1080](https://github.com/gazebosim/gz-rendering/pull/1080)

1. Unload rendering plugin also on Windows
    * [Pull request #1074](https://github.com/gazebosim/gz-rendering/pull/1074)

1. Fix running rendering tests in GZ_ENABLE_RELOCATABLE_INSTALL is ON
    * [Pull request #1073](https://github.com/gazebosim/gz-rendering/pull/1073)

1. Fix installation with GZ_ENABLE_RELOCATABLE_INSTALL=ON
    * [Pull request #1071](https://github.com/gazebosim/gz-rendering/pull/1071)

### Gazebo Rendering 9.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 8.2.0 and earlier.

1. Update code for loading normal and albedo maps from image data
    * [Pull request #1068](https://github.com/gazebosim/gz-rendering/pull/1068)

1. examples/depth_camera: Fixed windows usage
    * [Pull request #1060](https://github.com/gazebosim/gz-rendering/pull/1060)

1. examples/heightmap: Fixed Windows build
    * [Pull request #1062](https://github.com/gazebosim/gz-rendering/pull/1062)

1. Add Github Action for Windows and fix build
    * [Pull request #1049](https://github.com/gazebosim/gz-rendering/pull/1049)

1. Miscellaneous documentation fixes
    * [Pull request #1051](https://github.com/gazebosim/gz-rendering/pull/1051)
    * [Pull request #1053](https://github.com/gazebosim/gz-rendering/pull/1053)
    * [Pull request #1054](https://github.com/gazebosim/gz-rendering/pull/1054)
    * [Pull request #1055](https://github.com/gazebosim/gz-rendering/pull/1055)
    * [Pull request #1061](https://github.com/gazebosim/gz-rendering/pull/1061)
    * [Pull request #1064](https://github.com/gazebosim/gz-rendering/pull/1064)
    * [Pull request #1069](https://github.com/gazebosim/gz-rendering/pull/1069)

1. Update badges to point to gz-rendering9 branch
    * [Pull request #1044](https://github.com/gazebosim/gz-rendering/pull/1044)

1. Ionic Changelog
    * [Pull request #1042](https://github.com/gazebosim/gz-rendering/pull/1042)

1. Remove gl preference in cmake
    * [Pull request #1041](https://github.com/gazebosim/gz-rendering/pull/1041)

1. Fix segmentation camera color id calculation
    * [Pull request #1040](https://github.com/gazebosim/gz-rendering/pull/1040)

1. Expose shadow texture size for directional lighting in SDF
    * [Pull request #1034](https://github.com/gazebosim/gz-rendering/pull/1034)

1. Workaround for ogre crash on shutdown on noble
    * [Pull request #1033](https://github.com/gazebosim/gz-rendering/pull/1033)

1. Fix overloaded virtual warnings on noble
    * [Pull request #1036](https://github.com/gazebosim/gz-rendering/pull/1036)

1. Enable 24.04 CI, require cmake 3.22.1
    * [Pull request #1037](https://github.com/gazebosim/gz-rendering/pull/1037)

1. Hide optix related cmake warnings
    * [Pull request #1032](https://github.com/gazebosim/gz-rendering/pull/1032)

1. Update troubleshooting link
    * [Pull request #1030](https://github.com/gazebosim/gz-rendering/pull/1030)

1. Fixed integer underflow in OgreDistortionPass
    * [Pull request #994](https://github.com/gazebosim/gz-rendering/pull/994)

1. Add gamma correction to simple_demo_qml example
    * [Pull request #1019](https://github.com/gazebosim/gz-rendering/pull/1019)

1. Change return type of ogre 2 ray query related functions from Ogre::Item to Ogre::MovableObject
    * [Pull request #1024](https://github.com/gazebosim/gz-rendering/pull/1024)

1. Add projection matrix functions to ogre2 camera classes
    * [Pull request #1021](https://github.com/gazebosim/gz-rendering/pull/1021)

1. Remove deprecated functions and enums
    * [Pull request #1031](https://github.com/gazebosim/gz-rendering/pull/1031)
    * [Pull request #1020](https://github.com/gazebosim/gz-rendering/pull/1020)

1. Optimization: remove extra copy of data buffer in Ogre2GpuRays and Ogre2DepthCamera
    * [Pull request #1022](https://github.com/gazebosim/gz-rendering/pull/1022)

1. Fix crash when wide angle camera FOV is > 180
    * [Pull request #1016](https://github.com/gazebosim/gz-rendering/pull/1016)

1. Fix debug builds of gz-rendering-ogre2 plugin
    * [Pull request #1014](https://github.com/gazebosim/gz-rendering/pull/1014)

1. Fix lidar 1st pass texture resolution calculation for FOV < 90 degrees
    * [Pull request #1012](https://github.com/gazebosim/gz-rendering/pull/1012)

1. Use single cubemap camera in lidar
    * [Pull request #1013](https://github.com/gazebosim/gz-rendering/pull/1013)

1. Adding cone primitives.
    * [Pull request #1003](https://github.com/gazebosim/gz-rendering/pull/1003)

1. Parse correctly OGRE2_RESOURCE_PATH on Windows
    * [Pull request #996](https://github.com/gazebosim/gz-rendering/pull/996)

1. Find python directly instead of using GzPython
    * [Pull request #990](https://github.com/gazebosim/gz-rendering/pull/990)

1. Fix simple_demo on Windows
    * [Pull request #987](https://github.com/gazebosim/gz-rendering/pull/987)

1. Fix unused variable warnings
    * [Pull request #989](https://github.com/gazebosim/gz-rendering/pull/989)

1. Fix compilation on simple_demo_qml on Windows
    * [Pull request #986](https://github.com/gazebosim/gz-rendering/pull/986)

1. Work around conda-forge's CDT version of GL
    * [Pull request #983](https://github.com/gazebosim/gz-rendering/pull/983)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #953](https://github.com/gazebosim/gz-rendering/pull/953)

1. Use HIDE_SYMBOLS_BY_DEFAULT
    * [Pull request #941](https://github.com/gazebosim/gz-rendering/pull/941)

1. Bumps in Ionic: gz-rendering9
    * [Pull request #935](https://github.com/gazebosim/gz-rendering/pull/935)
    * [Pull request #936](https://github.com/gazebosim/gz-rendering/pull/936)

1. Add LookAt function to GizmoVisual class
    * [Pull request #882](https://github.com/gazebosim/gz-rendering/pull/882)

1. Add projection matrix set/get functions to Ogre Depth camera
    * [Pull request #928](https://github.com/gazebosim/gz-rendering/pull/928)

## Gazebo Rendering 8.X

### Gazebo Rendering 8.2.2 (2025-01-30)

1. Fix RenderingIface_TEST for ogre2 when there is no display
    * [Pull request #1098](https://github.com/gazebosim/gz-rendering/pull/1098)

1. Better approach to use std::variant
    * [Pull request #1093](https://github.com/gazebosim/gz-rendering/pull/1093)

### Gazebo Rendering 8.2.1 (2024-11-08)

1. Update code for loading normal and albedo maps from image data
    * [Pull request #1068](https://github.com/gazebosim/gz-rendering/pull/1068)

1. Check for zero quaternion value in MoveToHelper
    * [Pull request #1076](https://github.com/gazebosim/gz-rendering/pull/1076)

1. Fix waves 'double' to 'float' truncation warning
    * [Pull request #1065](https://github.com/gazebosim/gz-rendering/pull/1065)

1. Install Roboto fonts to ogre plugin
    * [Pull request #1035](https://github.com/gazebosim/gz-rendering/pull/1035)

1. Fixed integer underflow in OgreDistortionPass
    * [Pull request #994](https://github.com/gazebosim/gz-rendering/pull/994)

1. Add gamma correction to simple_demo_qml example
    * [Pull request #1019](https://github.com/gazebosim/gz-rendering/pull/1019)

1. Optimization: remove extra copy of data buffer in Ogre2GpuRays and Ogre2DepthCamera
    * [Pull request #1022](https://github.com/gazebosim/gz-rendering/pull/1022)

1. Fix crash when wide angle camera FOV is > 180
    * [Pull request #1016](https://github.com/gazebosim/gz-rendering/pull/1016)

1. Fix debug builds of gz-rendering-ogre2 plugin
    * [Pull request #1014](https://github.com/gazebosim/gz-rendering/pull/1014)

1. Fix lidar 1st pass texture resolution calculation for FOV < 90 degrees
    * [Pull request #1012](https://github.com/gazebosim/gz-rendering/pull/1012)

1. Use single cubemap camera in lidar
    * [Pull request #1013](https://github.com/gazebosim/gz-rendering/pull/1013)

1. Parse correctly OGRE2_RESOURCE_PATH on Windows
    * [Pull request #996](https://github.com/gazebosim/gz-rendering/pull/996)

### Gazebo Rendering 8.2.0 (2024-06-19)

1. Backport: Adding cone primitives.
    * [Pull request #1003](https://github.com/gazebosim/gz-rendering/pull/1003)

1. Fixes deleter passed to the std::shared_ptr<void>
    * [Pull request #1009](https://github.com/gazebosim/gz-rendering/pull/1009)

1. ogre2: Set custom projection matrix for other types of cameras
    * [Pull request #1002](https://github.com/gazebosim/gz-rendering/pull/1002)

1. Fix gz-cmake declaration on package.xml (Fix windows builds)
    * [Pull request #1005](https://github.com/gazebosim/gz-rendering/pull/1005)

1. Add package.xml
    * [Pull request #981](https://github.com/gazebosim/gz-rendering/pull/981)

1. Workaround on warnings for Ubuntu Noble
    * [Pull request #995](https://github.com/gazebosim/gz-rendering/pull/995)

1. Ogre2RenderEngine: on Windows if useCurrentGLContext is specified, set the externalWindowsHandle ogre-next option
    * [Pull request #992](https://github.com/gazebosim/gz-rendering/pull/992)

### Gazebo Rendering 8.1.1 (2024-04-10)

1. Use relative install paths for plugin shared libraries and media files
    * [Pull request #984](https://github.com/gazebosim/gz-rendering/pull/984)

1. Handle null mesh pointers in examples
    * [Pull request #982](https://github.com/gazebosim/gz-rendering/pull/982)

### Gazebo Rendering 8.1.0 (2024-03-14)

1. Fix setting normal map when not all submeshes have texcoords (ogre2)
    * [Pull request #976](https://github.com/gazebosim/gz-rendering/pull/976)

1. Update ogre2 symbol visibility macro in private classes
    * [Pull request #972](https://github.com/gazebosim/gz-rendering/pull/972)

1. Improve Ogre2DepthCamera  performance
    * [Pull request #971](https://github.com/gazebosim/gz-rendering/pull/971)
    * [Pull request #965](https://github.com/gazebosim/gz-rendering/pull/965)

1. Fix repeatedly loading engine when calling sceneFromFirstEngine
    * [Pull request #961](https://github.com/gazebosim/gz-rendering/pull/961)

1. Improve Ogre2GpuRays performance
    * [Pull request #955](https://github.com/gazebosim/gz-rendering/pull/955)
    * [Pull request #973](https://github.com/gazebosim/gz-rendering/pull/973)

1. Correctly set position-independent code
    * [Pull request #950](https://github.com/gazebosim/gz-rendering/pull/950)

1. Remove todo in gpu ray test
    * [Pull request #949](https://github.com/gazebosim/gz-rendering/pull/949)

1. Update CI badges to point to release branch job
    * [Pull request #948](https://github.com/gazebosim/gz-rendering/pull/948)
    * [Pull request #947](https://github.com/gazebosim/gz-rendering/pull/947)

1. Set projector emissive texture
    * [Pull request #946](https://github.com/gazebosim/gz-rendering/pull/946)

1. Add minor comments to BaseGizmoVisual
    * [Pull request #881](https://github.com/gazebosim/gz-rendering/pull/881)

1. Update examples to use metal as default graphics API on mac
    * [Pull request #937](https://github.com/gazebosim/gz-rendering/pull/937)

1. Extend ogre 1.x custom shaders support
    * [Pull request #908](https://github.com/gazebosim/gz-rendering/pull/908)

1. Support skybox in wide angle cam view
    * [Pull request #901](https://github.com/gazebosim/gz-rendering/pull/901)

1. Minor fix for projector visibility flags
    * [Pull request #920](https://github.com/gazebosim/gz-rendering/pull/920)

1. Fix use of srgb format for texture map data
    * [Pull request #931](https://github.com/gazebosim/gz-rendering/pull/931)

1. Infrastructure
    * [Pull request #925](https://github.com/gazebosim/gz-rendering/pull/925)
    * [Pull request #939](https://github.com/gazebosim/gz-rendering/pull/939)
    * [Pull request #940](https://github.com/gazebosim/gz-rendering/pull/940)
    * [Pull request #967](https://github.com/gazebosim/gz-rendering/pull/967)

1. Fix float comparison warnings in test
    * [Pull request #929](https://github.com/gazebosim/gz-rendering/pull/929)

### Gazebo Rendering 8.0.0 (2023-09-29)

1. Remove gazebo scene viewer example
    * [Pull request #915](https://github.com/gazebosim/gz-rendering/pull/915)

1. Fix projector FOV, orientation, and crashes
    * [Pull request #913](https://github.com/gazebosim/gz-rendering/pull/913)

1. Include the CGL header on macOS
    * [Pull request #911](https://github.com/gazebosim/gz-rendering/pull/911)

1. ogre2: use CMAKE_INSTALL_RPATH
    * [Pull request #909](https://github.com/gazebosim/gz-rendering/pull/909)

1. Fixed build on MSVC.
    * [Pull request #897](https://github.com/gazebosim/gz-rendering/pull/897)

1. Documentation fixes
    * [Pull request #895](https://github.com/gazebosim/gz-rendering/pull/895)
    * [Pull request #906](https://github.com/gazebosim/gz-rendering/pull/906)
    * [Pull request #907](https://github.com/gazebosim/gz-rendering/pull/907)

1. Fix wide angle camera visibility flags
    * [Pull request #900](https://github.com/gazebosim/gz-rendering/pull/900)

1. Fix arrow visual indexing
    * [Pull request #889](https://github.com/gazebosim/gz-rendering/pull/889)

1. Do not load optix engine in examples
    * [Pull request #893](https://github.com/gazebosim/gz-rendering/pull/893)

1. Infrastructure
    * [Pull request #888](https://github.com/gazebosim/gz-rendering/pull/888)
    * [Pull request #868](https://github.com/gazebosim/gz-rendering/pull/868)

1. Clean up internal resources on Destroy
    * [Pull request #886](https://github.com/gazebosim/gz-rendering/pull/886)

1. ign -> gz
    * [Pull request #877](https://github.com/gazebosim/gz-rendering/pull/877)

1. Remove inheritance from Singleton class.
    * [Pull request #876](https://github.com/gazebosim/gz-rendering/pull/876)
    * [Pull request #874](https://github.com/gazebosim/gz-rendering/pull/874)

1. Add private data class to Image
    * [Pull request #875](https://github.com/gazebosim/gz-rendering/pull/875)

1. Override PreRender function in Ogre2ParticleEnitter
    * [Pull request #871](https://github.com/gazebosim/gz-rendering/pull/871)

1. Fix calling GpuRays::Destroy more than once
    * [Pull request #860](https://github.com/gazebosim/gz-rendering/pull/860)

1. Use std::vector instead of std::map in BaseStorage
    * [Pull request #831](https://github.com/gazebosim/gz-rendering/pull/831)

1. Limit Global Illumination CI CVT method to vulkan backend
    * [Pull request #798](https://github.com/gazebosim/gz-rendering/pull/798)

1. Global illumination VCT & CI VCT
    * [Pull request #675](https://github.com/gazebosim/gz-rendering/pull/675)

1. Add LensFlare to ogre1
    * [Pull request #775](https://github.com/gazebosim/gz-rendering/pull/775)

1. Cleanup config.hh.in and deprecate public HAVE_ macros
    * [Pull request #791](https://github.com/gazebosim/gz-rendering/pull/791)

1. Add GZ_RENDERING_HAVE_VULKAN to guard vulkan code in header
    * [Pull request #790](https://github.com/gazebosim/gz-rendering/pull/790)

1. Look for vulkan in CMakeLists
    * [Pull request #782](https://github.com/gazebosim/gz-rendering/pull/782)

1. Add LensFlarePass (ogre2 engine)
    * [Pull request #752](https://github.com/gazebosim/gz-rendering/pull/752)

1. Add Support for wide-angle cameras in ogre2
    * [Pull request #733](https://github.com/gazebosim/gz-rendering/pull/733)

1. Fix wrong Z check in OgreWideAngleCamera::Project3d
    * [Pull request #746](https://github.com/gazebosim/gz-rendering/pull/746)

1. Add necessary headers for Vulkan QML GUI backend
    * [Pull request #706](https://github.com/gazebosim/gz-rendering/pull/706)

1. Revert "Expose camera intrinsic matrix API (#700)"
    * [Pull request #756](https://github.com/gazebosim/gz-rendering/pull/756)

1. Expose camera intrinsic matrix API
    * [Pull request #700](https://github.com/gazebosim/gz-rendering/pull/700)

1. ⬆️  Bump main to 8.0.0~pre1
    * [Pull request #699](https://github.com/gazebosim/gz-rendering/pull/699)


### Gazebo Rendering 7.X

### Gazebo Rendering 7.4.2 (2023-09-26)

1. ogre2: use CMAKE_INSTALL_RPATH
    * [Pull request #909](https://github.com/gazebosim/gz-rendering/pull/909)

1. Disable setting color range in particle emitter
    * [Pull request #903](https://github.com/gazebosim/gz-rendering/pull/903)

1. Documentation updates
    * [Pull request #892](https://github.com/gazebosim/gz-rendering/pull/892)
    * [Pull request #896](https://github.com/gazebosim/gz-rendering/pull/896)

1. Revert mesh viewer background color back to gray
    * [Pull request #894](https://github.com/gazebosim/gz-rendering/pull/894)

### Gazebo Rendering 7.4.1 (2023-08-17)

1. Infrastructure
    * [Pull request #879](https://github.com/gazebosim/gz-rendering/pull/879)
    * [Pull request #878](https://github.com/gazebosim/gz-rendering/pull/878)

1. Restore missing Ogre2Heightmap::DestroyImpl implementation
    * [Pull request #870](https://github.com/gazebosim/gz-rendering/pull/870)

1. Destroy sensor node on exit
    * [Pull request #869](https://github.com/gazebosim/gz-rendering/pull/869)

1. Parse `OGRE2_RESOURCE_PATH` as a list of strings
    * [Pull request #867](https://github.com/gazebosim/gz-rendering/pull/867)

1. Fix a leak in ogre2 ray query
    * [Pull request #866](https://github.com/gazebosim/gz-rendering/pull/866)

1. Fixed light visual in OGRE
    * [Pull request #862](https://github.com/gazebosim/gz-rendering/pull/862)

1. Removed unused parameter warning
    * [Pull request #861](https://github.com/gazebosim/gz-rendering/pull/861)

1. Replace ignerr with gzerr
    * [Pull request #857](https://github.com/gazebosim/gz-rendering/pull/857)

1. Minor cleanup of ogre/ogre2: fix namespace, unused variable, missing header
    * [Pull request #858](https://github.com/gazebosim/gz-rendering/pull/858)

1. Fix missing header in CameraLens
    * [Pull request #855](https://github.com/gazebosim/gz-rendering/pull/855)

1. Fix missing cstdint header in latest gcc build
    * [Pull request #853](https://github.com/gazebosim/gz-rendering/pull/853)

1. OgreNext: Fix shaders compilation on d3d12 mesa driver (WSLg)
    * [Pull request #851](https://github.com/gazebosim/gz-rendering/pull/851)

1. Fix visibility decorator of Ogre2Projector
    * [Pull request #850](https://github.com/gazebosim/gz-rendering/pull/850)

### Gazebo Rendering 7.4.0 (2023-05-18)

1. Add optional binary relocatability
    * [Pull request #804](https://github.com/gazebosim/gz-rendering/pull/804)

1. Add Projector
    * [Pull request #845](https://github.com/gazebosim/gz-rendering/pull/845)

1. Add support for bayer images to Ogre and Ogre2
    * [Pull request #838](https://github.com/gazebosim/gz-rendering/pull/838)

1. Fix requiring OpenGL on macOS
    * [Pull request #844](https://github.com/gazebosim/gz-rendering/pull/844)

### Gazebo Rendering 7.3.1 (2023-04-13)

1. Lower severity level for ogre2 visibility mask msgs and unavailable render passes
    * [Pull request #830](https://github.com/gazebosim/gz-rendering/pull/830)

1.  Fix deleting ogre2 when created (and deleted) in a thread
    * [Pull request #840](https://github.com/gazebosim/gz-rendering/pull/840)
    * [Pull request #841](https://github.com/gazebosim/gz-rendering/pull/841)

1. CI workflow: use checkout v3
    * [Pull request #834](https://github.com/gazebosim/gz-rendering/pull/834)

1. Rename COPYING to LICENSE
    * [Pull request #833](https://github.com/gazebosim/gz-rendering/pull/833)

1. Add message to see troubleshooting page when render engine fails to create dummy window
    * [Pull request #829](https://github.com/gazebosim/gz-rendering/pull/829)

1. Tweak max camera position limit
    * [Pull request #827](https://github.com/gazebosim/gz-rendering/pull/827)

### Gazebo Rendering 7.3.0 (2023-03-03)

1. Limit max camera position vector length
    * [Pull request #824](https://github.com/gazebosim/gz-rendering/pull/824)

1. Cleanup GpuRays properly
    * [Pull request #817](https://github.com/gazebosim/gz-rendering/pull/817)

1. Ensure composite BaseVisuals are destroyed correctly.
    * [Pull request #816](https://github.com/gazebosim/gz-rendering/pull/816)

1. Remove fini to resolve segfaault at shutdown
    * [Pull request #813](https://github.com/gazebosim/gz-rendering/pull/813)

1. Fix transparency over heightmap
    * [Pull request #811](https://github.com/gazebosim/gz-rendering/pull/811)

1. Fix duplicate PreRender calls
    * [Pull request #809](https://github.com/gazebosim/gz-rendering/pull/809)

1. Mesh cleanup on destruction
    * [Pull request #807](https://github.com/gazebosim/gz-rendering/pull/807)

1. Fix gpu lidar memory leak [ogre2]
    * [Pull request #799](https://github.com/gazebosim/gz-rendering/pull/799)

1. Fix selecting semi-transparent visuals
    * [Pull request #800](https://github.com/gazebosim/gz-rendering/pull/800)

1. Fix Ogre2DepthCamera on macOS (Fortress)
    * [Pull request #797](https://github.com/gazebosim/gz-rendering/pull/797)

1. Add Heightmap integration test
    * [Pull request #785](https://github.com/gazebosim/gz-rendering/pull/785)

1. More ign to gz changes
    * [Pull request #795](https://github.com/gazebosim/gz-rendering/pull/795)

1. Restore GLX context after OGRE plugin loading finishes.
    * [Pull request #794](https://github.com/gazebosim/gz-rendering/pull/794)

1. Add message to see troubleshooting page when render engine fails to initialize
    * [Pull request #793](https://github.com/gazebosim/gz-rendering/pull/793)

1.  Do not overwrite reserved visibility flags in ogre2
    * [Pull request #784](https://github.com/gazebosim/gz-rendering/pull/784)
    * [Pull request #783](https://github.com/gazebosim/gz-rendering/pull/783)

1. Fix Metal shader compiler error in GpuRays (Lidar)
    * [Pull request #786](https://github.com/gazebosim/gz-rendering/pull/786)

1.  Fix memory corruption in Ogre2GpuRays
    * [Pull request #788](https://github.com/gazebosim/gz-rendering/pull/788)
    * [Pull request #787](https://github.com/gazebosim/gz-rendering/pull/787)

1. Enable ray heightmap ray query
    * [Pull request #778](https://github.com/gazebosim/gz-rendering/pull/778)

1. Rename ign to gz in LuxCore demo
    * [Pull request #781](https://github.com/gazebosim/gz-rendering/pull/781)

1. Fix RenderTarget::destroy not getting called in both ogre1 & ogre2
    * [Pull request #776](https://github.com/gazebosim/gz-rendering/pull/776)

1. Wrong error log message
    * [Pull request #777](https://github.com/gazebosim/gz-rendering/pull/777)

1. Implement consistent Auto Aspect Ratio
    * [Pull request #764](https://github.com/gazebosim/gz-rendering/pull/764)

1. Fix bounding box for glbs
    * [Pull request #769](https://github.com/gazebosim/gz-rendering/pull/769)

1. Disable flaky VisualAt test on mac
    * [Pull request #767](https://github.com/gazebosim/gz-rendering/pull/767)

1. Use proper sRGB conversion in DepthCamera
    * [Pull request #766](https://github.com/gazebosim/gz-rendering/pull/766)

1. Enable Heightmap Test for ogre2 & fix crash when using GpuRays camera
    * [Pull request #762](https://github.com/gazebosim/gz-rendering/pull/762)

1. Fix non-deterministic rendering due to background texture loading
    * [Pull request #765](https://github.com/gazebosim/gz-rendering/pull/765)

1. ign -> gz Migrate Ignition Headers : gz-rendering
    * [Pull request #705](https://github.com/gazebosim/gz-rendering/pull/705)

1. Fix lidar heightmap detection
    * [Pull request #760](https://github.com/gazebosim/gz-rendering/pull/760)

1. bump to 3.6.1 and update changelog
    * [Pull request #759](https://github.com/gazebosim/gz-rendering/pull/759)

1. Improved coverage: MeshDescriptor, Mesh, MoveToHelper, OrbitViewController, PixelFormat and ShadersParams
    * [Pull request #748](https://github.com/gazebosim/gz-rendering/pull/748)

1. Suppress Windows warnings
    * [Pull request #749](https://github.com/gazebosim/gz-rendering/pull/749)

### Gazebo Rendering 7.2.0 (2022-11-14)

1. Enable gpu ray query on macOS.
    * [Pull request #758](https://github.com/gazebosim/gz-rendering/pull/758)

1. Calculate camera intrinsics : Refactor https://github.com/gazebosim/gz-rendering/pull/700
    * [Pull request #755](https://github.com/gazebosim/gz-rendering/pull/755)

### Gazebo Rendering 7.1.0 (2022-10-31)

1. Initialize wide angle camera pixel format
    * [Pull request #735](https://github.com/gazebosim/gz-rendering/pull/735)

1. Fix use of singletons
    * [Pull request #734](https://github.com/gazebosim/gz-rendering/pull/734)

1. All changes merged forward from ign-rendering6
    * [Pull request #750](https://github.com/gazebosim/gz-rendering/pull/750)
    * [Pull request #743](https://github.com/gazebosim/gz-rendering/pull/743)

### Gazebo Rendering 7.0.0 (2022-09-22)

1. Fix build in case no suitable OGRE is found
    * [Pull request #726](https://github.com/gazebosim/ign-rendering/pull/726)

1. Fix destroying wide angle camera
    * [Pull request #718](https://github.com/gazebosim/ign-rendering/pull/718)

1. Remove unused point cloud color uniform variable
    * [Pull request #721](https://github.com/gazebosim/ign-rendering/pull/721)

1. Camera ShaderSelection: avoid seg-fault
    * [Pull request #720](https://github.com/gazebosim/ign-rendering/pull/720)

1. Fix setting emissive map
    * [Pull request #723](https://github.com/gazebosim/ign-rendering/pull/723)

1. Tutorial updates
    * [Pull request #717](https://github.com/gazebosim/ign-rendering/pull/717)
    * [Pull request #716](https://github.com/gazebosim/ign-rendering/pull/716)
    * [Pull request #715](https://github.com/gazebosim/ign-rendering/pull/715)
    * [Pull request #722](https://github.com/gazebosim/ign-rendering/pull/722)
    * [Pull request #724](https://github.com/gazebosim/ign-rendering/pull/724)
    * [Pull request #725](https://github.com/gazebosim/ign-rendering/pull/725)
    * [Pull request #714](https://github.com/gazebosim/ign-rendering/pull/714)
    * [Pull request #713](https://github.com/gazebosim/ign-rendering/pull/713)

1. ign -> gz
    * [Pull request #679](https://github.com/gazebosim/ign-rendering/pull/679)
    * [Pull request #672](https://github.com/gazebosim/ign-rendering/pull/672)
    * [Pull request #669](https://github.com/gazebosim/ign-rendering/pull/669)
    * [Pull request #657](https://github.com/gazebosim/ign-rendering/pull/657)
    * [Pull request #656](https://github.com/gazebosim/ign-rendering/pull/656)
    * [Pull request #645](https://github.com/gazebosim/ign-rendering/pull/645)
    * [Pull request #643](https://github.com/gazebosim/ign-rendering/pull/643)
    * [Pull request #652](https://github.com/gazebosim/ign-rendering/pull/652)
    * [Pull request #639](https://github.com/gazebosim/ign-rendering/pull/639)
    * [Pull request #629](https://github.com/gazebosim/ign-rendering/pull/629)
    * [Pull request #631](https://github.com/gazebosim/ign-rendering/pull/631)
    * [Pull request #618](https://github.com/gazebosim/ign-rendering/pull/618)
    * [Pull request #677](https://github.com/gazebosim/ign-rendering/pull/677)

1. Provide a Virtual Method Anchor for Classes in Headers
    * [Pull request #708](https://github.com/gazebosim/ign-rendering/pull/708)

1. Add regression test for multiple engine reloads
    * [Pull request #704](https://github.com/gazebosim/ign-rendering/pull/704)

1. Fix INTEGRATION_camera/shader_selection on macOS
    * [Pull request #703](https://github.com/gazebosim/ign-rendering/pull/703)

1. Hotfix typo in ogre2 include statement
    * [Pull request #709](https://github.com/gazebosim/ign-rendering/pull/709)

1. Destroy GpuRay camera on exit
    * [Pull request #698](https://github.com/gazebosim/ign-rendering/pull/698)

1. Support for textures as binary blobs
    * [Pull request #642](https://github.com/gazebosim/ign-rendering/pull/642)

1. Port gz-rendering to Ogre 2.3
    * [Pull request #553](https://github.com/gazebosim/ign-rendering/pull/553)

1. Add coverage ignore file
    * [Pull request #689](https://github.com/gazebosim/ign-rendering/pull/689)

1. Ensure one texture per selection buffer per camera.
    * [Pull request #665](https://github.com/gazebosim/ign-rendering/pull/665)

1. Update GoogleTest to latest version
    * [Pull request #658](https://github.com/gazebosim/ign-rendering/pull/658)

1. Use pose multiplication instead of subtraction
    * [Pull request #644](https://github.com/gazebosim/ign-rendering/pull/644)

1. Suppress a new Jammy warnings
    * [Pull request #632](https://github.com/gazebosim/ign-rendering/pull/632)

1. Dependency bumps in garden : ign-rendering7
    * [Pull request #610](https://github.com/gazebosim/ign-rendering/pull/610)
    * [Pull request #608](https://github.com/gazebosim/ign-rendering/pull/608)
    * [Pull request #525](https://github.com/gazebosim/ign-rendering/pull/525)

1. hello_world_plugin: fix find version variables
    * [Pull request #607](https://github.com/gazebosim/ign-rendering/pull/607)

1. Replace deprecated common::SubMesh::MaterialIndex() with GetMaterialIndex()
    * [Pull request #606](https://github.com/gazebosim/ign-rendering/pull/606)

1. Separate cache files for each heightmap LOD (ogre 1.x)
    * [Pull request #595](https://github.com/gazebosim/ign-rendering/pull/595)

1. Default cameraPassCountPerGpuFlush to 6
    * [Pull request #601](https://github.com/gazebosim/ign-rendering/pull/601)

1. Don't crash if directly set low level materials are used in sensors
    * [Pull request #593](https://github.com/gazebosim/ign-rendering/pull/593)

1. Remove Bionic from future releases (Garden+)
    * [Pull request #583](https://github.com/gazebosim/ign-rendering/pull/583)

1. Require geospatial component
    * [Pull request #539](https://github.com/gazebosim/ign-rendering/pull/539)

1. Respect vertex shader from low level materials in sensor passes (#544)
    * [Pull request #578](https://github.com/gazebosim/ign-rendering/pull/578)

1. Expose GraphicsAPI
    * [Pull request #565](https://github.com/gazebosim/ign-rendering/pull/565)

1. Fixed segfault caused by OgreRTShaderSystem
    * [Pull request #563](https://github.com/gazebosim/ign-rendering/pull/563)

1. Change to use SuppressWarning from ign-utils
    * [Pull request #559](https://github.com/gazebosim/ign-rendering/pull/559)

1. [Metal] add metal shaders to waves example
    * [Pull request #555](https://github.com/gazebosim/ign-rendering/pull/555)

1. [Metal] update custom_shaders_uniforms example
    * [Pull request #554](https://github.com/gazebosim/ign-rendering/pull/554)

1. Implement Hlms custom pieces swap for Objects & Terrain
    * [Pull request #545](https://github.com/gazebosim/ign-rendering/pull/545)

1. [Metal] Fix per-point colors in point clouds to match #494
    * [Pull request #550](https://github.com/gazebosim/ign-rendering/pull/550)

1. [Metal] Fix sky background in RGBD camera to match #515
    * [Pull request #549](https://github.com/gazebosim/ign-rendering/pull/#549)

1. Update migration guide
    * [Pull request #538](https://github.com/gazebosim/ign-rendering/pull/538)

1. Rename Ogre2IgnHlmsCustomizations to Ogre2IgnHlmsSphericalClipMinDistance
    * [Pull request #534](https://github.com/gazebosim/ign-rendering/pull/534)

1. Fix color of last / extra marker points
    * [Pull request #519](https://github.com/gazebosim/ign-rendering/pull/519)

1. Per-point colors in point clouds
    * [Pull request #494](https://github.com/gazebosim/ign-rendering/pull/494)

1. [ogre1.x] Add support for wide angle camera
    * [Pull request #490](https://github.com/gazebosim/ign-rendering/pull/490)

1. Fix Ogre2 assertions during ray queries
    * [Pull request #415](https://github.com/gazebosim/ign-rendering/pull/415)

1. Use ignition-utils1
    * [Pull request #492](https://github.com/gazebosim/ign-rendering/pull/492)

1. Fix windows warnings
    * [Pull request #489](https://github.com/gazebosim/ign-rendering/pull/489)

1. Enable ray query for depth, segmentation and thermal cameras
    * [Pull request #483](https://github.com/gazebosim/ign-rendering/pull/483)

1. Add a method to render targets and cameras to retrieve a Metal texture
    * [Pull request #478](https://github.com/gazebosim/ign-rendering/pull/478)

1. add 64 bit int support to user data
    * [Pull request #479](https://github.com/gazebosim/ign-rendering/pull/479)

### Gazebo Rendering 6.X

### Gazebo Rendering 6.6.2 (2023-10-03)

1. Backport camera intrinsics calculation : Refactor
    * [Pull request #905](https://github.com/gazebosim/gz-rendering/pull/905)
    * [Pull request #929](https://github.com/gazebosim/gz-rendering/pull/929)

1. Revert mesh viewer background color back to gray
    * [Pull request #894](https://github.com/gazebosim/gz-rendering/pull/894)

### Gazebo Rendering 6.6.1 (2023-09-01)

1. Fixed light visual in OGRE
    * [Pull request #864](https://github.com/gazebosim/gz-rendering/pull/864)

1. Lower severity level for ogre2 visibility mask msgs and unavailable render passes
    * [Pull request #830](https://github.com/gazebosim/gz-rendering/pull/830)

1. Infrastructure
    * [Pull request #834](https://github.com/gazebosim/gz-rendering/pull/834)

1. Rename COPYING to LICENSE
    * [Pull request #833](https://github.com/gazebosim/gz-rendering/pull/833)

1. Add message to see troubleshooting page when render engine fails to create dummy window
    * [Pull request #829](https://github.com/gazebosim/gz-rendering/pull/829)

1. Tweak max camera position limit
    * [Pull request #827](https://github.com/gazebosim/gz-rendering/pull/827)

1. Limit max camera position vector length
    * [Pull request #824](https://github.com/gazebosim/gz-rendering/pull/824)

### Gazebo Rendering 6.6.0 (2023-02-02)

1. Backport Composite BaseVisual destroy fix to 6
    * [Pull request #818](https://github.com/gazebosim/ign-rendering/pull/818)

1. Forward port 3 to 6.
    * [Pull request #815](https://github.com/gazebosim/ign-rendering/pull/815)
    * [Pull request #819](https://github.com/gazebosim/ign-rendering/pull/819)

1. Remove fini to resolve segfault at shutdown
    * [Pull request #813](https://github.com/gazebosim/ign-rendering/pull/813)

1. Fix transparency over heightmap
    * [Pull request #811](https://github.com/gazebosim/ign-rendering/pull/811)

1. Fix duplicate PreRrender calls
    * [Pull request #809](https://github.com/gazebosim/ign-rendering/pull/809)

1. ign -> gz Migrate Ignition Headers : gz-rendering
    * [Pull request #705](https://github.com/gazebosim/ign-rendering/pull/705)

1. Improved coverage: MeshDescriptor, Mesh, MoveToHelper, OrbitViewController, PixelFormat and ShadersParams
    * [Pull request #748](https://github.com/gazebosim/ign-rendering/pull/748)

1. Suppress Windows warnings
    * [Pull request #749](https://github.com/gazebosim/ign-rendering/pull/749)

### Gazebo Rendering 6.5.2 (2023-01-19)

1. Mesh clean up in destructor
    * [Pull request #807](https://github.com/gazebosim/ign-rendering/pull/807)

1. Fix Ogre2DepthCamera on macOS (Fortress)
    * [Pull request #797](https://github.com/gazebosim/ign-rendering/pull/797)

1. Restore GLX context after OGRE plugin loading finishes.
    * [Pull request #794](https://github.com/gazebosim/ign-rendering/pull/794)

1. Add message to see troubleshooting page when render engine fails to initialize
    * [Pull request #793](https://github.com/gazebosim/ign-rendering/pull/793)

1. Do not overwrite reserved visibility flags in ogre2 (Fortress)
    * [Pull request #783](https://github.com/gazebosim/ign-rendering/pull/783)

1. Fix Metal shader compiler error in GpuRays (Lidar)
    * [Pull request #786](https://github.com/gazebosim/ign-rendering/pull/786)

1. Fix memory corruption in Ogre2GpuRays
    * [Pull request #787](https://github.com/gazebosim/ign-rendering/pull/787)

1. Fix RenderTarget::destroy not getting called in both ogre1 & ogre2
    * [Pull request #776](https://github.com/gazebosim/ign-rendering/pull/776)

1. Fix wrong error log message when creating scene
    * [Pull request #777](https://github.com/gazebosim/ign-rendering/pull/777)

1. Use proper sRGB conversion in DepthCamera
    * [Pull request #766](https://github.com/gazebosim/ign-rendering/pull/766)

1. Fix non-deterministic rendering due to background texture loading
    * [Pull request #765](https://github.com/gazebosim/ign-rendering/pull/765)

1. Fix lidar heightmap detection
    * [Pull request #760](https://github.com/gazebosim/ign-rendering/pull/760)

1. Improved RenderEngine and Mesh coverage
    * [Pull request #739](https://github.com/gazebosim/ign-rendering/pull/739)

1. Increased Marker coverage
    * [Pull request #738](https://github.com/gazebosim/ign-rendering/pull/738)

1. Added shaderType coverage
    * [Pull request #740](https://github.com/gazebosim/ign-rendering/pull/740)

1. Improved grid coverage
    * [Pull request #737](https://github.com/gazebosim/ign-rendering/pull/737)

1. Improved coverage Arrow and Axis visuals and fixed some warnings
    * [Pull request #736](https://github.com/gazebosim/ign-rendering/pull/736)

1. Remove redundant namespace references
    * [Pull request #701](https://github.com/gazebosim/ign-rendering/pull/701)

1. Change IGN_DESIGNATION to GZ_DESIGNATION
    * [Pull request #677](https://github.com/gazebosim/ign-rendering/pull/677)

1. Add coverage ignore file
    * [Pull request #689](https://github.com/gazebosim/ign-rendering/pull/689)

### Gazebo Rendering 6.5.1 (2022-07-28)

1. Backport multiple ray queries fix (#665)
    * [Pull request #690](https://github.com/gazebosim/gz-rendering/pull/690)

1. Fixed CMakeLists.txt in examples
    * [Pull request #686](https://github.com/gazebosim/gz-rendering/pull/686)

1. Update Marker test
    * [Pull request #673](https://github.com/gazebosim/gz-rendering/pull/673)

1. Increase TransformControl test coverage
    * [Pull request #668](https://github.com/gazebosim/gz-rendering/pull/668)

1. Enable ogre2 heightmap test
    * [Pull request #670](https://github.com/gazebosim/gz-rendering/pull/670)

1. Add tests for ShaderType, GraphicsAPI, and DistortionPass
    * [Pull request #671](https://github.com/gazebosim/gz-rendering/pull/671)

1. Fixed waves example CMakeLists.txt
    * [Pull request #678](https://github.com/gazebosim/gz-rendering/pull/678)

1. Ignition -> Gazebo
    * [Pull request #674](https://github.com/gazebosim/gz-rendering/pull/674)

1. Disable thermal camera tests on MacOS
    * [Pull request #659](https://github.com/gazebosim/gz-rendering/pull/659)

1. Fix Ogre2RenderTarget::TargetFSAA method that caused black screen when used with llvmpipe
    * [Pull request #661](https://github.com/gazebosim/gz-rendering/pull/661)

1. Protect against non-finite inputs on Node and OrbitViewController
    * [Pull request #655](https://github.com/gazebosim/gz-rendering/pull/655)

1. Fix checking unofficial ogre version when ogre 1.x is not installed
    * [Pull request #648](https://github.com/gazebosim/gz-rendering/pull/648)


### Gazebo Rendering 6.5.0 (2022-06-15)

1. Bounding Box Camera
    * [Pull request #334](https://github.com/gazebosim/gz-rendering/pull/334)

1. Make GLX dependencies optional for Ogre2.
    * [Pull request #646](https://github.com/gazebosim/gz-rendering/pull/646)

1. Add missing utility include for `std::exchange`
    * [Pull request #650](https://github.com/gazebosim/gz-rendering/pull/650)

1. Change `aspect` to `AspectRatio()`
    * [Pull request #635](https://github.com/gazebosim/gz-rendering/pull/635)

1. `Camera::HFOV` override for Ogre and Ogre2
    * [Pull request #636](https://github.com/gazebosim/gz-rendering/pull/636)

### Gazebo Rendering 6.4.0 (2022-05-12)

1. Add support for GpuRays (lidar) visibility mask
    * [Pull request #625](https://github.com/gazebosim/gz-rendering/pull/625)

1. Use gz-utils test macros
    * [Pull request #624](https://github.com/gazebosim/gz-rendering/pull/624)

1. Cleanup depth camera resources on destroy
    * [Pull request #623](https://github.com/gazebosim/gz-rendering/pull/623)

1. Fixed Readme.md typo
    * [Pull request #620](https://github.com/gazebosim/gz-rendering/pull/620)

1. Destroy root node and clean up sensor resources on exit
    * [Pull request #617](https://github.com/gazebosim/gz-rendering/pull/617)

1. Add repo specific issue templates
    * [Pull request #616](https://github.com/gazebosim/gz-rendering/pull/616)

### Gazebo Rendering 6.3.1 (2022-04-13)

1.  Revert OGRE-Next custom support and back to use GzOGRE2 module
    * [Pull request #605](https://github.com/gazebosim/gz-rendering/pull/605)

### Gazebo Rendering 6.3.0 (2022-04-06)

1. Flip heightmap's Y position on Ogre 2 and add sanity checks for NaN
    * [Pull request #585](https://github.com/gazebosim/gz-rendering/pull/585)

1. Use custom simulation time variants for Ogre
    * [Pull request #584](https://github.com/gazebosim/gz-rendering/pull/584)

1. Add Ubuntu Jammy CI
    * [Pull request #577](https://github.com/gazebosim/gz-rendering/pull/577)

1. Fix shaders for other sensors that require material switching (ogre2)
    * [Pull request #579](https://github.com/gazebosim/gz-rendering/pull/579)

1. fix render pass demo
    * [Pull request #576](https://github.com/gazebosim/gz-rendering/pull/576)

1. Fix objects using shaders when there is a lidar in the scene (ogre2)
    * [Pull request #575](https://github.com/gazebosim/gz-rendering/pull/575)

1. Do not set shader constants for textures on non-OpenGL graphics API
    * [Pull request #574](https://github.com/gazebosim/gz-rendering/pull/574)

1. Use pose multiplication instead of addition
    * [Pull request #573](https://github.com/gazebosim/gz-rendering/pull/573)

1. Fix sky background compositor setup
    * [Pull request #571](https://github.com/gazebosim/gz-rendering/pull/571)

1. Backport OgreRTShaderSystem segfault fix
    * [Pull request #570](https://github.com/gazebosim/gz-rendering/pull/570)

1. Remove unnecessary error message when headless rendering is enabled
    * [Pull request #569](https://github.com/gazebosim/gz-rendering/pull/569)

### Gazebo Rendering 6.2.1 (2022-02-24)

1. Restore format conversion in Ogre2RenderTarget::Copy
    * [Pull request #564](https://github.com/gazebosim/gz-rendering/pull/564)

1. Increase particle emitter quota
    * [Pull request #562](https://github.com/gazebosim/gz-rendering/pull/562)

1. Make sure shader param exists before setting its value
    * [Pull request #558](https://github.com/gazebosim/gz-rendering/pull/558)

1. Backport wave changes
    * [Pull request #557](https://github.com/gazebosim/gz-rendering/pull/557)

### Gazebo Rendering 6.2.0 (2022-01-28)

1. Fix missing terrain shadows casted on objects
    * [Pull request #547](https://github.com/gazebosim/gz-rendering/pull/547)

1. Add waves
    * [Pull request #541](https://github.com/gazebosim/gz-rendering/pull/541)

1. Fix crash when hovering the cursor in heightmap.sdf
    * [Pull request #536](https://github.com/gazebosim/gz-rendering/pull/536)

1. Fix material switching for objects using shaders (ogre2)
    * [Pull request #533](https://github.com/gazebosim/gz-rendering/pull/533)

1. [Metal] Fix depth camera
    * [Pull request #535](https://github.com/gazebosim/gz-rendering/pull/535)
    * A contribution from Rhys Mainwaring <rhys.mainwaring@me.com>

1. Bind shader uniforms to constants from ogre
    * [Pull request #531](https://github.com/gazebosim/gz-rendering/pull/531)

1. Silence CMake policy CMP0072
    * [Pull request #528](https://github.com/gazebosim/gz-rendering/pull/528)

1. Fix various issues with Ogre2GpuRays
    * [Pull request #522](https://github.com/gazebosim/gz-rendering/pull/522)
    * [Pull request #527](https://github.com/gazebosim/gz-rendering/pull/527)

1. Fix Ogre2ThermalCamera using garbage depth data
    * [Pull request #523](https://github.com/gazebosim/gz-rendering/pull/523)

1. Performance optimization by avoiding unnecessary passes
    * [Pull request #524](https://github.com/gazebosim/gz-rendering/pull/524)

1. Support using custom shader materials and updating uniform variables (ogre2)
    * [Pull request #520](https://github.com/gazebosim/gz-rendering/pull/520)

1. Port Camera Distortion effect from gazebo11
    * [Pull request #502](https://github.com/gazebosim/gz-rendering/pull/502)

1. Fix sky background in RGBD camera
    * [Pull request #515](https://github.com/gazebosim/gz-rendering/pull/515)

1. RenderOrder is in different scale in Reverse Z
    * [Pull request #514](https://github.com/gazebosim/gz-rendering/pull/514)

### Gazebo Rendering 6.1.0 (2021-12-09)

1. Check for OpenGL backend before calling GL APIs
    * [Pull request #509](https://github.com/gazebosim/gz-rendering/pull/509)

1. Fix loading grayscale emissive map
    * [Pull request #503](https://github.com/gazebosim/gz-rendering/pull/503)
    * [Pull request #501](https://github.com/gazebosim/gz-rendering/pull/501)

1. Fixes to selection buffer
    * [Pull request #446](https://github.com/gazebosim/gz-rendering/pull/446)
    * [Pull request #510](https://github.com/gazebosim/gz-rendering/pull/510)
    * [Pull request #498](https://github.com/gazebosim/gz-rendering/pull/498)

1. Add Metal support for the Ogre2 Render Engine
    * [Pull request #463](https://github.com/gazebosim/gz-rendering/pull/463)

1. Add a depth camera example
    * [Pull request #467](https://github.com/gazebosim/gz-rendering/pull/467)

1. More nullptr checking on visuals and base axis
    * [Pull request #486](https://github.com/gazebosim/gz-rendering/pull/486)

1. Fix Utils unit test
    * [Pull request #481](https://github.com/gazebosim/gz-rendering/pull/481)

1. Fix context attributes of glXCreateContextAttribsARB.
    * [Pull request #460](https://github.com/gazebosim/gz-rendering/pull/460)

1. [Ogre2] fix invalid anti-aliasing level warning
    * [Pull request #470](https://github.com/gazebosim/gz-rendering/pull/470)

1. Avoid symbol redefinition to fix armel builds
    * [Pull request #457](https://github.com/gazebosim/gz-rendering/pull/457)

1. fix grayscale albedo map
    * [Pull request #466](https://github.com/gazebosim/gz-rendering/pull/466)

1. Fix selection buffer material script
    * [Pull request #456](https://github.com/gazebosim/gz-rendering/pull/456)

1. [macOS] modify definition of bufferFetch1 to work when GL_ARB_texture_buffer_range is not available
    * [Pull request #462](https://github.com/gazebosim/gz-rendering/pull/462)

1. Fix compilation against Ogre 1.10.12
    * [Pull request #390](https://github.com/gazebosim/gz-rendering/pull/390)

1. Fix logic on warning for ogre versions different than 1.9.x
    * [Pull request #465](https://github.com/gazebosim/gz-rendering/pull/465)

1. Fix point cloud material syntax error
    * [Pull request #433](https://github.com/gazebosim/gz-rendering/pull/433)

1. Run ogre2 particles by default
    * [Pull request #430](https://github.com/gazebosim/gz-rendering/pull/430)

1. Ogre: Do not assume that ogre plugins have lib prefix on macOS
    * [Pull request #454](https://github.com/gazebosim/gz-rendering/pull/454)

1. Fix heightmap crash if only shadow casting spotlights are one scene
    * [Pull request #451](https://github.com/gazebosim/gz-rendering/pull/451)

1. Added macOS instructions
    * [Pull request #448](https://github.com/gazebosim/gz-rendering/pull/448)

1. Ogre: Add missing required Paging component
    * [Pull request #452](https://github.com/gazebosim/gz-rendering/pull/452)

1. Fix ray query distance calculation
    * [Pull request #438](https://github.com/gazebosim/gz-rendering/pull/438)

### Gazebo Rendering 6.0.1 (2021-10-01)

1. Disable using selection buffer for ray queries
    * [Pull request #447](https://github.com/gazebosim/gz-rendering/pull/447)

### Gazebo Rendering 6.0.0 (2021-09-30)

1. Avoid configure warning when checking ogre-1.10
    * [Pull request #411](https://github.com/gazebosim/gz-rendering/pull/411)
    * [Pull request #413](https://github.com/gazebosim/gz-rendering/pull/413)

1. Use selection buffer in ray queries (ogre2)
    * [Pull request #378](https://github.com/gazebosim/gz-rendering/pull/378)

1. Fix particle effect randomness
    * [Pull request #388](https://github.com/gazebosim/gz-rendering/pull/388)

1. Update test config to run ogre 1.x tests in gz-rendering6 on macOS
    * [Pull request #407](https://github.com/gazebosim/gz-rendering/pull/407)
    * [Pull request #409](https://github.com/gazebosim/gz-rendering/pull/409)
    * [Pull request #409](https://github.com/gazebosim/gz-rendering/pull/409)

1. Clone visuals and geometries
    * [Pull request #397](https://github.com/gazebosim/gz-rendering/pull/397)
    * [Pull request #434](https://github.com/gazebosim/gz-rendering/pull/434)

1. Add SetSize API for LidarVisual and markers
    * [Pull request #392](https://github.com/gazebosim/gz-rendering/pull/392)

1. Add Ogre2Heightmap functionality
    * [Pull request #386](https://github.com/gazebosim/gz-rendering/pull/386)
    * [Pull request #416](https://github.com/gazebosim/gz-rendering/pull/416)

1. Prevent default-constructed variants from holding a type
    * [Pull request #371](https://github.com/gazebosim/gz-rendering/pull/371)
    * [Pull request #396](https://github.com/gazebosim/gz-rendering/pull/396)

1. Temporarily set number of camera pass count per flush to 0 in ogre2 to prevent downstream build failures
    * [Pull request #367](https://github.com/gazebosim/gz-rendering/pull/367)

1. New visuals
    1. Joint visual
        * [Pull request #366](https://github.com/gazebosim/gz-rendering/pull/366)
        * [Pull request #387](https://github.com/gazebosim/gz-rendering/pull/387)

    1. Center of mass visual
        * [Pull request #345](https://github.com/gazebosim/gz-rendering/pull/345)

    1. Inertia visual
        * [Pull request #326](https://github.com/gazebosim/gz-rendering/pull/326)

1. UserData methods moved from Visual to Node
    * [Pull request #358](https://github.com/gazebosim/gz-rendering/pull/358)

1. Replace renderOneFrame for per-workspace update calls
    * [Pull request #353](https://github.com/gazebosim/gz-rendering/pull/353)

1. Segmentation Camera
    * [Pull request #329](https://github.com/gazebosim/gz-rendering/pull/329)
    * [Pull request #419](https://github.com/gazebosim/gz-rendering/pull/419)
    * [Pull request #443](https://github.com/gazebosim/gz-rendering/pull/443)

1. Stub bounding box camera APIs
    * [Pull request #420](https://github.com/gazebosim/gz-rendering/pull/420)

1. Changed calculation for range clipping
    * [Pull request #325](https://github.com/gazebosim/gz-rendering/pull/325)

1. Destroy material when a mesh is deleted
    * [Pull request #324](https://github.com/gazebosim/gz-rendering/pull/324)

1. Add orthographic view controller
    * [Pull request #322](https://github.com/gazebosim/gz-rendering/pull/322)
    * [Pull request #361](https://github.com/gazebosim/gz-rendering/pull/361)

1. Visualize wireframes
    * [Pull request #314](https://github.com/gazebosim/gz-rendering/pull/314)

1. Add environment map tutorial
    * [Pull request #295](https://github.com/gazebosim/gz-rendering/pull/295)

1. From Ogre 2.1 to Ogre 2.2
    * [Pull request #272](https://github.com/gazebosim/gz-rendering/pull/272)
    * [Pull request #393](https://github.com/gazebosim/gz-rendering/pull/393)
    * [Pull request #436](https://github.com/gazebosim/gz-rendering/pull/436)
    * [Pull request #426](https://github.com/gazebosim/gz-rendering/pull/426)
    * [Pull request #440](https://github.com/gazebosim/gz-rendering/pull/440)

1. Documentation updates
    * [Pull request #425](https://github.com/gazebosim/gz-rendering/pull/425)
    * [Pull request #431](https://github.com/gazebosim/gz-rendering/pull/431)

1. Infrastructure
    * [Pull request #318](https://github.com/gazebosim/gz-rendering/pull/318)

### Gazebo Rendering 5.2.1 (2022-03-35)

1. Silence CMake policy CMP0072
    * [Pull request #528](https://github.com/gazebosim/gz-rendering/pull/528)

### Gazebo Rendering 5.2.0 (2021-11-02)

1. Fix context attributes of glXCreateContextAttribsARB.
    * [Pull request #460](https://github.com/gazebosim/gz-rendering/pull/460)

1. Fix logic on warning for ogre versions different than 1.9.x
    * [Pull request #465](https://github.com/gazebosim/gz-rendering/pull/465)

1. Avoid symbol redefition to fix armel builds
    * [Pull request #457](https://github.com/gazebosim/gz-rendering/pull/457)

1. Fix selection buffer material script
    * [Pull request #456](https://github.com/gazebosim/gz-rendering/pull/456)

1. Fix compilation against Ogre 1.10.12
    * [Pull request #390](https://github.com/gazebosim/gz-rendering/pull/390)

1. ogre: Do not assume that ogre plugins have lib prefix on macOS
    * [Pull request #454](https://github.com/gazebosim/gz-rendering/pull/454)

1. ogre: Add missing required Paging component
    * [Pull request #452](https://github.com/gazebosim/gz-rendering/pull/452)

1. Fix ray query distance calculation
    * [Pull request #438](https://github.com/gazebosim/gz-rendering/pull/438)

1. Fix the spelling of USE_UNOFFICIAL_OGRE_VERSIONS flag
    * [Pull request #413](https://github.com/gazebosim/gz-rendering/pull/413)

1. Avoid configure warning when checking ogre-1.10
    * [Pull request #411](https://github.com/gazebosim/gz-rendering/pull/411)

1. Use selection buffer in ray queries (ogre2)
    * [Pull request #378](https://github.com/gazebosim/gz-rendering/pull/378)

1. Fix particle effect randomness
    * [Pull request #388](https://github.com/gazebosim/gz-rendering/pull/388)

1. Update simple_demo_qml coding style
    * [Pull request #404](https://github.com/gazebosim/gz-rendering/pull/404)

1. Disabling flaky depth and thermal camera tests on mac
    * [Pull request #405](https://github.com/gazebosim/gz-rendering/pull/405)

1. [macOS] add a QML version of simple_demo
    * [Pull request #373](https://github.com/gazebosim/gz-rendering/pull/373)

1. Fix single ray gpu lidar
    * [Pull request #384](https://github.com/gazebosim/gz-rendering/pull/384)

1. Warn about non-existent submesh
    * [Pull request #391](https://github.com/gazebosim/gz-rendering/pull/391)

1. Improved doxygen
    * [Pull request #398](https://github.com/gazebosim/gz-rendering/pull/398)
    * [Pull request #389](https://github.com/gazebosim/gz-rendering/pull/389)

1. Set OGRE minimal version to 1.8. Warn on versions not supported (gz-rendering3)
    * [Pull request #376](https://github.com/gazebosim/gz-rendering/pull/376)

1. 👩‍🌾 Remove bitbucket-pipelines.yml
    * [Pull request #385](https://github.com/gazebosim/gz-rendering/pull/385)

1. [macOS] support OpenGL < 4.2 in PBS shader
    * [Pull request #377](https://github.com/gazebosim/gz-rendering/pull/377)

1. Moved ScreenToPlane and ScreenToScene from gz-gui to gz-rendering
    * [Pull request #363](https://github.com/gazebosim/gz-rendering/pull/363)
    * [Pull request #368](https://github.com/gazebosim/gz-rendering/pull/368)

1. More nullptr checking on Node classes
    * [Pull request #364](https://github.com/gazebosim/gz-rendering/pull/364)

### Gazebo Rendering 5.1.0 (2021-06-22)

1. add ifdef for apple in integration test
    * [Pull request #349](https://github.com/gazebosim/gz-rendering/pull/349)

1. Update light map tutorial
    * [Pull request #346](https://github.com/gazebosim/gz-rendering/pull/346)

1. relax gaussian test tolerance
    * [Pull request #344](https://github.com/gazebosim/gz-rendering/pull/344)

1. Fix custom shaders uniforms gz version number
    * [Pull request #343](https://github.com/gazebosim/gz-rendering/pull/343)

1. recreate node only when needed
    * [Pull request #342](https://github.com/gazebosim/gz-rendering/pull/342)

1. Backport memory fixes found by ASAN
    * [Pull request #340](https://github.com/gazebosim/gz-rendering/pull/340)

1. Fix FSAA in UI and lower VRAM consumption
    * [Pull request #313](https://github.com/gazebosim/gz-rendering/pull/313)

1. Fix depth alpha
    * [Pull request #316](https://github.com/gazebosim/gz-rendering/pull/316)

1. Fix floating point precision bug handling alpha channel (#332)
    * [Pull request #333](https://github.com/gazebosim/gz-rendering/pull/333)

1. Fix heap overflow when reading
    * [Pull request #337](https://github.com/gazebosim/gz-rendering/pull/337)

1. Fix new [] / delete mismatch
    * [Pull request #338](https://github.com/gazebosim/gz-rendering/pull/338)

1. Test re-enabling depth camera integration test on mac
    * [Pull request #335](https://github.com/gazebosim/gz-rendering/pull/335)

1. Include MoveTo Helper class to gz-rendering
    * [Pull request #311](https://github.com/gazebosim/gz-rendering/pull/311)

1. Remove `tools/code_check` and update codecov
    * [Pull request #321](https://github.com/gazebosim/gz-rendering/pull/321)

1. [OGRE 1.x] Uniform buffer shader support
    * [Pull request #294](https://github.com/gazebosim/gz-rendering/pull/294)

1. Helper function to get a scene
    * [Pull request #320](https://github.com/gazebosim/gz-rendering/pull/320)

1. fix capsule mouse picking
    * [Pull request #319](https://github.com/gazebosim/gz-rendering/pull/319)

1. Fix depth alpha
    * [Pull request #316](https://github.com/gazebosim/gz-rendering/pull/316)

1. Add shadows to Ogre2DepthCamera without crashing
    * [Pull request #303](https://github.com/gazebosim/gz-rendering/pull/303)

1. Reduce lidar data discretization
    * [Pull request #296](https://github.com/gazebosim/gz-rendering/pull/296)

1. update light visual size
    * [Pull request #306](https://github.com/gazebosim/gz-rendering/pull/306)

1. Improve build times by reducing included headers
    * [Pull request #299](https://github.com/gazebosim/gz-rendering/pull/299)

1. Add light map tutorial
    * [Pull request #302](https://github.com/gazebosim/gz-rendering/pull/302)

1. Prevent console warnings when multiple texture coordinates are present
    * [Pull request #301](https://github.com/gazebosim/gz-rendering/pull/301)

1. Fix gazebo scene viewer build
    * [Pull request #289](https://github.com/gazebosim/gz-rendering/pull/289)

1. Silence noisy sky error
    * [Pull request #282](https://github.com/gazebosim/gz-rendering/pull/282)

1. Added command line argument to pick version of Ogre
    * [Pull request #277](https://github.com/gazebosim/gz-rendering/pull/277)

### Gazebo Rendering 5.0.0 (2021-03-30)

1. Add ogre2 skybox support
    * [Pull request #168](https://github.com/gazebosim/gz-rendering/pull/168)

1. Add light visual support
    * [Pull request #202](https://github.com/gazebosim/gz-rendering/pull/202)
    * [Pull request #228](https://github.com/gazebosim/gz-rendering/pull/228)

1. Add API for particle emitter scatter ratio
    * [Pull request #275](https://github.com/gazebosim/gz-rendering/pull/275)

1. Added capsule geometry
    * [Pull request #200](https://github.com/gazebosim/gz-rendering/pull/200)
    * [Pull request #278](https://github.com/gazebosim/gz-rendering/pull/278)

1. Added ellipsoid Geometry
    * [Pull request #203](https://github.com/gazebosim/gz-rendering/pull/203)

1. ogre2: explicitly request OpenGL 3.3 core profile context.
    * [Pull request #244](https://github.com/gazebosim/gz-rendering/pull/244)

1. Bump in edifice: gz-common4
    * [Pull request #209](https://github.com/gazebosim/gz-rendering/pull/209)

1. Support setting horizontal and vertical resolution for GpuRays
    * [Pull request #229](https://github.com/gazebosim/gz-rendering/pull/229)

1. Added Light Intensity
    * [Pull request #233](https://github.com/gazebosim/gz-rendering/pull/233)
    * [Pull request #284](https://github.com/gazebosim/gz-rendering/pull/284)

1. Heightmap for Ogre 1
    * [Pull request #180](https://github.com/gazebosim/gz-rendering/pull/180)

1. Added render order to material
    * [Pull request #188](https://github.com/gazebosim/gz-rendering/pull/188)

1. Add support for lightmaps in ogre2
    * [Pull request #182](https://github.com/gazebosim/gz-rendering/pull/182)

1. Documentation updates
    * [Pull request #288](https://github.com/gazebosim/gz-rendering/pull/288)
    * [Pull request #287](https://github.com/gazebosim/gz-rendering/pull/287)
    * [Pull request #286](https://github.com/gazebosim/gz-rendering/pull/286)

### Gazebo Rendering 4.X

### Gazebo Rendering 4.9.0 (2021-09-15)

1. Output warning message if a mesh with zero submeshes is created
    * [Pull request #391](https://github.com/gazebosim/gz-rendering/pull/391)

1. Fix particle effect randomness
    * [Pull request #388](https://github.com/gazebosim/gz-rendering/pull/388)

1. Fix single ray gpu lidar
    * [Pull request #384](https://github.com/gazebosim/gz-rendering/pull/384)

1. Use selection buffer in ray queries (ogre2)
    * [Pull request #378](https://github.com/gazebosim/gz-rendering/pull/378)
    * [Pull request #383](https://github.com/gazebosim/gz-rendering/pull/383)

1. All changes merged forward from gz-rendering3
    * [Pull request #382](https://github.com/gazebosim/gz-rendering/pull/382)
    * [Pull request #398](https://github.com/gazebosim/gz-rendering/pull/398)

### Gazebo Rendering 4.8.0 (2021-06-18)

1. relax gaussian test tolerance
    * [Pull request #344](https://github.com/gazebosim/gz-rendering/pull/344)

1. recreate node only when needed
    * [Pull request #342](https://github.com/gazebosim/gz-rendering/pull/342)

1. Backport memory fixes found by ASAN
    * [Pull request #340](https://github.com/gazebosim/gz-rendering/pull/340)

1. Re-enable part of depth camera test on macOS
    * [Pull request #335](https://github.com/gazebosim/gz-rendering/pull/335)
    * [Pull request #347](https://github.com/gazebosim/gz-rendering/pull/347)

1. Fix depth alpha
    * [Pull request #316](https://github.com/gazebosim/gz-rendering/pull/316)

1. Fix floating point precision bug handling alpha channel (#332)
    * [Pull request #333](https://github.com/gazebosim/gz-rendering/pull/333)

1. Include MoveTo Helper class to gz-rendering
    * [Pull request #311](https://github.com/gazebosim/gz-rendering/pull/311)

1. Remove `tools/code_check` and update codecov
    * [Pull request #321](https://github.com/gazebosim/gz-rendering/pull/321)

1. [OGRE 1.x] Uniform buffer shader support
    * [Pull request #294](https://github.com/gazebosim/gz-rendering/pull/294)

1. Helper function to get a scene
    * [Pull request #320](https://github.com/gazebosim/gz-rendering/pull/320)

1. Reduce lidar data discretization
    * [Pull request #296](https://github.com/gazebosim/gz-rendering/pull/296)

1. Prevent console warnings when multiple texture coordinates are present
    * [Pull request #301](https://github.com/gazebosim/gz-rendering/pull/301)

1. Added command line argument to pick version of Ogre
    * [Pull request #277](https://github.com/gazebosim/gz-rendering/pull/277)

### Gazebo Rendering 4.7.0 (2021-03-17)

1. Enable depth write in particles example
    * [Pull Request #217](https://github.com/gazebosim/gz-rendering/pull/217)

1. Fix gazebo_scene_viewer for macOS and ensure clean exit
    * [Pull Request #259](https://github.com/gazebosim/gz-rendering/pull/259)

1. Master branch updates
    * [Pull Request #268](https://github.com/gazebosim/gz-rendering/pull/268)

1. Expose particle scatter ratio parameter
    * [Pull Request #269](https://github.com/gazebosim/gz-rendering/pull/269)

1. Fix overriding blend mode
    * [Pull Request #266](https://github.com/gazebosim/gz-rendering/pull/266)

1. Fix DepthGaussianNoise test
    * [Pull Request #271](https://github.com/gazebosim/gz-rendering/pull/271)

1. Handle non-positive object temperatures
    * [Pull Request #243](https://github.com/gazebosim/gz-rendering/pull/243)

### Gazebo Rendering 4.6.0 (2021-03-01)

1. Improve particle scattering noise
    * [Pull Request #261](https://github.com/gazebosim/gz-rendering/pull/261)

1. Fix custom_scene_viewer for macOS
    * [Pull Request #256](https://github.com/gazebosim/gz-rendering/pull/256)

### Gazebo Rendering 4.5.0 (2021-02-17)

1. More verbose messages when failing to load render engines
    * [Pull Request #236](https://github.com/gazebosim/gz-rendering/pull/236)

1. Fixed OBJ textures with the same name
    * [Pull Request #239](https://github.com/gazebosim/gz-rendering/pull/239)

1. Fix setting particle size
    * [Pull Request #241](https://github.com/gazebosim/gz-rendering/pull/241)

1. Make particle emitter invisible in thermal camera image
    * [Pull Request #240](https://github.com/gazebosim/gz-rendering/pull/240)

1. Apply particle scattering effect to depth cameras
    * [Pull Request #251](https://github.com/gazebosim/gz-rendering/pull/251)

### Gazebo Rendering 4.4.0 (2021-02-11)

1. Add support for 8 bit thermal camera image format
    * [Pull Request #235](https://github.com/gazebosim/gz-rendering/pull/235)

1. All changes up to version 3.4.0

### Gazebo Rendering 4.3.1 (2021-02-03)

1. Fix converting Pbs to Unlit material conversion (#230)
    * [Pull Request #230](https://github.com/gazebosim/gz-rendering/pull/230)

### Gazebo Rendering 4.3.0 (2021-02-02)

1. Tutorial: Transform FBX to Collada and change its origin
    * [Pull request #195](https://github.com/gazebosim/gz-rendering/pull/195)

1. Remove Windows Warnings
    * [Pull request #206](https://github.com/gazebosim/gz-rendering/pull/206)
    * [Pull request #213](https://github.com/gazebosim/gz-rendering/pull/213)
    * [Pull request #215](https://github.com/gazebosim/gz-rendering/pull/215)

1. Fix spelling Desconstructor -> Destructor
    * [Pull request #220](https://github.com/gazebosim/gz-rendering/pull/220)

1. Thermal Camera: Visualize objects with a heat signature
    * [Pull request #189](https://github.com/gazebosim/gz-rendering/pull/189)

1. Add laser retro support in Ogre2
    * [Pull request #194](https://github.com/gazebosim/gz-rendering/pull/194)

### Gazebo Rendering 4.2.0 (2021-01-22)

1. Remove some windows warnings.
    * [Pull request #183](https://github.com/gazebosim/gz-rendering/pull/183)

1. Fix transparency issue for textures without alpha channel.
    * [Pull request #186](https://github.com/gazebosim/gz-rendering/pull/186)

1. Call XCloseDisplay in screenScalingFactor.
    * [Pull request #204](https://github.com/gazebosim/gz-rendering/pull/204)

1. All changes up to version 3.3.0.

### Gazebo Rendering 4.1.0 (2020-11-04)

1. Improve fork experience
    * [Pull request #165](https://github.com/gazebosim/gz-rendering/pull/165)

1. Add Custom Render Engine support
    * [Pull request 161](https://github.com/gazebosim/gz-rendering/pull/161)
    * [Pull request 154](https://github.com/gazebosim/gz-rendering/pull/154)
    * [Pull request 142](https://github.com/gazebosim/gz-rendering/pull/142)
    * [Pull request 141](https://github.com/gazebosim/gz-rendering/pull/141)

1. Update tutorials
    * [Pull request #159](https://github.com/gazebosim/gz-rendering/pull/159)
    * [Pull request #153](https://github.com/gazebosim/gz-rendering/pull/153)

1. Limit number of shadow casting lights in ogre2
    * [Pull Request 155](https://github.com/gazebosim/gz-rendering/pull/155)

1. Ogre2 depth camera fix
    * [Pull Request 138](https://github.com/gazebosim/gz-rendering/pull/138)

1. Add support for Gaussian noise render pass in Ogre2DepthCamera
    * [Pull Request 122](https://github.com/gazebosim/gz-rendering/pull/122)

### Gazebo Rendering 4.0.0 (2020-09-29)

1. Update tutorials
    * [Pull request #151](https://github.com/gazebosim/gz-rendering/pull/151)
    * [Pull request #150](https://github.com/gazebosim/gz-rendering/pull/150)
    * [Pull request #149](https://github.com/gazebosim/gz-rendering/pull/149)
    * [Pull request #148](https://github.com/gazebosim/gz-rendering/pull/148)
    * [Pull request #147](https://github.com/gazebosim/gz-rendering/pull/147)
    * [Pull request #146](https://github.com/gazebosim/gz-rendering/pull/146)
    * [Pull request #145](https://github.com/gazebosim/gz-rendering/pull/145)
    * [Pull request #144](https://github.com/gazebosim/gz-rendering/pull/144)
    * [Pull request #143](https://github.com/gazebosim/gz-rendering/pull/143)
    * [Pull request #125](https://github.com/gazebosim/gz-rendering/pull/125)

1. Pimpl'ize ogre2 classes
    * [Pull request #137](https://github.com/gazebosim/gz-rendering/pull/137)

1. Replace common::Time with std::chrono
    * [Pull request #135](https://github.com/gazebosim/gz-rendering/pull/135)

1. Add Ogre2 particle system
    * [Pull request #126](https://github.com/gazebosim/gz-rendering/pull/126)
    * [Pull request #117](https://github.com/gazebosim/gz-rendering/pull/117)
    * [Pull request #113](https://github.com/gazebosim/gz-rendering/pull/113)
    * [Pull request #107](https://github.com/gazebosim/gz-rendering/pull/107)

1. Add LidarVisual point colors for Ogre1
    * [Pull request #124](https://github.com/gazebosim/gz-rendering/pull/124)

1. Added Lidar Visual for Ogre2
    * [Pull request #116](https://github.com/gazebosim/gz-rendering/pull/116)

1. Added Lidar Visual Types for Ogre1
    * [Pull request #114](https://github.com/gazebosim/gz-rendering/pull/114)

1. Added Lidar Visualisation for Ogre1
    * [Pull request #103](https://github.com/gazebosim/gz-rendering/pull/103)

1. Add environment variables to locate plugins
    * [Pull request #104](https://github.com/gazebosim/gz-rendering/pull/104)

1. Added a method to show the head on arrows and axis
    * [Pull request #95](https://github.com/gazebosim/gz-rendering/pull/95)

1. Complete Shaft and Head methods in BaseArrowVisual
    * [Pull request #95](https://github.com/gazebosim/gz-rendering/pull/94)

1. Scale BaseAxis properly
    * [Pull request #88](https://github.com/gazebosim/gz-rendering/pull/88)

1. Add ogre2 AxisVisual and ArrowVisual
    * [Pull request 87](https://github.com/gazebosim/gz-rendering/pull/87)

1. Support setting skeleton node weights
    * [BitBucket pull request 256](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/256)

1. Add support for setting visibility mask and flags
    * [BitBucket pull request 252](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/252)

1. Increase precision of Ogre2 mesh vertices from half to full precision float
    * [BitBucket pull request 254](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/254)

1. Update transparency based on diffuse alpha channel for ogre2
    * [BitBucket pull request 243](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/243)

1. Skeleton animation updates
    * [BitBucket pull request 235](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/235)

1. Add support for transparency based on textures alpha channel for ogre1 and ogre2
    * [BitBucket pull request 229](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/229)

### Gazebo Rendering 3.7.2 (2024-01-05)

1. Update github action workflows
    * [Pull request #940](https://github.com/gazebosim/gz-rendering/pull/940)
    * [Pull request #834](https://github.com/gazebosim/gz-rendering/pull/834)
    * [Pull request #833](https://github.com/gazebosim/gz-rendering/pull/833)

1. Add minor comments to BaseGizmoVisual
    * [Pull request #881](https://github.com/gazebosim/gz-rendering/pull/881)

### Gazebo Rendering 3.7.1 (2023-02-03)

1. Remove fini to resolve segfault at shutdown.
    * [Pull request #813](https://github.com/gazebosim/gz-rendering/pull/813)

### Gazebo Rendering 3.6.0 (2021-10-15)

1. Avoid symbol redefition to fix armel builds
    * [Pull request 457](https://github.com/gazebosim/gz-rendering/pull/457)

1. Fix floating point precision bug handling alpha channel
    * [Pull request 322](https://github.com/gazebosim/gz-rendering/pull/322)
    * [Pull request 333](https://github.com/gazebosim/gz-rendering/pull/333)

1. Ogre: Do not assume that ogre plugins have lib prefix on macOS
    * [Pull request 454](https://github.com/gazebosim/gz-rendering/pull/454)

1. Moved ScreenToPlane and ScreenToScene from gz-gui to gz-rendering
    * [Pull request 363](https://github.com/gazebosim/gz-rendering/pull/363)
    * [Pull request 368](https://github.com/gazebosim/gz-rendering/pull/368)

1. Relax Gaussian noise test tolerance to fix flaky test
    * [Pull request 344](https://github.com/gazebosim/gz-rendering/pull/344)

1. Fix ogre2 render pass high GPU usage
    * [Pull request 342](https://github.com/gazebosim/gz-rendering/pull/342)

1. Backport memory fixes found by ASAN
    * [Pull request 340](https://github.com/gazebosim/gz-rendering/pull/340)

1. CMake warning on Ogre versions that are not officially supported.
   To disable the new warning, it is enough to enable the cmake option USE_UNOFFICIAL_OGRE_VERSIONS
    * [Pull request 376](https://github.com/gazebosim/gz-rendering/pull/376)

### Gazebo Rendering 3.5.0 (2021-05-25)

1. Include MoveTo Helper class to gz-rendering
    * [Pull request 311](https://github.com/gazebosim/gz-rendering/pull/311)

1. Remove tools/code_check and update codecov
    * [Pull request 321](https://github.com/gazebosim/gz-rendering/pull/321)

1. Helper function to get a scene (#320)
    * [Pull request 320](https://github.com/gazebosim/gz-rendering/pull/320)

1. Fix DepthGaussianNoise test (#271)
    * [Pull request 271](https://github.com/gazebosim/gz-rendering/pull/271)

1. Master branch updates (#268)
    * [Pull request 268](https://github.com/gazebosim/gz-rendering/pull/268)

1. 👩🌾 Make GitHub actions tests that are flaky due to display more verbose information (#255)
    * [Pull request 255](https://github.com/gazebosim/gz-rendering/pull/255)

1. Fixed OBJ textures with the same name (#239)
    * [Pull request 239](https://github.com/gazebosim/gz-rendering/pull/239)

1. More verbose messages when failing to load render engine (#236)
    * [Pull request 236](https://github.com/gazebosim/gz-rendering/pull/236)

### Gazebo Rendering 3.4.0 (2021-02-09)

1. Clear all Windows warnings
    * [Pull request 206](https://github.com/gazebosim/gz-rendering/pull/206)
    * [Pull request 215](https://github.com/gazebosim/gz-rendering/pull/215)
    * [Pull request 225](https://github.com/gazebosim/gz-rendering/pull/225)

1. Add laser_retro support
    * [Pull request 194](https://github.com/gazebosim/gz-rendering/pull/194)

### Gazebo Rendering 3.3.0 (2021-01-22)

1. Add Windows installation.
    * [Pull request 196](https://github.com/gazebosim/gz-rendering/pull/196)

1. Make flaky VisualAt test more verbose.
    * [Pull request 174](https://github.com/gazebosim/gz-rendering/pull/174)

1. Resolve updated codecheck issues.
    * [Pull request 173](https://github.com/gazebosim/gz-rendering/pull/173)

1. Fix crash due to NaN pose values.
    * [Pull request 169](https://github.com/gazebosim/gz-rendering/pull/169)

1. Improve fork experience.
    * [Pull request 165](https://github.com/gazebosim/gz-rendering/pull/165)

### Gazebo Rendering 3.2.0 (2020-10-13)

1. Add Custom Render Engine support
    * [Pull request 161](https://github.com/gazebosim/gz-rendering/pull/161)
    * [Pull request 154](https://github.com/gazebosim/gz-rendering/pull/154)
    * [Pull request 142](https://github.com/gazebosim/gz-rendering/pull/142)
    * [Pull request 141](https://github.com/gazebosim/gz-rendering/pull/141)

1. Add getter for Ogre2Camera
    * [Pull request 97](https://github.com/gazebosim/gz-rendering/pull/97)

1. Update test configuration to use ogre2
    * [Pull request 83](https://github.com/gazebosim/gz-rendering/pull/83)

1. Ogre2 GPU Ray: Cleanup all resources on destruction
    * [BitBucket pull request 258](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/258)

1. Ogre2: Add Grid
    * [BitBucket pull request 246](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/246)

1. Includes all changes up to and including those in version 2.5.1

### Gazebo Rendering 3.1.0 (2020-03-05)

1. Use default material properties from gz-common PBR material when loading a mesh
    * [BitBucket pull request 242](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/242)

1. Fix build for ogre version >= 1.12.0
    * [BitBucket pull request 241](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/241)

1. Add Ogre2 DynamicRenderable class
    * [BitBucket pull request 226](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/226)

1. Skeletons for Ogre2
    * [BitBucket pull request 224](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/224)

1. Fixed gazebo scene viewer example
    * [BitBucket pull request 228](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/228)

1. Ogre2: Fix shadows by dynamically generating shadow maps
    * [BitBucket pull request 222](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/222)

1. Ogre2: Destroy resources before recreating shadows
    * [BitBucket pull request 223](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/223)

1. Fix custom shaders example
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/230)

1. Add API to check which engines are loaded
    * [BitBucket pull request 233](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/233)

### Gazebo Rendering 3.0.0 (2019-12-10)

1. Support setting cast shadows
    * [BitBucket pull request 209](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/209)

1. Add Thermal Camera
    * [BitBucket pull request 202](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/202)

1. Add support for emissive map
    * [BitBucket pull request 207](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/207)

1. Add Marker class and port ogre dynamic lines and renderable
    * [BitBucket pull request 195](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/195)
    * [BitBucket pull request 208](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/208)

1. Support storing user data in Visual
    * [BitBucket pull request 200](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/200)

1. Add 16 bit format
    * [BitBucket pull request 201](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/201)

1. Destroy camera when the scene is destroyed
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/175)

1. Added function to get the number of channels from a GpuRay.
    * [BitBucket pull request 178](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/178)

1. Ogre 1.x skeletal animation changes
    * [BitBucket pull request 184](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/184)
    * [BitBucket pull request 199](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/199)
    * [BitBucket pull request 204](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/204)

### Gazebo Rendering 2.5.1 (2020-10-05)

1. Limit number of shadow casting lights in ogre2
    * [Pull Request 155](https://github.com/gazebosim/gz-rendering/pull/155)

1. Backport ogre2 depth camera fix
    * [Pull Request 138](https://github.com/gazebosim/gz-rendering/pull/138)

### Gazebo Rendering 2.5.0 (2020-08-07)

1. Add support for Gaussian noise render pass in Ogre2DepthCamera
    * [Pull Request 122](https://github.com/gazebosim/gz-rendering/pull/122)

### Gazebo Rendering 2.4.0 (2020-04-17)

1. Fix flaky VisualAt test
    * [BitBucket pull request 248](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/248)

1. Port windows fixes
    * [BitBucket pull request 253](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/253)

1. Backport ogre2 mesh vertices precision change.
    * [BitBucket pull request 255](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/255)

1. Backport transparency setting based on diffuse alpha
    * [BitBucket pull request 247](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/247)

### Gazebo Rendering 2.3.0 (2020-02-19)

1. Add API to check which engines are loaded
    * [BitBucket pull request 233](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/233)

1. Fix custom shaders example
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/230)

1. Ogre2: Fix shadows by dynamically generating shadow maps
    * [BitBucket pull request 222](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/222)
    * [BitBucket pull request 223](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/223)

### Gazebo Rendering 2.2.0 (2019-11-20)

1. Backport setting cast shadows
    * [BitBucket pull request 210](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/210)

1. Ogre2: Add more shadow maps
    * [BitBucket pull request 203](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/203)

### Gazebo Rendering 2.1.2 (2019-09-10)

1. Enable gamma write on color texture in depth camera
    * [BitBucket pull request 193](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/193)

### Gazebo Rendering 2.1.1 (2019-09-06)

1. Add checks on mouse selection
    * [BitBucket pull request 191](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/191)

### Gazebo Rendering 2.1.0 (2019-08-27)

1. Support generating point cloud data in DepthCamera
    * [BitBucket pull request 187](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/187)

1. Fix GizmoVisual material memory leak
    * [BitBucket pull request 185](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/185)

1. Fix PixelFormat FLOAT32 byte and channel count
    * [BitBucket pull request 186](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/186)

### Gazebo Rendering 2.0.1 (2019-07-16)

1. Clean up camera objects when destroying scene
    * [BitBucket pull request 174](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/174)
    * [BitBucket pull request 180](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/180)

### Gazebo Rendering 2.0.0 (2019-05-20)

1. More ogre2 error messages for debugging mesh loading.
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/170)

1. Support for ogre 1.11 on windows
    * [BitBucket pull request 167](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/167)

1. Fix ogre mouse picking after window resize
    * [BitBucket pull request 168](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/168)

1. Ogre2: Support mouse picking for objects with overlay material
    * [BitBucket pull request 165](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/165)
    * [BitBucket pull request 166](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/166)

1. Add GizmoVisual class
    * [BitBucket pull request 163](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/163)

1. Add Transform Controller for translating, rotating, and scaling nodes.
    * [BitBucket pull request 162](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/162)

1. Add depth check and depth write properties to Material
    * [BitBucket pull request 160](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/160)

1. Move scale functions from Visual to Node class.
    * [BitBucket pull request 159](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/159)

1. Add support for Render Passes and implement Gaussian Noise pass
    * [BitBucket pull request 154](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/154)
    * [BitBucket pull request 155](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/155)

### Gazebo Rendering 1.X

1. Ogre2: Fix loading mesh with large index count
    * [BitBucket pull request 156](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/156)

1. Move OgreDepthCameraPrivate class to src file so it is not installed.
    * [BitBucket pull request 150](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/150)

### Gazebo Rendering 1.0.0 (2019-02-28)
