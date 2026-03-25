## Gazebo Common 6.x

### Gazebo Common 6.3.0 (2026-01-28)

1. Add functions for clearing FindFile* callbacks
    * [Pull request #754](https://github.com/gazebosim/gz-common/pull/754)

1. TempDirectory: improve windows reliability
    * [Pull request #744](https://github.com/gazebosim/gz-common/pull/744)

1. [bazel/infra] Add manual BCR release workflow
    * [Pull request #732](https://github.com/gazebosim/gz-common/pull/732)

1. Manual backport of: Bazel updates
    * [Pull request #726](https://github.com/gazebosim/gz-common/pull/726)

### Gazebo Common 6.2.1 (2025-11-12)

1. Fix Image::ChannelData for 16 bit RGB[A] images (backport #720)
    * [Pull request #721](https://github.com/gazebosim/gz-common/pull/721)

### Gazebo Common 6.2.0 (2025-11-11)

1. Add function for extracting single channel data from an RGB[A] image (backport #706)
    * [Pull request #709](https://github.com/gazebosim/gz-common/pull/709)

1. Support reading pixel values from color 16 bit images (backport #699)
    * [Pull request #701](https://github.com/gazebosim/gz-common/pull/701)

1. Bump assimp version to fix build with --dynamic_mode off
    * [Pull request #687](https://github.com/gazebosim/gz-common/pull/687)

### Gazebo Common 6.1.0 (2025-05-28)

1. Add support for custom profiler
    * [Pull request #682](https://github.com/gazebosim/gz-common/pull/682)

1. Migrate bazel build setup to use bzlmod
    * [Pull request #679](https://github.com/gazebosim/gz-common/pull/679)
    * [Pull request #681](https://github.com/gazebosim/gz-common/pull/681)
    * [Pull request #683](https://github.com/gazebosim/gz-common/pull/683)
    * [Pull request #684](https://github.com/gazebosim/gz-common/pull/684)

1. Add Heightmap utility functions
    * [Pull request #680](https://github.com/gazebosim/gz-common/pull/680)

1. Check valid indices before doing convex decomposition
    * [Pull request #677](https://github.com/gazebosim/gz-common/pull/677)

1. ci: run cppcheck, cpplint on noble
    * [Pull request #669](https://github.com/gazebosim/gz-common/pull/669)

1. Added missing includes
    * [Pull request #672](https://github.com/gazebosim/gz-common/pull/672)

1. Add check for valid indices in submesh
    * [Pull request #667](https://github.com/gazebosim/gz-common/pull/667)

1. Fix C4305 double truncated to float warnings (#666)
    * [Pull request #668](https://github.com/gazebosim/gz-common/pull/668)

1. Add missing include chrono in Event.hh header
    * [Pull request #664](https://github.com/gazebosim/gz-common/pull/664)

### Gazebo Common 6.0.2 (2025-02-12)

1. Add missing includes
    * [Pull request #660](https://github.com/gazebosim/gz-common/pull/660)

### Gazebo Common 6.0.1 (2024-12-17)

1. Check normal presence before trying to read asMesh normals
    * [Pull request #654](https://github.com/gazebosim/gz-common/pull/654)

1. Include `math::Angle` since it is used here
    * [Pull request #649](https://github.com/gazebosim/gz-common/pull/649)

1. Adds EoL to example
    * [Pull request #642](https://github.com/gazebosim/gz-common/pull/642)

1. Remove gts references from documentation
    * [Pull request #645](https://github.com/gazebosim/gz-common/pull/645)

## Gazebo Common 6.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 5.6.0 and earlier.

1. Add a note about the conda-distributed ffmpeg on windows
    * [Pull request #640](https://github.com/gazebosim/gz-common/pull/640)

1. Fix table of content
    * [Pull request #639](https://github.com/gazebosim/gz-common/pull/639)

1. Fix severity level of gzlog
    * [Pull request #635](https://github.com/gazebosim/gz-common/pull/635)

1. Fix loading lightmaps from gltf / glb meshes
    * [Pull request #630](https://github.com/gazebosim/gz-common/pull/630)

1. Fix AssimpLoader collada texture coordinates
    * [Pull request #634](https://github.com/gazebosim/gz-common/pull/634)

1. Implement console logging using `spdlog`
    * [Pull request #615](https://github.com/gazebosim/gz-common/pull/615)

1. Update Changelog, README and prepare for gz-common6.0.0~pre1 release
    * [Pull request #626](https://github.com/gazebosim/gz-common/pull/626)

1. Use self-pipe trick to implement signal handlers
    * [Pull request #618](https://github.com/gazebosim/gz-common/pull/618)

1. Replace GTS with CDT
    * [Pull request #617](https://github.com/gazebosim/gz-common/pull/617)

1. Remove debug logging introduced accidentally in PR review
    * [Pull request #622](https://github.com/gazebosim/gz-common/pull/622)

1. Fix crash when calling absPath with empty input
    * [Pull request #620](https://github.com/gazebosim/gz-common/pull/620)

1. DEM: Add support for GDAL vsicurl, vsizip support and avoid segfaults with huge VRT datasets
    * [Pull request #597](https://github.com/gazebosim/gz-common/pull/597)

1. Enable 24.04 CI, require cmake 3.22.1
    * [Pull request #619](https://github.com/gazebosim/gz-common/pull/619)

1. SubMesh::RecalculateNormals improvement
    * [Pull request #609](https://github.com/gazebosim/gz-common/pull/609)

1. fix data race tsan issue
    * [Pull request #612](https://github.com/gazebosim/gz-common/pull/612)

1. Generate a more unique texture name for glb embedded textures
    * [Pull request #606](https://github.com/gazebosim/gz-common/pull/606)

1. Add package.xml
    * [Pull request #587](https://github.com/gazebosim/gz-common/pull/587)

1. Fix deprecation warnings
    * [Pull request #603](https://github.com/gazebosim/gz-common/pull/603)

1. Fix macOS workflow and update on-push branches
    * [Pull request #602](https://github.com/gazebosim/gz-common/pull/602)

1. port: 5 to main
    * [Pull request #594](https://github.com/gazebosim/gz-common/pull/594)

1. Clean up Clang compiler warnings
    * [Pull request #589](https://github.com/gazebosim/gz-common/pull/589)

1. Add new function in MeshManager to merge all submeshes of a mesh into one
    * [Pull request #588](https://github.com/gazebosim/gz-common/pull/588)

1. Adds new function in MeshManager for performing convex decomposition
    * [Pull request #585](https://github.com/gazebosim/gz-common/pull/585)

1. Fix compatibility with FFmpeg5.0
    * [Pull request #581](https://github.com/gazebosim/gz-common/pull/581)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #568](https://github.com/gazebosim/gz-common/pull/568)

1. Fix search for UUID on Windows logic
    * [Pull request #556](https://github.com/gazebosim/gz-common/pull/556)

1. Fix windows compilation by setting right visibility attributes
    * [Pull request #550](https://github.com/gazebosim/gz-common/pull/550)

1. Drop the internal vendored version of TinyXML2
    * [Pull request #542](https://github.com/gazebosim/gz-common/pull/542)

1. Revert "Add missing visibility declarations (#548)"
    * [Pull request #553](https://github.com/gazebosim/gz-common/pull/553)

1. Add missing visibility declarations
    * [Pull request #548](https://github.com/gazebosim/gz-common/pull/548)

1. Remove ignition
    * [Pull request #525](https://github.com/gazebosim/gz-common/pull/525)

1. Bumps in ionic: use gz-cmake4, gz-utils3, gz-math8
    * [Pull request #539](https://github.com/gazebosim/gz-common/pull/539)

1. Remove deprecated std::iterator inheritance
    * [Pull request #529](https://github.com/gazebosim/gz-common/pull/529)

1. Collada and Assimp test for default animation names
    * [Pull request #476](https://github.com/gazebosim/gz-common/pull/476)

1. Avoid Io.hh header name clash
    * [Pull request #471](https://github.com/gazebosim/gz-common/pull/471)

1. Default name assignment to animations without names
    * [Pull request #413](https://github.com/gazebosim/gz-common/pull/413)

1. Skip CSV header when reading DataFrame
    * [Pull request #435](https://github.com/gazebosim/gz-common/pull/435)

1. Main deprecations
    * [Pull request #433](https://github.com/gazebosim/gz-common/pull/433)

1. Add CSV data parsing
    * [Pull request #402](https://github.com/gazebosim/gz-common/pull/402)

1. Introduces Flips UV flag, removes empty space
    * [Pull request #420](https://github.com/gazebosim/gz-common/pull/420)

1. ⬆️  Bump main to 6.0.0~pre1
    * [Pull request #412](https://github.com/gazebosim/gz-common/pull/412)

## Gazebo Common 5.x

### Gazebo Common 5.7.1 (2025-01-30)

1. Add include <chrono> for system_clock in Event.hh
    * [Pull request #655](https://github.com/gazebosim/gz-common/pull/655)

1. Check normal presence before trying to read asMesh normals
    * [Pull request #654](https://github.com/gazebosim/gz-common/pull/654)

1. Remove Conda Windows warnings
    * [Pull request #652](https://github.com/gazebosim/gz-common/pull/652)

## Gazebo Common 5.7.0 (2024-11-08)

1. Fix loading lightmaps from gltf / glb meshes (#630)
    * [Pull request #646](https://github.com/gazebosim/gz-common/pull/646)

1. Fix AssimpLoader collada texture coordinates
    * [Pull request #634](https://github.com/gazebosim/gz-common/pull/634)

1. Remove debug logging introduced accidentally in PR review
    * [Pull request #622](https://github.com/gazebosim/gz-common/pull/622)

1. Fix crash when calling absPath with empty input
    * [Pull request #620](https://github.com/gazebosim/gz-common/pull/620)

1. DEM: Add support for GDAL vsicurl, vsizip support and avoid segfaults with huge VRT datasets
    * [Pull request #597](https://github.com/gazebosim/gz-common/pull/597)

1. SubMesh::RecalculateNormals improvement
    * [Pull request #609](https://github.com/gazebosim/gz-common/pull/609)

1. fix data race tsan issue
    * [Pull request #612](https://github.com/gazebosim/gz-common/pull/612)

1. Generate a more unique texture name for glb embedded textures
    * [Pull request #606](https://github.com/gazebosim/gz-common/pull/606)

1. Add package.xml
    * [Pull request #587](https://github.com/gazebosim/gz-common/pull/587)

1. Fix deprecation warnings
    * [Pull request #603](https://github.com/gazebosim/gz-common/pull/603)

1. Fix macOS workflow and update on-push branches
    * [Pull request #602](https://github.com/gazebosim/gz-common/pull/602)

1. Fix compatibility with FFmpeg5.0
    * [Pull request #581](https://github.com/gazebosim/gz-common/pull/581)

## Gazebo Common 5.6.0 (2024-04-23)

1. Clarify units for the DEM classes
    * [Pull request #595](https://github.com/gazebosim/gz-common/pull/595)

1. Remove pessimizing move
    * [Pull request #593](https://github.com/gazebosim/gz-common/pull/593)

1. Fix bazel build
    * [Pull request #592](https://github.com/gazebosim/gz-common/pull/592)

1. Add new function in MeshManager to merge all submeshes of a mesh into one
    * [Pull request #588](https://github.com/gazebosim/gz-common/pull/588)

1. Adds new function in MeshManager for performing convex decomposition
    * [Pull request #585](https://github.com/gazebosim/gz-common/pull/585)

## Gazebo Common 5.5.1 (2024-03-14)

1. Various Bazel adjustments for linting
    * [Pull request #582](https://github.com/gazebosim/gz-common/pull/582)

1. Extend AssimpLoader to parse material transmission factor
    * [Pull request #577](https://github.com/gazebosim/gz-common/pull/577)

1. Fix noise issue that appears in certain image textures
    * [Pull request #578](https://github.com/gazebosim/gz-common/pull/578)

## Gazebo Common 5.5.0 (2024-02-26)

1. Be louder when graphics is missing for geospatial
    * [Pull request #573](https://github.com/gazebosim/gz-common/pull/573)

1. Multiple memory cleanup fixes
    * [Pull request #571](https://github.com/gazebosim/gz-common/pull/571)

1. Clarify GDAL version requirement
    * [Pull request #574](https://github.com/gazebosim/gz-common/pull/574)

1. 🎈  3.17.0
    * [Pull request #567](https://github.com/gazebosim/gz-common/pull/567)

1. Update CI badges in README
    * [Pull request #566](https://github.com/gazebosim/gz-common/pull/566)

1. Backwards compatible assimp texture name fix
    * [Pull request #565](https://github.com/gazebosim/gz-common/pull/565)

1. Fix error output when creating directories
    * [Pull request #561](https://github.com/gazebosim/gz-common/pull/561)

1. Update github action workflows
    * [Pull request #558](https://github.com/gazebosim/gz-common/pull/558)

1. Fix segfault in case of no write access to log dir
    * [Pull request #546](https://github.com/gazebosim/gz-common/pull/546)

1. ign -> gz
    * [Pull request #547](https://github.com/gazebosim/gz-common/pull/547)

1. Support loading glb with compressed jpeg textures
    * [Pull request #545](https://github.com/gazebosim/gz-common/pull/545)

1. Fix glTF / glb root node transform
    * [Pull request #543](https://github.com/gazebosim/gz-common/pull/543)

1. EnumIface: suppress deprecation warning
    * [Pull request #540](https://github.com/gazebosim/gz-common/pull/540)

1. Prevent loading lightmaps if mesh is a glb file that has an occlusion-metallic-roughness texture
    * [Pull request #538](https://github.com/gazebosim/gz-common/pull/538)

1. 🎈 3.16.0
    * [Pull request #519](https://github.com/gazebosim/gz-common/pull/519)

1. Fix cstdint with GCC 13
    * [Pull request #528](https://github.com/gazebosim/gz-common/pull/528)
    * [Pull request #517](https://github.com/gazebosim/gz-common/pull/517)
    * [Pull request #513](https://github.com/gazebosim/gz-common/pull/513)

## Gazebo Common 5.4.2 (2023-09-26)

1. Documentation fixes
    * [Pull request #534](https://github.com/gazebosim/gz-common/pull/534)
    * [Pull request #535](https://github.com/gazebosim/gz-common/pull/535)

1. Fix glTF metalness and roughness map orientation
    * [Pull request #532](https://github.com/gazebosim/gz-common/pull/532)

1. Build examples from CMake rather than executable
    * [Pull request #502](https://github.com/gazebosim/gz-common/pull/502)


## Gazebo Common 5.4.1 (2023-08-21)

1. Use `pull_request_target`  for triage workflow
    * [Pull request #527](https://github.com/gazebosim/gz-common/pull/527)

1. Fix Github project automation for new project board
    * [Pull request #526](https://github.com/gazebosim/gz-common/pull/526)

1. Fix Github Actions on macOS
    * [Pull request #524](https://github.com/gazebosim/gz-common/pull/524)

1. Only build integration tests if libraries exist
    * [Pull request #523](https://github.com/gazebosim/gz-common/pull/523)

1. Fix compiling under linux
    * [Pull request #521](https://github.com/gazebosim/gz-common/pull/521)

1. Add missing <fstream> header includes
    * [Pull request #518](https://github.com/gazebosim/gz-common/pull/518)

1. Header was dropped in the forward port and breaks downstream
    * [Pull request #515](https://github.com/gazebosim/gz-common/pull/515)

1. Port: 4 to 5
    * [Pull request #511](https://github.com/gazebosim/gz-common/pull/511)

1. Lint
    * [Pull request #Lint](https://github.com/gazebosim/gz-common/pull/Lint)

1. 🎈 4.7.0
    * [Pull request #510](https://github.com/gazebosim/gz-common/pull/510)

1. Fix build error when using gz:: with ign-common4
    * [Pull request #489](https://github.com/gazebosim/gz-common/pull/489)

## Gazebo Common 5.4.0 (2023-04-28)

1. Add support for bayer images to be saved in a directory
    * [Pull request #498](https://github.com/gazebosim/gz-common/pull/498)

1. Fix for ffmpeg v6
    * [Pull request #497](https://github.com/gazebosim/gz-common/pull/497)

1. Include cstring for memcpy
    * [Pull request #501](https://github.com/gazebosim/gz-common/pull/501)

1. Fixed MeshManager Singleton
    * [Pull request #451](https://github.com/gazebosim/gz-common/pull/451)

1. Fix for ffmpeg v6
    * [Pull request #497](https://github.com/gazebosim/gz-common/pull/497)

1. Rename COPYING to LICENSE
    * [Pull request #494](https://github.com/gazebosim/gz-common/pull/494)

1. Add marcoag as codeowner
    * [Pull request #493](https://github.com/gazebosim/gz-common/pull/493)

1. CI workflow: use checkout v3
    * [Pull request #490](https://github.com/gazebosim/gz-common/pull/490)

1. Update common5 for bazel builds
    * [Pull request #488](https://github.com/gazebosim/gz-common/pull/488)

1. Fix profile tutorial
    * [Pull request #487](https://github.com/gazebosim/gz-common/pull/487)

1. Update version in Changelog
    * [Pull request #486](https://github.com/gazebosim/gz-common/pull/486)

1. Fixed metalness/roughness in ASSIMP loader
    * [Pull request #485](https://github.com/gazebosim/gz-common/pull/485)

1. Fix Capsule mesh generation
    * [Pull request #475](https://github.com/gazebosim/gz-common/pull/475)

1. Improved coverage remotery
    * [Pull request #467](https://github.com/gazebosim/gz-common/pull/467)

1. Added BVH and STL loader tests
    * [Pull request #466](https://github.com/gazebosim/gz-common/pull/466)

1. Increased Image coverage
    * [Pull request #465](https://github.com/gazebosim/gz-common/pull/465)

## Gazebo Common 5.3.1 (2022-11-17)

1. Fix out of bounds access to rotation array in assimp loader
    * [Pull request #479](https://github.com/gazebosim/ign-common/pull/479)

1. Fix issues detected by ASAN
    * [Pull request #480](https://github.com/gazebosim/ign-common/pull/480)

## Gazebo Common 5.3.0 (2022-11-14)

1. Expose Vertex & Index raw ptrs for efficient reading
    * [Pull request #474](https://github.com/gazebosim/ign-common/pull/474)

## Gazebo Common 5.2.2 (2022-10-26)

1. [Backport] Avoid Io.hh header name clash (#471)
    * [Pull request #472](https://github.com/gazebosim/gz-common/pull/472)

## Gazebo Common 5.2.1 (2022-10-19)

1. Fix arm builds and tests
    * [Pull request #462](https://github.com/gazebosim/gz-common/pull/462)
    * [Pull request #463](https://github.com/gazebosim/gz-common/pull/463)

## Gazebo Common 5.2.0 (2022-10-18)

1. Add CSV data parsing
    * [Pull request #402](https://github.com/gazebosim/gz-common/pull/402)

1. Skip CSV header when reading DataFrame.
    * [Pull request #435](https://github.com/gazebosim/gz-common/pull/435)

1. Adds an API to retrieve keys.
    * [Pull request #446](https://github.com/gazebosim/gz-common/pull/446)

## Gazebo Common 5.1.0 (2022-10-13)

1. 4 ➡️  5
    * [Pull request #457](https://github.com/gazebosim/gz-common/pull/457)

1. 🎈 4.6.1
    * [Pull request #456](https://github.com/gazebosim/gz-common/pull/456)

1. 🎈 4.6.0
    * [Pull request #452](https://github.com/gazebosim/gz-common/pull/452)

1. 🎈 4.5.2
    * [Pull request #423](https://github.com/gazebosim/gz-common/pull/423)

1. 3 ➡️  4
    * [Pull request #448](https://github.com/gazebosim/gz-common/pull/448)
    * [Pull request #445](https://github.com/gazebosim/gz-common/pull/445)

1. 🎈 3.15.1
    * [Pull request #454](https://github.com/gazebosim/gz-common/pull/454)

1. 🎈 3.15.0
    * [Pull request #447](https://github.com/gazebosim/gz-common/pull/447)

1. 🎈 3.14.2
    * [Pull request #419](https://github.com/gazebosim/gz-common/pull/419)

1. Don't install auto-generated profiler.hh
    * [Pull request #453](https://github.com/gazebosim/gz-common/pull/453)

1. Fixed MeshManager Singleton
    * [Pull request #451](https://github.com/gazebosim/gz-common/pull/451)

1. Don't install CMakeLists.txt
    * [Pull request #449](https://github.com/gazebosim/gz-common/pull/449)

1. ign -> gz Migrate Ignition Headers : gz-common
    * [Pull request #418](https://github.com/gazebosim/gz-common/pull/418)

1. Suppress a bunch of unnecessary test output
    * [Pull request #442](https://github.com/gazebosim/gz-common/pull/442)

1. Fix metallic roughness file path for gltf assets
    * [Pull request #436](https://github.com/gazebosim/gz-common/pull/436)

1. Include cstring for memcpy
    * [Pull request #437](https://github.com/gazebosim/gz-common/pull/437)

1. Backport from 5 to 3 - Fix deprecation warning from new ffmpeg
    * [Pull request #421](https://github.com/gazebosim/gz-common/pull/421)

1. Remove redundant namespace references
    * [Pull request #414](https://github.com/gazebosim/gz-common/pull/414)

## Gazebo Common 5.0.0 (2022-09-22)

1. Improved visualization hw encoding tutorial gz-common5
    * [Pull request #441](https://github.com/gazebosim/gz-common/pull/441)

1. Updated install instructions gz-common5
    * [Pull request #440](https://github.com/gazebosim/gz-common/pull/440)

1. Fix reading pixel values from single channel 16 bit image
    * [Pull request #439](https://github.com/gazebosim/gz-common/pull/439)

1. Add gdal to prerequisites
    * [Pull request #438](https://github.com/gazebosim/gz-common/pull/438)

1. Cleanup long-deprecated ifdefs
    * [Pull request #329](https://github.com/gazebosim/gz-common/pull/329)

1. Add a test for loading gltf with external textures
    * [Pull request #409](https://github.com/gazebosim/gz-common/pull/409)

1. Fix ABI checker with testing module
    * [Pull request #427](https://github.com/gazebosim/gz-common/pull/427)

1. Implement XDisplacement function for checking x displacement for all nodes
    * [Pull request #411](https://github.com/gazebosim/gz-common/pull/411)

1. Fix deprecation warning from new ffmpeg
    * [Pull request #416](https://github.com/gazebosim/gz-common/pull/416)

1. Use assimp for loading meshes
    * [Pull request #393](https://github.com/gazebosim/gz-common/pull/393)

1. Add API for getting Image bytes as std::vector
    * [Pull request #372](https://github.com/gazebosim/gz-common/pull/372)

1. Support lunar coordinate transformations
    * [Pull request #377](https://github.com/gazebosim/gz-common/pull/377)

1. Remove configure.bat, export CXX_STANDARD
    * [Pull request #387](https://github.com/gazebosim/gz-common/pull/387)

1. Update GoogleTest to latest version
    * [Pull request #361](https://github.com/gazebosim/gz-common/pull/361)

1. Support compressed images and RGBA data in Image class
    * [Pull request #368](https://github.com/gazebosim/gz-common/pull/368)

1. Expose API to set non earth DEMs flag
    * [Pull request #353](https://github.com/gazebosim/gz-common/pull/353)

1. Fix console bug when IGN_HOMEDIR is unset
    * [Pull request #346](https://github.com/gazebosim/gz-common/pull/346)

1. Test updates for common::testing module.
    * [Pull request #315](https://github.com/gazebosim/gz-common/pull/315)

1. Ignition -> Gz Renaming
    * Move header files with git mv
    * Create redirection aliases
    * Migrate sources in src, test, examples, and include
    * Remove ignition redirection headers for Plugin [Pull request #350](https://github.com/gazebosim/gz-common/pull/350)
    * Move Plugin headers back from gz to ignition [Pull request #350](https://github.com/gazebosim/gz-common/pull/350)
    * ign -> gz: namespaces and logging functions [Pull request #356](https://github.com/gazebosim/gz-common/pull/356)
    * Deprecation warnings for Plugin [Pull request #350](https://github.com/gazebosim/gz-common/pull/350)
    * Use ignition headers for plugin [Pull request #350](https://github.com/gazebosim/gz-common/pull/350)
    * Fix profiler option [Pull request #371](https://github.com/gazebosim/gz-common/pull/371)
    * ign -> gz Macro Migration : gz-common [Pull request #366](https://github.com/gazebosim/gz-common/pull/366)
    * [ign -> gz] CMake functions [Pull request #370](https://github.com/gazebosim/gz-common/pull/370)
    * ign -> gz Environment Variable Migration [Pull request #365](https://github.com/gazebosim/gz-common/pull/365)
    * Migrate IGNITION- CMake variables [Pull request #381](https://github.com/gazebosim/gz-common/pull/381)
    * Rename CMake project to gz [Pull request #352](https://github.com/gazebosim/gz-common/pull/352)
    * ign -> gz Partial Docs Migration and Project Name Followups : gz-common [Pull request #383](https://github.com/gazebosim/gz-common/pull/383)
    * Partial cmake and source migrations [Pull request #392](https://github.com/gazebosim/gz-common/pull/392)
    * Migrate ign_remotery_vis [Pull request #392](https://github.com/gazebosim/gz-common/pull/392)
    * Finale: Source hard-tocks [Pull request #395](https://github.com/gazebosim/gz-common/pull/395)
    * Remove deprecated include [Pull request #400](https://github.com/gazebosim/gz-common/pull/400)

1. [DOCUMENTATION] Adds full path for the profiler
    * [Pull request #347](https://github.com/gazebosim/gz-common/pull/347)

1. prevent switch case statement fall through.
    * [Pull request #345](https://github.com/gazebosim/gz-common/pull/345)

1. Make Timer_TEST more robust
    * [Pull request #343](https://github.com/gazebosim/gz-common/pull/343)

1. Bumps in garden : ign-common5
    * [Pull request #342](https://github.com/gazebosim/gz-common/pull/342)

1. Use ign-utils instead of ign-cmake utilities
    * [Pull request #341](https://github.com/gazebosim/gz-common/pull/341)

1. Bumps in garden : ign-common5
    * [Pull request #340](https://github.com/gazebosim/gz-common/pull/340)

1. examples/CMakeLists.txt: fix find version variable
    * [Pull request #339](https://github.com/gazebosim/gz-common/pull/339)

1. Add common::testing module
    * [Pull request #314](https://github.com/gazebosim/gz-common/pull/314)

1. Fix typo bug in MaterialIndex
    * [Pull request #338](https://github.com/gazebosim/gz-common/pull/338)

1. Fix edge case handling for SubMesh::MaterialIndex
    * [Pull request #319](https://github.com/gazebosim/gz-common/pull/319)

1. DEM improvements to nodata, buffer and other corner cases
    * [Pull request #321](https://github.com/gazebosim/gz-common/pull/321)

1. 🧹 Drop all test main functions from gtest files
    * [Pull request #322](https://github.com/gazebosim/gz-common/pull/322)

1. Class with final destructor should be final
    * [Pull request #323](https://github.com/gazebosim/gz-common/pull/323)

1. Geospatial component for heightmaps and DEM support
    * [Pull request #292](https://github.com/gazebosim/gz-common/pull/292)

1. Fix filesystem::separator, parentPath and basename behavior
    * [Pull request #308](https://github.com/gazebosim/gz-common/pull/308)

1. Miscellaneous cleanup of Events API
    * [Pull request #297](https://github.com/gazebosim/gz-common/pull/297)

1. Adds an `isRelativePath` to determine if path is relative
    * [Pull request #312](https://github.com/gazebosim/gz-common/pull/312)

1. Image: make several methods const
    * [Pull request #310](https://github.com/gazebosim/gz-common/pull/310)

1. Utilize ImplPtr in AV component
    * [Pull request #296](https://github.com/gazebosim/gz-common/pull/296)

1. Update tests to use more STL functionality
    * [Pull request #282](https://github.com/gazebosim/gz-common/pull/282)

1. Fixed parentPath() return when input has no parent
    * [Pull request #307](https://github.com/gazebosim/gz-common/pull/307)

1. Utilize ImplPtr for Timer
    * [Pull request #299](https://github.com/gazebosim/gz-common/pull/299)

1. Update to use std::filesystem rather than Boost
    * [Pull request #238](https://github.com/gazebosim/gz-common/pull/238)

1. Use SuppressWarning from ign-utils
    * [Pull request #295](https://github.com/gazebosim/gz-common/pull/295)

1. README: update badge
    * [Pull request #290](https://github.com/gazebosim/gz-common/pull/290)

1. Revert geospatial component for now
    * [Pull request #291](https://github.com/gazebosim/gz-common/pull/291)

1. Move geospatial headers to subfolder
    * [Pull request #289](https://github.com/gazebosim/gz-common/pull/289)

1. Geospatial component for heightmap & DEMs
    * [Pull request #267](https://github.com/gazebosim/gz-common/pull/267)

1. Upload coverage for Focal builds
    * [Pull request #284](https://github.com/gazebosim/gz-common/pull/284)

1. Try to checkout matching branch in macos workflow
    * [Pull request #286](https://github.com/gazebosim/gz-common/pull/286)

1. Bumps in garden: use ignition-math7
    * [Pull request #285](https://github.com/gazebosim/gz-common/pull/285)

1. Add macOS 11 workflow
    * [Pull request #276](https://github.com/gazebosim/gz-common/pull/276)

1. Use ImplPtr where relevant
    * [Pull request #274](https://github.com/gazebosim/gz-common/pull/274)

1. Remove all deprecated functionality from main
    * [Pull request #273](https://github.com/gazebosim/gz-common/pull/273)

1. ign-common5: drop Bionic
    * [Pull request #270](https://github.com/gazebosim/gz-common/pull/270)

1. Update tension for common5
    * [Pull request #260](https://github.com/gazebosim/gz-common/pull/260)

1. Bump main to 5.0.0~pre1
    * [Pull request #193](https://github.com/gazebosim/gz-common/pull/193)

## Gazebo Common 4.x

## Gazebo Common 4.7.0 (2023-05-17)

1. Forward Ports: 3 -> 4 (#506)
    * [Pull request #506](https://github.com/gazebosim/gz-common/pull/506)
    * [Pull request #500](https://github.com/gazebosim/gz-common/pull/500)
    * [Pull request #495](https://github.com/gazebosim/gz-common/pull/495)
    * [Pull request #491](https://github.com/gazebosim/gz-common/pull/491)

1. Fix build error when using gz:: with ign-common4
    * [Pull request #489](https://github.com/gazebosim/gz-common/pull/489)

1. Fix for ffmpeg v6
    * [Pull request #497](https://github.com/gazebosim/gz-common/pull/497)

1. Include cstring for memcpy
    * [Pull request #501](https://github.com/gazebosim/gz-common/pull/501)

1. Fixed MeshManager Singleton
    * [Pull request #451](https://github.com/gazebosim/gz-common/pull/451)

1. Rename COPYING to LICENSE
    * [Pull request #494](https://github.com/gazebosim/gz-common/pull/494)

1. Add marcoag as codeowner
    * [Pull request #493](https://github.com/gazebosim/gz-common/pull/493)

1. CI workflow: use checkout v3
    * [Pull request #490](https://github.com/gazebosim/gz-common/pull/490)

1. Improved coverage remotery
    * [Pull request #467](https://github.com/gazebosim/gz-common/pull/467)

1. Added BVH and STL loader tests
    * [Pull request #466](https://github.com/gazebosim/gz-common/pull/466)

1. Increased Image coverage
    * [Pull request #465](https://github.com/gazebosim/gz-common/pull/465)

## Gazebo Common 4.6.2 (2022-11-17)

1. Fix Capsule mesh generation.
    * [Pull request #475](https://github.com/gazebosim/gz-common/pull/475)

## Gazebo Common 4.6.1 (2022-10-11)

1. All common3 changes up to 3.15.1 release

## Gazebo Common 4.6.0 (2022-10-11)

1. All common3 changes up to 3.15.0 release

1. Include cstring for memcpy
    * [Pull request #437](https://github.com/gazebosim/gz-common/pull/437)

## Gazebo Common 4.5.2 (2022-08-16)

1. Fix deprecation warning from new `ffmpeg`
    * [Pull request #421](https://github.com/gazebosim/gz-common/pull/421)

1. Remove redundant namespace references
    * [Pull request #414](https://github.com/gazebosim/gz-common/pull/414)

1. Add code coverage ignore file
    * [Pull request #394](https://github.com/gazebosim/gz-common/pull/394)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #399](https://github.com/gazebosim/gz-common/pull/399)

1. Ignition -> Gazebo
    * [Pull request #396](https://github.com/gazebosim/gz-common/pull/396)

1. Support absolute Win paths with forward slashes in `common::FindFile`
    * [Pull request #389](https://github.com/gazebosim/gz-common/pull/389)

1. Fix `FindSharedLibrary` for relative plugin paths
    * [Pull request #382](https://github.com/gazebosim/gz-common/pull/382)

1. Fix race condition in `FileLogger`
    * [Pull request #378](https://github.com/gazebosim/gz-common/pull/378)

1. Synchronize console writes
    * [Pull request #227](https://github.com/gazebosim/gz-common/pull/227)

1. Profiler tutorial: viewing from Docker container
    * [Pull request #362](https://github.com/gazebosim/gz-common/pull/362)

## Gazebo Common 4.5.1 (2022-06-21)

1. Fix spelling in URI error message
    * [Pull request #333](https://github.com/gazebosim/gz-common/pull/333)

1. `gzLogInit` fails if run after console output
    * [Pull request #332](https://github.com/gazebosim/gz-common/pull/332)

1. Fix compatibility with FFmpeg 5.0
    * [Pull request #325](https://github.com/gazebosim/gz-common/pull/325)

1. packages.apt: don't install ruby-ronn
    * [Pull request #324](https://github.com/gazebosim/gz-common/pull/324)

1. Add some tests for `parentPath`/`basename`
    * [Pull request #309](https://github.com/gazebosim/gz-common/pull/309)

1. Add CI support for Jammy
    * [Pull request #316](https://github.com/gazebosim/gz-common/pull/316)

1. Minor typo in filesystem documentation
    * [Pull request #313](https://github.com/gazebosim/gz-common/pull/313)

1. Fix out-of-bounds access in sanitizeSlashes
    * [Pull request #303](https://github.com/gazebosim/gz-common/pull/303)

1. Fix out-of-bound access in URI
    * [Pull request #304](https://github.com/gazebosim/gz-common/pull/304)

1. Move performance test to examples
    * [Pull request #294](https://github.com/gazebosim/gz-common/pull/294)

1. Fix memory corruption & leaks in Image
    * [Pull request #240](https://github.com/gazebosim/gz-common/pull/240)

## Gazebo Common 4.5.0 (2022-01-12)

1. Fixed crash when a Collada file has an empty normal vector
    * [Pull request #280](https://github.com/gazebosim/gz-common/pull/280)

1. Support 16 bit heightmaps
    * [Pull request #266](https://github.com/gazebosim/gz-common/pull/266)

1. Fix bug in URIPath assignment operator
    * [Pull request #275](https://github.com/gazebosim/gz-common/pull/275)

1. Use `libexec` to install binary `remotery_vis`
    * [Pull request #272](https://github.com/gazebosim/gz-common/pull/272)

1. Normalize normal vectors from OBJ.
    * [Pull request #269](https://github.com/gazebosim/gz-common/pull/269)

1. Synchronize console writes
    * [Pull request #227](https://github.com/gazebosim/gz-common/pull/227)

1. Added method to remove meshes from the `MeshManager`
    * [Pull request #222](https://github.com/gazebosim/gz-common/pull/222)

1. Fixed macOS symbol in `common::Profiler`
    * [Pull request #262](https://github.com/gazebosim/gz-common/pull/262)

1. Fix skip logic for integration tests
    * [Pull request #264](https://github.com/gazebosim/gz-common/pull/264)

1. Use direct evaluation for SKIP_av.
    * [Pull request #250](https://github.com/gazebosim/gz-common/pull/250)

## Gazebo Common 4.4.0 (2021-10-15)

1. Add support for animation tension
    * [Pull request #256](https://github.com/gazebosim/gz-common/pull/256)

## Gazebo Common 4.3.0 (2021-09-27)

1. Remove gz-utils from TempDirectory
    * [Pull request #248](https://github.com/gazebosim/gz-common/pull/248)

1. Add functions and objects for Temporary Directories
    * [Pull request #244](https://github.com/gazebosim/gz-common/pull/244)

1. Fix memory corruption & leaks in Image
    * [Pull request #240](https://github.com/gazebosim/gz-common/pull/240)

1. Fix a typo in VideoEncoder_TEST.
    * [Pull request #231](https://github.com/gazebosim/gz-common/pull/231)

1. Fix segfault caused by destruction order of Event and Connection
    * [Pull request #234](https://github.com/gazebosim/gz-common/pull/234)

1. Infrastructure
    * [Pull request #62](https://github.com/gazebosim/gz-common/pull/62)
    * [Pull request #55](https://github.com/gazebosim/gz-common/pull/55)
    * [Pull request #241](https://github.com/gazebosim/gz-common/pull/241)

1. Documentation
    * [Pull request #252](https://github.com/gazebosim/gz-common/pull/252)
    * [Pull request #253](https://github.com/gazebosim/gz-common/pull/253)

## Gazebo Common 4.2.0 (2021-08-02)

1. Export lights to dae
    * [Pull request #228](https://github.com/gazebosim/gz-common/pull/228)

1. Add cstring for std::memcpy
    * [Pull request #230](https://github.com/gazebosim/gz-common/pull/230)

## Gazebo Common 4.1.0 (2021-06-22)

1. Make KeyEvent rule-of-five compliant
    * [Pull request #224](https://github.com/gazebosim/gz-common/pull/224)

1. Fix `av_*` API usage for deprecations
    * [Pull request #220](https://github.com/gazebosim/gz-common/pull/220)

1. Set project-wide standard to C++17
    * [Pull request #221](https://github.com/gazebosim/gz-common/pull/221)

1. Remove `CMAKE_CXX_FLAGS` from test targets
    * [Pull request #214](https://github.com/gazebosim/gz-common/pull/214)

1. Support loading PBR textures in OBJLoader
    * [Pull request #216](https://github.com/gazebosim/gz-common/pull/216)

1. Remove `tools/code_check` and update codecov
    * [Pull request #219](https://github.com/gazebosim/gz-common/pull/219)

1. Port codecov to new configuration
    * [Pull request #212](https://github.com/gazebosim/gz-common/pull/212)

1. Fix loading collada files with multiple texcoord sets using the same offset
    * [Pull request #208](https://github.com/gazebosim/gz-common/pull/208)

1. Add function to convert single channel image data to RGB image
    * [Pull request #205](https://github.com/gazebosim/gz-common/pull/205)

1. Remove ColladaExporter path constraint
    * [Pull request #204](https://github.com/gazebosim/gz-common/pull/204)

1. Avoid duplication of / in joinPaths (Windows)
    * [Pull request #201](https://github.com/gazebosim/gz-common/pull/201)
    * [Pull request #209](https://github.com/gazebosim/gz-common/pull/209)
    * [Pull request #215](https://github.com/gazebosim/gz-common/pull/215)

1. Fixed colladaLoader on Windows
    * [Pull request #200](https://github.com/gazebosim/gz-common/pull/200)

1. Improved Windows support
    * [Pull request #197](https://github.com/gazebosim/gz-common/pull/197)

1. Function to serialize compressed image to buffer
    * [Pull request #139](https://github.com/gazebosim/gz-common/pull/139)

1. Fixed tutorial link
    * [Pull request #194](https://github.com/gazebosim/gz-common/pull/194)

1. Use matching malloc/free for AudioDecoder.
    * [Pull request #192](https://github.com/gazebosim/gz-common/pull/192)

1. Add bazel buildsystem support
    * [Pull request #123](https://github.com/gazebosim/gz-common/pull/123)

## Gazebo Common 4.0.0 (2021-03-30)

1. Include windows instructions in examples README.md
    * [Pull request #190](https://github.com/gazebosim/gz-common/pull/190)

1. Fix crash when using BVH animations
    * [Pull request #188](https://github.com/gazebosim/gz-common/pull/188)

1. Rotate capsule to Z-up
    * [Pull request #186](https://github.com/gazebosim/gz-common/pull/186)

1. Add implementation pointer to gz-common classes
    * [Pull request #163](https://github.com/gazebosim/gz-common/pull/163)

1. Cleanup and add implementation pointer to Animation classes
    * [Pull request #160](https://github.com/gazebosim/gz-common/pull/160)

1. Install the profiler in a version-safe manner
    * [Pull request #170](https://github.com/gazebosim/gz-common/pull/170)

1. Restore support for model:// URIs
    * [Pull request #172](https://github.com/gazebosim/gz-common/pull/172)

1. Deprecated common::Time
    * [Pull request #90](https://github.com/gazebosim/gz-common/pull/90)

1. Update codeowners
    * [Pull request #88](https://github.com/gazebosim/gz-common/pull/88)

1. Workflow updates
    * [Pull request #64](https://github.com/gazebosim/gz-common/pull/64)

1. Update BitBucket Links
    * [Pull request #58](https://github.com/gazebosim/gz-common/pull/58)

1. Corrected `BAYER_RGGR8` to `BAYER_BGGR8` in `PixelFormatName` and
   `PixelFormatType` located in `graphics/include/gz/common/Image.hh`.
    * [BitBucket pull request 191](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/191)

1. Added URI Authority parsing to the URI class. Authority information can
   be found [here](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier#Generic_syntax). This update also caused some formally valid syntax to become invalid, and some formally invalid syntax to become valid. See the migration guide.
    * [BitBucket pull request xxx](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/213)

## Gazebo Common 3.x

## Gazebo Common 3.17.0 (2024-01-05)

1. Fix error output when creating directories
    * [Pull request #561](https://github.com/gazebosim/gz-common/pull/561)

1. Update github action workflows
    * [Pull request #558](https://github.com/gazebosim/gz-common/pull/558)

1. Fix segfault in case of no write access to log dir
    * [Pull request #546](https://github.com/gazebosim/gz-common/pull/546)

## Gazebo Common 3.16.0 (2023-06-05)

1. Include cstdint to build with GCC 13
    * [Pull request #517](https://github.com/gazebosim/gz-common/pull/517)

1. Fix missing cstdint header in latest gcc build
    * [Pull request #513](https://github.com/gazebosim/gz-common/pull/513)

1. Fix for ffmpeg v6
    * [Pull request #497](https://github.com/gazebosim/gz-common/pull/497)

1. Include cstring for memcpy
    * [Pull request #501](https://github.com/gazebosim/gz-common/pull/501)

1. Fixed MeshManager Singleton
    * [Pull request #451](https://github.com/gazebosim/gz-common/pull/451)

1. Rename COPYING to LICENSE
    * [Pull request #494](https://github.com/gazebosim/gz-common/pull/494)

1. Add marcoag as codeowner
    * [Pull request #493](https://github.com/gazebosim/gz-common/pull/493)

1. CI workflow: use checkout v3
    * [Pull request #490](https://github.com/gazebosim/gz-common/pull/490)

1. Improved coverage remotery
    * [Pull request #467](https://github.com/gazebosim/gz-common/pull/467)

1. Added BVH and STL loader tests
    * [Pull request #466](https://github.com/gazebosim/gz-common/pull/466)

1. Increased Image coverage
    * [Pull request #465](https://github.com/gazebosim/gz-common/pull/465)

## Gazebo Common 3.15.1 (2022-10-11)

1. Fix build on case-insensitive filesystems
    * [Pull request #453](https://github.com/gazebosim/gz-common/pull/453)

1. Don't install CMakeLists.txt
    * [Pull request #449](https://github.com/gazebosim/gz-common/pull/449)

## Gazebo Common 3.15.0 (2022-10-06)

1. ign -> gz Migrate Ignition Headers
    * [Pull request #418](https://github.com/gazebosim/gz-common/pull/418)

## Gazebo Common 3.14.2 (2022-08-16)

1. Fix deprecation warning from new `ffmpeg`
    * [Pull request #414](https://github.com/gazebosim/gz-common/pull/421)

## Gazebo Common 3.14.1 (2022-06-01)

1. Profiler tutorial: viewing from Docker container
    * [Pull request #362](https://github.com/gazebosim/gz-common/pull/362)

1. ignLogInit can be called after an ignerr, etc
    * [Pull request #332](https://github.com/gazebosim/gz-common/pull/332)

1. Fix compatibility with FFmpeg 5.0
    * [Pull request #325](https://github.com/gazebosim/gz-common/pull/325)

1. Fix out-of-bounds access in sanitizeSlashes
    * [Pull request #303](https://github.com/gazebosim/gz-common/pull/303)

1. Fix memory corruption & leaks in Image
    * [Pull request #287](https://github.com/gazebosim/gz-common/pull/287)

1. Normalize normal vectors from OBJ.
    * [Pull request #269](https://github.com/gazebosim/gz-common/pull/269)

1. Fixed macos symbol in `common::profiler`
    * [Pull request #262](https://github.com/gazebosim/gz-common/pull/262)

1. Fix skip logic for integration tests
    * [Pull request #264](https://github.com/gazebosim/gz-common/pull/264)

## Gazebo Common 3.14.0 (2021-10-12)

1. Support loading PBR textures in OBJLoader
    * [Pull request #216](https://github.com/gazebosim/gz-common/pull/216)

1. Remove CMAKE_CXX_FLAGS from test targetrs
    * [Pull request #214](https://github.com/gazebosim/gz-common/pull/214)

1. Set project-wide standard to C++17
    * [Pull request #221](https://github.com/gazebosim/gz-common/pull/221)

1. Fix av_* API usage for deprecations
    * [Pull request #220](https://github.com/gazebosim/gz-common/pull/220)

1. Make KeyEvent rule-of-five compliant
    * [Pull request #224](https://github.com/gazebosim/gz-common/pull/224)

1. Fix segfault caused by destructionb order of Event and Connection
    * [Pull request #234](https://github.com/gazebosim/gz-common/pull/234)

1. Fix a typo in VideoEncoder_TEST
    * [Pull request #231](https://github.com/gazebosim/gz-common/pull/231)

1. Use direct evaluation for SKIP_av
    * [Pull request #250](https://github.com/gazebosim/gz-common/pull/250)

## Gazebo Common 3.13.2 (2021-05-11)

1. Backport collada fixes (Backport #204)
    * [Pull request #211](https://github.com/gazebosim/gz-common/pull/211)

1. Fix join paths again
    * [Pull request #215](https://github.com/gazebosim/gz-common/pull/215)

## Gazebo Common 3.13.1 (2021-05-03)

1. Fix join paths to remove duplicate separators
    * [Pull request #205](https://github.com/gazebosim/gz-common/pull/209)

1. Fix loading collada files with multiple texcoord sets using the same offset.
    * [Pull request #208](https://github.com/gazebosim/gz-common/pull/208)

## Gazebo Common 3.13.0 (2021-04-27)

1. Add function to convert single channel image data to RGB image
    * [Pull request #205](https://github.com/gazebosim/gz-common/pull/205)

1. Avoid duplication of / in joinPaths (Windows)
    * [Pull request #201](https://github.com/gazebosim/gz-common/pull/201)

1. Fix colladaLoader on Windows
    * [Pull request #200](https://github.com/gazebosim/gz-common/pull/200)

1. Backport #188: Fix crash when using BVH animations.
    * [Pull request #199](https://github.com/gazebosim/gz-common/pull/199)

1. Improved Windows support
    * [Pull request #197](https://github.com/gazebosim/gz-common/pull/197)

## Gazebo Common 3.12.0 (2021-04-06)

1. Remove use of _SOURCE and _BINARY dirs in tests.
    * [Pull request #158](https://github.com/gazebosim/gz-common/pull/158)

1. Add bazel buildsystem support.
    * [Pull request #123](https://github.com/gazebosim/gz-common/pull/123)

1. Use matching malloc/free for AudioDecoder.
    * [Pull request #192](https://github.com/gazebosim/gz-common/pull/192)

1. Fixed tutorial link in the README.md.
    * [Pull request #194](https://github.com/gazebosim/gz-common/pull/194)

1. Function to serialize compressed image to buffer
    * [Pull request #139](https://github.com/gazebosim/gz-common/pull/139)

## Gazebo Common 3.11.1 (2021-03-08)

1. Relax expectation so encoder test passes on ARM
    * [Pull request #183](https://github.com/gazebosim/gz-common/pull/183)

1. Revert "Associate library materials effect with meshes (#151)"
    * [Pull request #182](https://github.com/gazebosim/gz-common/pull/182)

## Gazebo Common 3.11.0 (2021-03-05)

1. Fix image red and blue swapping
    * [Pull request #162](https://github.com/gazebosim/gz-common/pull/162)

1. Remove issue & PR templates
    * [Pull request #174](https://github.com/gazebosim/gz-common/pull/174)

1. Prevent console spamming when exporting a mesh that has multiple texture coordinates
    * [Pull request #171](https://github.com/gazebosim/gz-common/pull/171)

1. Associate library materials effect with meshes
    * [Pull request #151](https://github.com/gazebosim/gz-common/pull/151)
    * Reverted in 3.11.1

1. Added issue & PR templates
    * [Pull request #166](https://github.com/gazebosim/gz-common/pull/166)

1. Added support for HW-accelerated video encoding.
    * [Pull request #125](https://github.com/gazebosim/gz-common/pull/125)
    * [Pull request #169](https://github.com/gazebosim/gz-common/pull/169)
    * [Pull request #175](https://github.com/gazebosim/gz-common/pull/175)

1. Added FlagSet utility class
    * [Pull request #118](https://github.com/gazebosim/gz-common/pull/118)

1. Disable failing VideoEncoder and Audioecoder tests on Windows
    * [Pull request #149](https://github.com/gazebosim/gz-common/pull/149)

1. Added ellipsoid mesh
    * [Pull request #154](https://github.com/gazebosim/gz-common/pull/154)
    * [Pull request #159](https://github.com/gazebosim/gz-common/pull/159)

1. Added capsule mesh
    * [Pull request #155](https://github.com/gazebosim/gz-common/pull/155)

## Gazebo Common 3.10.1 (2021-01-20)

1. Fix transparency tag in the Collada exporter.
    * [Pull Request 152](https://github.com/gazebosim/gz-common/pull/152)

## Gazebo Common 3.10.0 (2021-01-05)

1. Added render order to material class
    * [Pull Request 142](https://github.com/gazebosim/gz-common/pull/142)

1. Fix memory leak in Pbr.cc
    * [Pull Request 147](https://github.com/gazebosim/gz-common/pull/147)

## Gazebo Common 3.9.0 (2020-12-21)

1. Support multiple texture coordinate sets
    * [Pull Request 131](https://github.com/gazebosim/gz-common/pull/131)

1. Add Lightmap to material
    * [Pull Request 132](https://github.com/gazebosim/gz-common/pull/132)

## Gazebo Common 3.8.0 (2020-12-08)

1. Call gz::common::load() from Video constructor
    * [Pull Request 111](https://github.com/gazebosim/gz-common/pull/111)

1. Cleanup
    * Drop unused forward declaration: [Pull Request 113](https://github.com/gazebosim/gz-common/pull/113)
    * Remove two known flaky tests from Windows CI: [Pull Request 119](https://github.com/gazebosim/gz-common/pull/119)
    * Make ignstrtok private to Util.cc: [Pull Request 134](https://github.com/gazebosim/gz-common/pull/134)
    * Fix memory leaks [Pull Request 136](https://github.com/gazebosim/gz-common/pull/136)

1. Windows
    * Do not hardcode USE_EXTERNAL_TINYXML2 to OFF: [Pull Request 116](https://github.com/gazebosim/gz-common/pull/116)
    * Fix GZ_HOMEDIR on Windows: [Pull Request 127](https://github.com/gazebosim/gz-common/pull/127)
    * Improve Windows support: [Pull Request 128](https://github.com/gazebosim/gz-common/pull/128)
    * Fix Windows AV CI warnings and test failures: [Pull Request 135](https://github.com/gazebosim/gz-common/pull/135)

1. Fix video encoder timing
    * [Pull Request 105](https://github.com/gazebosim/gz-common/pull/105)

1. Feature: Add transformation matrix to ColladaExport
    * [Pull Request 100](https://github.com/gazebosim/gz-common/pull/100)
    * Fix: [Pull Request 133](https://github.com/gazebosim/gz-common/pull/133)

## Gazebo Common 3.7.0 (2020-10-21)

1. Miscellaneous Cleanups
    * Feature: Join function for combining strings with a delimiter
    * [Pull Request 107](https://github.com/gazebosim/gz-common/pull/107)

1. Improve fork experience
    * [Pull Request 103](https://github.com/gazebosim/gz-common/pull/103)

1. Add Console example
    * [Pull Request 106](https://github.com/gazebosim/gz-common/pull/106)

1. Support fuel URLs for textures
    * [Pull Request 102](https://github.com/gazebosim/gz-common/pull/102)

1. Fix loading dae file with hierarchical node that does not have a name
    * [Pull Request 101](https://github.com/gazebosim/gz-common/pull/101)

1. Fix more Animation memory leaks
    * [Pull Request 98](https://github.com/gazebosim/gz-common/pull/98)

1. Fix env behavior to return true on empty vars
    * Feature: Add setenv for setting environment variables
    * Feature: Add unsetenv for clearing environment variables
    * Feature: Add alternative env implementation that respects empty variables
    * [Pull Request 97](https://github.com/gazebosim/gz-common/pull/97)

## Gazebo Common 3.6.1 (2020-09-16)

1. Fix hierarchical submesh processing in the ColladaLoader.
    * [Pull Request 78](https://github.com/gazebosim/gz-common/pull/78)

1. Tutorial updates
    * [Pull Request 86](https://github.com/gazebosim/gz-common/pull/86)
    * [Pull Request 87](https://github.com/gazebosim/gz-common/pull/87)
    * [Pull Request 92](https://github.com/gazebosim/gz-common/pull/92)

1. Fix the AudioDecoder class
    * [Pull Request 81](https://github.com/gazebosim/gz-common/pull/81)

1. Fix actions CI
    * [Pull Request 83](https://github.com/gazebosim/gz-common/pull/83)

1. Fix trajectory info memory leak
    * [Pull Request 93](https://github.com/gazebosim/gz-common/pull/93)

1. Remove URI warning on constructor
    * [Pull Request 94](https://github.com/gazebosim/gz-common/pull/94)

## Gazebo Common 3.6.0 (2020-06-09)

1. Refactor Image::MaxColor to reduce computation time
    * [Pull request 66](https://github.com/gazebosim/gz-common/pull/66)

1. Update to gtest 1.10.0 with patch for gcc warnings
    * [Pull request 67](https://github.com/gazebosim/gz-common/pull/67)

1. Fix ColladaLoader loading <transparent> tag when <transparency> does not exist
    * [Pull request 68](https://github.com/gazebosim/gz-common/pull/68)

1. Remove unused included headers
    * [Pull request 71](https://github.com/gazebosim/gz-common/pull/71)

1. More file path APIs
    * [Pull request 70](https://github.com/gazebosim/gz-common/pull/70)

## Gazebo Common 3.5.0 (2020-04-09)

1. Add interpolate\_x property to actor animations
    * [BitBucket pull request 232](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/232)

1. Update PBR metalness default value to 0.0
    * [BitBucket pull request 231](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/231)

1. Fix crash when loading OBJ meshes with invalid materials
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/230)

## Gazebo Common 3.4.0 (2020-02-20)

1. Add PBR material class
    * [BitBucket pull request 227](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/227)

1. Custom paths for file URIs (env var + URI)
    * [BitBucket pull request 210](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/210)

1. Add function to get NodeAnimation
    * [BitBucket pull request 223](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/223)

1. Handle unexpected COLLADA more gracefully
    * [BitBucket pull request 218](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/218)

1. Add support for alpha based texture transparency.
    * [BitBucket pull request 221](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/221)

1. `tiny_obj_loader`: set better diffuse value when `map_kD` is present and `Kd` is not specified.
    * [BitBucket pull request 216](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/216)
    * [BitBucket pull request 217](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/217)

1. ColladaLoader: load files with actors with multiple skeletons
    * [BitBucket pull request 215](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/215)

1. Compute volume of mesh shapes with `Mesh::Volume` and `SubMesh::Volume`.
    * [BitBucket pull request 214](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/214)

1. Port Skeleton fixes from gazebo.
    * [BitBucket pull request 209](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/209)

1. Port gazebo9 fix for ColladaLoader with multiple geometries
    * [BitBucket pull request 208](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/208)

1. `SystemPaths::FindSharedLibrary`: return input if its is an absolute path to a file
    * [BitBucket pull request 206](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/206)

1. Fix `Image::ConvertPixelFormat` for `BAYER_BGGR8`
    * [BitBucket pull request 211](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/211)

1. Support custom callbacks to find files on global interface
    * [BitBucket pull request 226](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/226)

## Gazebo Common 3.3.0 (2019-08-27)

1. skip GTSMeshUtils test on Windows due to issue #50
    * [BitBucket pull request 205](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/205)

1. Add bvh animation support for skeleton and TrajectoryInfo
    * [BitBucket pull request 197](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/197)

1. ColladaLoader: fix wrong node weights caused by buffer overflow bug
    * [BitBucket pull request 201](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/201)

1. Filesystem copy directory and parentpath
    * [BitBucket pull request 200](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/200)

## Gazebo Common 3.2.0 (2019-08-07)

1. Allow gzLogInit to use an absolute path. Added a timeToIso that converts a given time to an ISO string. Some console timestamps were using `GZ_SYSTEM_TIME_NS()` and others `gz::common::systemTimeISO()`. Switched all to use `gz::common::systemTimeISO()`.
    * [BitBucket pull request 203](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/203)

1. Port ColladaLoader fix: use default value of 1 for stride parameter when unset.
    * [BitBucket pull request 196](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/196)

## Gazebo Common 3.1.0 (2019-05-17)

1. Image::PixelFormatType: append `BAYER_BGGR8` instead of replacing `BAYER_RGGR8`
    * [BitBucket pull request 192](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/192)

1. Use `std::tolower` with locale in lowercase helper function
    * [BitBucket pull request 190](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/190)

1. Profiler: give hint on osx to find frameworks so it doesn't find them inside Xcode
    * [BitBucket pull request 189](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/189)

1. Fix windows tests: SystemPathsFixture.FileSystemPaths and TimeTest.Sleep
    * [BitBucket pull request 188](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/188)

1. Filesystem functions to create unique paths
    * [BitBucket pull request 187](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/187)

1. Battery additions for LinearBatteryPlugin port to gz-sim
    * [BitBucket pull request 186](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/186)

1. Consolidate Console::log streams to reduce colored log size
    * [BitBucket pull request 185](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/185)
    * [Issue 47](https://github.com/gazebosim/gz-common/issues/47)

1. Fix ColladaLoader to support mixamo models and fix skeleton animation loading
    * [BitBucket pull request 179](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/179)
    * [BitBucket osrf/gazebo pull request 3071](https://osrf-migration.github.io/gazebo-gh-pages/#!/osrf/gazebo/pull-requests/3071)

## Gazebo Common 3.0.0 (2019-02-28)

1. Use gz-cmake2 and support Ubuntu Bionic (18.04).
    * [BitBucket pull request 131](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/131)

1. WorkerPool: allow user to set minimum number of worker threads.
    * [BitBucket pull request 151](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/151)

1. ColladaLoader fixes: setting multiple texcoord's, check for line breaks when using `split`
    * [BitBucket pull request 152](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/152)
    * [BitBucket pull request 157](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/157)

1. Event: replace `operator()` and `Signal()` explicit templates with variadic templates and perfect forwarding.
    * [BitBucket pull request 156](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/156)

1. Event: use second template argument to disambiguate events with matching signatures.
    * [BitBucket pull request 164](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/164)
    * [BitBucket pull request 166](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/166)

1. Add profiler using Remotery.
    * [BitBucket pull request 162](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/162)
    * [BitBucket pull request 169](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/169)

1. Added hash64 function for converting `std::string` to `uint64_t` as `constexpr`.
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/170)
    * [BitBucket pull request 171](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/171)
    * [BitBucket pull request 172](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/172)

1. SystemPaths: search paths in `GZ_FILE_PATH` environment variable when finding files
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/175)

1. `Time::Sleep`: return amount of time actually slept
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/176)
    * [Issue 44](https://github.com/gazebosim/gz-common/issues/44)

1. NodeTransform: use unique_ptr for private data, add copy constructor/assignment
    * [BitBucket pull request 181](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/181)

## Gazebo Common 2.x

## Gazebo Common 2.x.x (2018-XX-XX)

## Gazebo Common 2.0.0 (2018-02-11)

1. Use ignition-cmake1 and components for av, events, and graphics
    * [BitBucket pull request 102](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/102)
    * [BitBucket pull request 103](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/103)
    * [BitBucket pull request 105](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/105)
    * [BitBucket pull request 106](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/106)
    * [BitBucket pull request 107](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/107)
    * [BitBucket pull request 108](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/108)
    * [BitBucket pull request 109](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/109)

1. Expand plugin functionality with PluginPtr and specializations
    * [BitBucket pull request 59](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/59)

1. Added signal handler class
    * [BitBucket pull request 115](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/115)

1. Update tinyobjloader to version that supports triangulating meshes with concave polygons.
    * [BitBucket pull request 160](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/160)

## Gazebo Common 1.x.x (2018-XX-XX)


## Gazebo Common 1.X.X (20XX-XX-XX)

1. Time::Sleep: use CLOCK_MONOTONIC on Linux
    * [BitBucket pull request 159](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/159)


## Gazebo Common 1.1.1 (2018-05-23)

1. Documentation upload improvements
    * [BitBucket pull request 117](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/117)

1. Fix build on homebrew with ffmpeg4
    * [BitBucket pull request 119](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/119)

1. Remove symbolic link directory
    * [BitBucket pull request 124](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/124)


## Gazebo Common 1.1.0 (2018-04-16)

1. Accept spaces on URI path
    * [BitBucket pull request 110](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/110)
    * [BitBucket pull request 111](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/111)

1. Fix gcc7 compiler warnings on artful
    * [BitBucket pull request 97](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/97)

1. Fix artful cppcheck
    * [BitBucket pull request 99](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/99)

1. Fix nanosecond to second conversion
    * [BitBucket pull request 100](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/100)
