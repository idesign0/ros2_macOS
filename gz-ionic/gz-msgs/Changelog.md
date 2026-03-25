## Gazebo Msgs 11.x

### Gazebo Msgs 11.1.0 (2025-05-09)

1. Add CI support for bzlmod
    * [Pull request #508](https://github.com/gazebosim/gz-msgs/pull/508)

1. Migrate bazel build setup to use bzlmod
    * [Pull request #507](https://github.com/gazebosim/gz-msgs/pull/507)

1. ci: enable doxygen check on noble
    * [Pull request #505](https://github.com/gazebosim/gz-msgs/pull/505)

1. Fix compatibility with protobuf v30 (cpp 6.30.0) (#499)
    * [Pull request #500](https://github.com/gazebosim/gz-msgs/pull/500)

1. Code cleanup (#485)
    * [Pull request #497](https://github.com/gazebosim/gz-msgs/pull/497)

1. Update maintainer email (#487)
    * [Pull request #490](https://github.com/gazebosim/gz-msgs/pull/490)

1. cmake extras: only find Python3 if needed
    * [Pull request #479](https://github.com/gazebosim/gz-msgs/pull/479)

1. Add rule to build python version of protos
    * [Pull request #474](https://github.com/gazebosim/gz-msgs/pull/474)

1. Return only unique message types in `MessageFactory::Types` function
    * [Pull request #472](https://github.com/gazebosim/gz-msgs/pull/472)

### Gazebo Msgs 11.0.2 (2025-01-10)

1. cmake extras: only find Python3 if needed
    * [Pull request #479](https://github.com/gazebosim/gz-msgs/pull/479)

1. ci.yml: run cppcheck, cpplint on noble
    * [Pull request #478](https://github.com/gazebosim/gz-msgs/pull/478)

### Gazebo Msgs 11.0.1 (2024-10-10)

1. Fix bash completion script install path
    * [Pull request #469](https://github.com/gazebosim/gz-msgs/pull/469)

### Gazebo Msgs 11.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 10.3.0 and earlier.

1. Miscellaneous documentation fixes
    * [Pull request #458](https://github.com/gazebosim/gz-msgs/pull/458)
    * [Pull request #461](https://github.com/gazebosim/gz-msgs/pull/461)
    * [Pull request #459](https://github.com/gazebosim/gz-msgs/pull/459)
    * [Pull request #457](https://github.com/gazebosim/gz-msgs/pull/457)

1. Update gz-msgs11 badge URLs
    * [Pull request #455](https://github.com/gazebosim/gz-msgs/pull/455)

1. Improve migration note on SphericalCoordinates
    * [Pull request #452](https://github.com/gazebosim/gz-msgs/pull/452)
    * [Pull request #451](https://github.com/gazebosim/gz-msgs/pull/451)

1. Adding cone primitives.
    * [Pull request #442](https://github.com/gazebosim/gz-msgs/pull/442)

1. Remove cone shape from particle emitter
    * [Pull request #446](https://github.com/gazebosim/gz-msgs/pull/446)

1. Enable 24.04 CI, require cmake 3.22.1
    * [Pull request #444](https://github.com/gazebosim/gz-msgs/pull/444)

1. Include config.hh to get needed macros
    * [Pull request #426](https://github.com/gazebosim/gz-msgs/pull/426)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #413](https://github.com/gazebosim/gz-msgs/pull/413)

1. Support standalone executable in gz-msgs11
    * [Pull request #357](https://github.com/gazebosim/gz-msgs/pull/357)

1. Added motion duration to gui_camera.proto
    * [Pull request #408](https://github.com/gazebosim/gz-msgs/pull/408)

1. Use HIDE_SYMBOLS_BY_DEFAULT
    * [Pull request #407](https://github.com/gazebosim/gz-msgs/pull/407)

1. Bumps in ionic : ci_matching_branch/bump_ionic_gz-msgs11
    * [Pull request #401](https://github.com/gazebosim/gz-msgs/pull/401)


## Gazebo Msgs 10.x

### Gazebo Msgs 10.3.2 (2025-02-10)

1. cmake extras: only find Python3 if needed
    * [Pull request #479](https://github.com/gazebosim/gz-msgs/pull/479)

### Gazebo Msgs 10.3.1 (2024-11-11)

1. Add rule to build python version of protos
    * [Pull request #474](https://github.com/gazebosim/gz-msgs/pull/474)

1. Return only unique message types in `MessageFactory::Types` function
    * [Pull request #472](https://github.com/gazebosim/gz-msgs/pull/472)

### Gazebo Msgs 10.3.0 (2024-06-18)

1. Backport: Adding cone primitives.
    * [Pull request #442](https://github.com/gazebosim/gz-msgs/pull/442)

### Gazebo Msgs 10.2.0 (2024-05-17)

1. CameraTrack message for advanced tracking and following.
    * [Pull request #440](https://github.com/gazebosim/gz-msgs/pull/440)

1. Update minimum version of gz-cmake3
    * [Pull request #439](https://github.com/gazebosim/gz-msgs/pull/439)

1. bazel: lint
    * [Pull request #438](https://github.com/gazebosim/gz-msgs/pull/438)

1. Add package.xml
    * [Pull request #432](https://github.com/gazebosim/gz-msgs/pull/432)

### Gazebo Msgs 10.1.2 (2024-04-08)

1. Use relative install paths for gz tool data and extra cmake files
    * [Pull request #436](https://github.com/gazebosim/gz-msgs/pull/436)

1. Bazel updates for Harmonic
    * [Pull request #397](https://github.com/gazebosim/gz-msgs/pull/397)

1. Fix MSVC "possible loss of data" warning
    * [Pull request #434](https://github.com/gazebosim/gz-msgs/pull/434)

### Gazebo Msgs 10.1.1 (2024-03-14)

1. Allow topic and service to construct messages from description files
    * [Pull request #428](https://github.com/gazebosim/gz-msgs/pull/428)

1. Conditionally use cmake_path on cmake < 3.20
    * [Pull request #424](https://github.com/gazebosim/gz-msgs/pull/424)

1. Remove duplicate find_package(Python3) call
    * [Pull request #420](https://github.com/gazebosim/gz-msgs/pull/420)

1. Bazel: Fix py_binary location
    * [Pull request #406](https://github.com/gazebosim/gz-msgs/pull/406)

1. Fix compiler warning in `generator.cc`
    * [Pull request #403](https://github.com/gazebosim/gz-msgs/pull/403)

1. Set GZ_TOOLS_VER to 2 for consistency with rest of Garden and Harmonic libraries
    * [Pull request #391](https://github.com/gazebosim/gz-msgs/pull/391)

### Gazebo Msgs 10.1.0 (2024-01-22)

1. Add proto message for MaterialColor.
    * [Pull request #414](https://github.com/gazebosim/gz-msgs/pull/414)
    * [Pull request #415](https://github.com/gazebosim/gz-msgs/pull/415)
    * [Pull request #416](https://github.com/gazebosim/gz-msgs/pull/416)

1. Update CI badges in README
    * [Pull request #412](https://github.com/gazebosim/gz-msgs/pull/412)

1. Ensure that tests pass fine with GZ_RELOCATABLE_INSTALL option enabled
    * [Pull request #394](https://github.com/gazebosim/gz-msgs/pull/394)

1. Added missing parts to the `CMakeLists.txt` code.
    * [Pull request #400](https://github.com/gazebosim/gz-msgs/pull/400)

1. Miscellaneous cleanups of protobuf generator plugin
    * [Pull request #405](https://github.com/gazebosim/gz-msgs/pull/405)

1. Fix memory leak in generator plugin code
    * [Pull request #404](https://github.com/gazebosim/gz-msgs/pull/404)

1. Enables cmd commands on Windows
    * [Pull request #402](https://github.com/gazebosim/gz-msgs/pull/402)

1. Miscellaneous housekeeping
    * [Pull request #398](https://github.com/gazebosim/gz-msgs/pull/398)

1. Fix cross-compilation support for gz-msg10
    * [Pull request #392](https://github.com/gazebosim/gz-msgs/pull/392)

1. Do not redefine GZ_PYTHON_INSTALL_PATH if it is already defined
    * [Pull request #393](https://github.com/gazebosim/gz-msgs/pull/393)

### Gazebo Msgs 10.0.0 (2023-09-29)

1. Add missing `<functional>` header
    * [Pull request #382](https://github.com/gazebosim/gz-msgs/pull/382)

1. Documentation fixes
    * [Pull request #381](https://github.com/gazebosim/gz-msgs/pull/381)

1. Generate messages in downstream builds
    * [Pull request #339](https://github.com/gazebosim/gz-msgs/pull/339)
    * [Pull request #356](https://github.com/gazebosim/gz-msgs/pull/356)
    * [Pull request #359](https://github.com/gazebosim/gz-msgs/pull/359)
    * [Pull request #361](https://github.com/gazebosim/gz-msgs/pull/361)
    * [Pull request #368](https://github.com/gazebosim/gz-msgs/pull/368)
    * [Pull request #374](https://github.com/gazebosim/gz-msgs/pull/374)
    * [Pull request #377](https://github.com/gazebosim/gz-msgs/pull/377)
    * [Pull request #379](https://github.com/gazebosim/gz-msgs/pull/379)
    * [Pull request #386](https://github.com/gazebosim/gz-msgs/pull/386)
    * [Pull request #384](https://github.com/gazebosim/gz-msgs/pull/384)

1. Add python message generation
    * [Pull request #362](https://github.com/gazebosim/gz-msgs/pull/362)
    * [Pull request #364](https://github.com/gazebosim/gz-msgs/pull/364)

1. Added AirSpeedSensor msgs to Sensor msg
    * [Pull request #365](https://github.com/gazebosim/gz-msgs/pull/365)

1. Add covariance fields to imu message
    * [Pull request #333](https://github.com/gazebosim/gz-msgs/pull/333)

1. Remove ignition
    * [Pull request #367](https://github.com/gazebosim/gz-msgs/pull/367)
    * [Pull request #335](https://github.com/gazebosim/gz-msgs/pull/335)

1. Infrastructure
    * [Pull request #370](https://github.com/gazebosim/gz-msgs/pull/370)
    * [Pull request #369](https://github.com/gazebosim/gz-msgs/pull/369)
    * [Pull request #363](https://github.com/gazebosim/gz-msgs/pull/363)
    * [Pull request #331](https://github.com/gazebosim/gz-msgs/pull/331)

1. Added cubemap_uri to sky
    * [Pull request #306](https://github.com/gazebosim/gz-msgs/pull/306)

1. Add id and visibility flags field to projector msg
    * [Pull request #345](https://github.com/gazebosim/gz-msgs/pull/345)

1. Add new discovery types
    * [Pull request #322](https://github.com/gazebosim/gz-msgs/pull/322)

1. ⬆️  Bump main to 10.0.0~pre1
    * [Pull request #292](https://github.com/gazebosim/gz-msgs/pull/292)


## Gazebo Msgs 9.x

### Gazebo Msgs 9.5.0 (2023-08-25)

1. Remove deprecated flag output_to_genfiles
    * [Pull request #347](https://github.com/gazebosim/gz-msgs/pull/347)

1. GzProtobuf: Do not require version 3
    * [Pull request #346](https://github.com/gazebosim/gz-msgs/pull/346)

1. Fix typos
    * [Pull request #344](https://github.com/gazebosim/gz-msgs/pull/344)
    * [Pull request #314](https://github.com/gazebosim/gz-msgs/pull/314)

1. Few clangtidy fixes
    * [Pull request #343](https://github.com/gazebosim/gz-msgs/pull/343)

1. Remove unused load in gz_msgs_generate
    * [Pull request #342](https://github.com/gazebosim/gz-msgs/pull/342)

1. Remove unused ignstrtok variable
    * [Pull request #340](https://github.com/gazebosim/gz-msgs/pull/340)

1. Adds a message that allows loading environments via a topic
    * [Pull request #320](https://github.com/gazebosim/gz-msgs/pull/320)

1. Rename COPYING to LICENSE
    * [Pull request #330](https://github.com/gazebosim/gz-msgs/pull/330)

1. Infrastructure
    * [Pull request #370](https://github.com/gazebosim/gz-msgs/pull/370)
    * [Pull request #369](https://github.com/gazebosim/gz-msgs/pull/369)
    * [Pull request #331](https://github.com/gazebosim/gz-msgs/pull/331)


### Gazebo Msgs 9.4.0 (2023-03-10)

1. Support for bazel in garden
    * [Pull request #325](https://github.com/gazebosim/ign-msgs/pull/325)

1. Add entity wrench map
    * [Pull request #328](https://github.com/gazebosim/ign-msgs/pull/328)

### Gazebo Msgs 9.3.0 (2023-02-08)

1. Added airspeed sensor
    * [Pull request #321](https://github.com/gazebosim/gz-msgs/pull/321)

1. Fix cross-compilation workflow
    * [Pull request #319](https://github.com/gazebosim/gz-msgs/pull/319)

### Gazebo Msgs 9.2.0 (2022-11-23)

1. Add DVL specific messages
    * [Pull request #317](https://github.com/gazebosim/gz-msgs/pull/317)

### Gazebo Msgs 9.1.0 (2022-11-17)

1. Forward port of msgs 8.7.0.
    * [Pull request #313](https://github.com/gazebosim/gz-msgs/pull/313)

### Gazebo Msgs 9.0.0 (2022-09-22)

1. Move generated message to details folder
    * [Pull request #304](https://github.com/gazebosim/gz-msgs/pull/304)

1. gz_TEST: one more ign -> gz
    * [Pull request #301](https://github.com/gazebosim/gz-msgs/pull/301)

1. Remove incorrect redirection header
    * [Pull request #303](https://github.com/gazebosim/gz-msgs/pull/303)

1. Migrate config and log directories
    * [Pull request #291](https://github.com/gazebosim/gz-msgs/pull/291)

1. Adding additional fields to the camera sensor message
    * [Pull request #201](https://github.com/gazebosim/gz-msgs/pull/201)

1. Add fluid added mass to inertial
    * [Pull request #271](https://github.com/gazebosim/gz-msgs/pull/271)

1. Safer move/rename for Windows
    * [Pull request #285](https://github.com/gazebosim/gz-msgs/pull/285)

1. Added stepping bool to world statistics
    * [Pull request #199](https://github.com/gazebosim/gz-msgs/pull/199)

1. [ign to gz] Factory::New accept ignition with warning
    * [Pull request #281](https://github.com/gazebosim/gz-msgs/pull/281)

1. Allow moon and custom surfaces in spherical coords msg
    * [Pull request #279](https://github.com/gazebosim/gz-msgs/pull/279)

1. Change message generation workflow
    * [Pull request #256](https://github.com/gazebosim/gz-msgs/pull/256)

1. ign -> gz Provisional Finale: Source Migration : gz-msgs
    * [Pull request #272](https://github.com/gazebosim/gz-msgs/pull/272)

1. Migrate bazel file
    * [Pull request #270](https://github.com/gazebosim/gz-msgs/pull/270)

1. Update GoogleTest to latest version
    * [Pull request #257](https://github.com/gazebosim/gz-msgs/pull/257)

1. ign -> gz Partial Docs Migration and Project Name Followups : gz-msgs
    * [Pull request #267](https://github.com/gazebosim/gz-msgs/pull/267)

1. Migrate IGNITION- CMake variables
    * [Pull request #266](https://github.com/gazebosim/gz-msgs/pull/266)

1. Rename CMake project to gz
    * [Pull request #250](https://github.com/gazebosim/gz-msgs/pull/250)

1. ign -> gz CLI Migration : gz-msgs
    * [Pull request #262](https://github.com/gazebosim/gz-msgs/pull/262)

1. ign -> gz Upstream Macro Migration : gz-msgs
    * [Pull request #260](https://github.com/gazebosim/gz-msgs/pull/260)

1. [ign -> gz] CMake functions
    * [Pull request #263](https://github.com/gazebosim/gz-msgs/pull/263)

1. ign -> gz Environment Variable Migration
    * [Pull request #259](https://github.com/gazebosim/gz-msgs/pull/259)

1. [garden] Added shininess to material message
    * [Pull request #251](https://github.com/gazebosim/gz-msgs/pull/251)

1. ign -> gz: namespaces and CMake var
    * [Pull request #252](https://github.com/gazebosim/gz-msgs/pull/252)

1. ign -> gz migration
    * [Pull request #249](https://github.com/gazebosim/gz-msgs/pull/249)

1. Added visualize_visual to light.proto
    * [Pull request #248](https://github.com/gazebosim/gz-msgs/pull/248)

1. Bumps in garden : ign-msgs9
    * [Pull request #246](https://github.com/gazebosim/gz-msgs/pull/246)

1. Remove SuppressWarning from msgs.hh to fix warnings
    * [Pull request #245](https://github.com/gazebosim/gz-msgs/pull/245)

1. Use SuppressWarning from ign-utils
    * [Pull request #243](https://github.com/gazebosim/gz-msgs/pull/243)

1. Remove Bionic from future releases (Garden+)
    * [Pull request #229](https://github.com/gazebosim/gz-msgs/pull/229)

1. Change is_light_on to is_light_off
    * [Pull request #228](https://github.com/gazebosim/gz-msgs/pull/228)

1. Added field to light.proto to define if a light is on or off
    * [Pull request #223](https://github.com/gazebosim/gz-msgs/pull/223)

1. Added continuous joint type
    * [Pull request #215](https://github.com/gazebosim/gz-msgs/pull/215)

1. Bumps in garden : depend on ign-math7
    * [Pull request #208](https://github.com/gazebosim/gz-msgs/pull/208)

1. Configurable marker point color
    * [Pull request #202](https://github.com/gazebosim/gz-msgs/pull/202)

1. Bumps in garden : ci_matching_branch/bump_garden_ign-msgs9
    * [Pull request #203](https://github.com/gazebosim/gz-msgs/pull/203)

## Gazebo Msgs 8.x

### Gazebo Msgs 8.7.0 (2022-09-27)

1. ign -> gz Redirect Gz Headers
    * [Pull request #300](https://github.com/gazebosim/gz-msgs/pull/300)
    * [Pull request #307](https://github.com/gazebosim/gz-msgs/pull/307)

1. Add parameter component message definitions
    * [Pull request #241](https://github.com/gazebosim/gz-msgs/pull/241)
    * [Pull request #298](https://github.com/gazebosim/gz-msgs/pull/298)

1. Remove redundant namespace references
    * [Pull request #293](https://github.com/gazebosim/gz-msgs/pull/293)

### Gazebo Msgs 8.6.0 (2022-08-02)

1. Increase `Utility` test coverage
    * [Pull request #283](https://github.com/gazebosim/gz-msgs/pull/283)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #282](https://github.com/gazebosim/gz-msgs/pull/282)
    * [Pull request #284](https://github.com/gazebosim/gz-msgs/pull/284)

1. Add entity wrench message
    * [Pull request #274](https://github.com/gazebosim/gz-msgs/pull/274)

1. Ignition -> Gazebo
    * [Pull request #273](https://github.com/gazebosim/gz-msgs/pull/273)

1. Add `ACTOR` and `WORLD` types to entity
    * [Pull request #275](https://github.com/gazebosim/gz-msgs/pull/275)

### Gazebo Msgs 8.5.0 (2022-06-23)

1. Add entity plugin v proto file
    * [Pull request #261](https://github.com/gazebosim/gz-msgs/pull/261)

1. Bash completion for flags
    * [Pull request #254](https://github.com/gazebosim/gz-msgs/pull/254)

1. Remove dead files
    * [Pull request #255](https://github.com/gazebosim/gz-msgs/pull/255)

1. Fix typo REGRESION/REGRESSION
    * [Pull request #247](https://github.com/gazebosim/gz-msgs/pull/247)

1. Generator: Add missing std namespace to string arguments
    * [Pull request #242](https://github.com/gazebosim/gz-msgs/pull/242)

### Gazebo Msgs 8.4.0 (2022-04-06)

1. Add Dataframe msg
    * [Pull request #238](https://github.com/gazebosim/gz-msgs/pull/238)

### Gazebo Msgs 8.3.0 (2022-03-24)

1. Focal CI: static checkers and doxygen linters
    * [Pull request #230](https://github.com/gazebosim/gz-msgs/pull/230)

1. Pose, twist and odometry with covariance msgs
    * [Pull request #224](https://github.com/gazebosim/gz-msgs/pull/224)

1. Add wheel slip message definition
    * [Pull request #205](https://github.com/gazebosim/gz-msgs/pull/205)
    * [Pull request #227](https://github.com/gazebosim/gz-msgs/pull/227)

1. Add Ubuntu Jammy CI
    * [Pull request #225](https://github.com/gazebosim/gz-msgs/pull/225)

1. Add PointCloudPacked iterators
    * [Pull request #210](https://github.com/gazebosim/gz-msgs/pull/210)
    * [Pull request #218](https://github.com/gazebosim/gz-msgs/pull/218)

1. Document pose + spherical coordinates combination on spawn
    * [Pull request #212](https://github.com/gazebosim/gz-msgs/pull/212)

1. Utility_TEST: adjust expectations for 32-bit arch (#120)
    * [Pull request #120) (#196](https://github.com/gazebosim/gz-msgs/pull/120) (#196)

### Gazebo Msgs 8.2.0 (2021-12-14)

1. Add NavSat messages
    * [Pull request #206](https://github.com/gazebosim/gz-msgs/pull/206)

### Gazebo Msgs 8.1.0 (2021-11-02)

1. Added WorldControlState message.
    * [Pull request #190](https://github.com/gazebosim/gz-msgs/pull/190)

1. Fixed trivial typo in command line help message.
    * [Pull request #191](https://github.com/gazebosim/gz-msgs/pull/191)

### Gazebo Msgs 8.0.0 (2021-09-27)

1. scene.proto: add shadow_caster_material_name
    * [Pull request #179](https://github.com/gazebosim/gz-msgs/pull/179)

1. 🌐 Spherical coordinates conversion and commands
    * [Pull request #177](https://github.com/gazebosim/gz-msgs/pull/177)

1. Add BoundingBox Msg
    * [Pull request #165](https://github.com/gazebosim/gz-msgs/pull/165)

1. Add particle scatter ratio field to particle emitter
    * [Pull request #162](https://github.com/gazebosim/gz-msgs/pull/162)

1. Add particle emitters to link.proto and add topic to particle_emitter.proto
    * [Pull request #153](https://github.com/gazebosim/gz-msgs/pull/153)

1. Infrastructure
    * [Pull request #159](https://github.com/gazebosim/gz-msgs/pull/159)
    * [Pull request #155](https://github.com/gazebosim/gz-msgs/pull/155)
    * [Pull request #151](https://github.com/gazebosim/gz-msgs/pull/151)

## Gazebo Msgs 7.x

### Gazebo Msgs 7.3.0 (2022-03-17)

1. Backport NavSat message for bridge compatibility
    * [Pull request #231](https://github.com/gazebosim/gz-msgs/pull/231)

1. Focal CI: static checkers and doxygen linters
    * [Pull request #230](https://github.com/gazebosim/gz-msgs/pull/230)

1. Add wheel slip message definition
    * [Pull request #205](https://github.com/gazebosim/gz-msgs/pull/205)
    * [Pull request #227](https://github.com/gazebosim/gz-msgs/pull/227)

1. Add PointCloudPacked iterators
    * [Pull request #210](https://github.com/gazebosim/gz-msgs/pull/210)
    * [Pull request #218](https://github.com/gazebosim/gz-msgs/pull/218)

1. Utility\_TEST: adjust expectations for 32-bit arch (#120)
    * [Pull request #120) (#196](https://github.com/gazebosim/gz-msgs/pull/120) (#196)

1. Fix trivial typo in command line help message
    * [Pull request #191](https://github.com/gazebosim/gz-msgs/pull/191)

1. Support colcon in windows CI
    * [Pull request #189](https://github.com/gazebosim/gz-msgs/pull/189)

1. 🥳 Update gz-tools issue on README
    * [Pull request #184](https://github.com/gazebosim/gz-msgs/pull/184)

### Gazebo Msgs 7.2.0 (2021-09-10)

1. Adds PerformanceSensorMetrics proto message.
    * [Pull request #172](https://github.com/gazebosim/gz-msgs/pull/172)

1. Detect gz instead of using cmake module to check for gz-tools
    * [Pull request #166](https://github.com/gazebosim/gz-msgs/pull/166)

1. Bazel build support
    * [Pull request #95](https://github.com/gazebosim/gz-msgs/pull/95)

1. Remove tools/code_check and update codecov
    * [Pull request #164](https://github.com/gazebosim/gz-msgs/pull/164)

1. Fixed the width spelling in a comment
    * [Pull request #161](https://github.com/gazebosim/gz-msgs/pull/161)

### Gazebo Msgs 7.1.0 (2021-04-30)

1. Added particle_emitter vector message
    * [Pull request #149](https://github.com/gazebosim/gz-msgs/pull/149)

### Gazebo Msgs 7.0.0 (2021-03-30)

1. Master branch updates
    * [Pull request #141](https://github.com/gazebosim/gz-msgs/pull/141)

1. Added light intensity to light.proto
    * [Pull request #131](https://github.com/gazebosim/gz-msgs/pull/131)

1. Add double sided field to material
    * [Pull request #130](https://github.com/gazebosim/gz-msgs/pull/130)

1. Added capsule and ellipsoid geom msgs
    * [Pull request #128](https://github.com/gazebosim/gz-msgs/pull/128)

1. Add lightmap texcoord set
    * [Pull request #124](https://github.com/gazebosim/gz-msgs/pull/124)

1. README updates for Edifice
    * [Pull request #125](https://github.com/gazebosim/gz-msgs/pull/125)
    * [Pull request #148](https://github.com/gazebosim/gz-msgs/pull/148)

1. Added render order to material msgs
    * [Pull request #118](https://github.com/gazebosim/gz-msgs/pull/118)

1. Added run_to_sim_time to world_control message.
    * [Pull request #108](https://github.com/gazebosim/gz-msgs/pull/108)

1. Add a is_periodic_change field to SerializedStateMap
    * [Pull request #119](https://github.com/gazebosim/gz-msgs/pull/119)

1. Remove deprecations, and a now invalid todo
    * [Pull request #112](https://github.com/gazebosim/gz-msgs/pull/112)

1. Added light map to material.proto
    * [Pull request #111](https://github.com/gazebosim/gz-msgs/pull/111)

## Gazebo Msgs 6.x

### Gazebo Msgs 6.6.0 (2021-08-30)

1. Adds PerformanceSensorMetrics proto message.
    * [Pull request #172](https://github.com/gazebosim/gz-msgs/pull/172)

1. Detect gz instead of using cmake module to check for gz-tools
    * [Pull request #166](https://github.com/gazebosim/gz-msgs/pull/166)

1. Bazel build support for gz-msgs6
    * [Pull request #95](https://github.com/gazebosim/gz-msgs/pull/95)

1. Remove tools/code_check and update codecov
    * [Pull request #164](https://github.com/gazebosim/gz-msgs/pull/164)

### Gazebo Msgs 6.5.0 (2021-04-06)

1. Added particle_emitter vector message
    * [Pull request 149](https://github.com/gazebosim/gz-msgs/pull/149)

### Gazebo Msgs 6.4.0 (2021-03-01)

1. Support 'has' functionality in particle Emitter.
    * [Pull request 137](https://github.com/gazebosim/gz-msgs/pull/137)

1. Add gz version of nav_msgs/OccupancyGrid.
    * [Pull request 138](https://github.com/gazebosim/gz-msgs/pull/138)

### Gazebo Msgs 6.3.0 (2021-02-17)

1. Particle emitter message.
    * [Pull request 127](https://github.com/gazebosim/gz-msgs/pull/127)

1. Add std:: to strings in Generator.cc/hh
    * [Pull request 129](https://github.com/gazebosim/gz-msgs/pull/129)

1. Add windows installation; move installation in README to tutorial
    * [Pull request 126](https://github.com/gazebosim/gz-msgs/pull/126)

1. Includes changes found in version 5.6.0.

### Gazebo Msgs 6.2.0 (2020-12-22)

1. Make ignstrtok private to Utility.cc
    * [Pull request 107](https://github.com/gazebosim/gz-msgs/pull/107)

1. Added JointTrajectory message
    * [Pull request 106](https://github.com/gazebosim/gz-msgs/pull/106)

1. Added Statistics message
    * [Pull request 110](https://github.com/gazebosim/gz-msgs/pull/110)

### Gazebo Msgs 6.1.0 (2020-12-01)

1. Includes changes found in version 5.4.0.

### Gazebo Msgs 6.0.0 (2020-09-28)

1. Convert functions between duration and gz::time.
    * [Pull request 85](https://github.com/gazebosim/gz-msgs/pull/85)

1. Added utility functions to convert between msg::time and std::chrono time_point.
    * [Pull request 81](https://github.com/gazebosim/gz-msgs/pull/81)

1. Update codeowners.
    * [Pull request 80](https://github.com/gazebosim/gz-msgs/pull/80)

1. Included autogenerated code to the gz_create_docs function.
    * [Pull request 76](https://github.com/gazebosim/gz-msgs/pull/76)

1. Feature/add visual arrow and axis.
    * [Pull request 62](https://github.com/gazebosim/gz-msgs/pull/62)

1. Add Cone Geometry Type to marker.proto.
    * [Pull request 61](https://github.com/gazebosim/gz-msgs/pull/61)

1. Use `/bigobj` in MSVC to support protobuf 3.12 library.
    * [Pull request 57](https://github.com/gazebosim/gz-msgs/pull/57)

1. Workflow updates.
    * [Pull request 51](https://github.com/gazebosim/gz-msgs/pull/51)

1. Add Cone Geometry Message.
    * [Pull request 47](https://github.com/gazebosim/gz-msgs/pull/47)

1. Add .gitignore.
    * [Pull request 44](https://github.com/gazebosim/gz-msgs/pull/44)

1. Update BitBucket links.
    * [Pull request 41](https://github.com/gazebosim/gz-msgs/pull/41)

## Gazebo Msgs 5.x

### Gazebo Msgs 5.11.0 (2022-10-10)

1. Ignition to Gazebo renaming.
    * [Pull request #282](https://github.com/gazebosim/gz-msgs/pull/282)
    * [Pull request #293](https://github.com/gazebosim/gz-msgs/pull/293)
    * [Pull request #300](https://github.com/gazebosim/gz-msgs/pull/300)

1. Remove mistaken VERSION.
    * [Pull request #284](https://github.com/gazebosim/gz-msgs/pull/284)

1. Increase utility coverage.
    * [Pull request #283](https://github.com/gazebosim/gz-msgs/pull/283)

### Gazebo Msgs 5.10.0 (2022-07-18)

1. Add entity wrench message
    * [Pull request #274](https://github.com/gazebosim/gz-msgs/pull/274)

1. Ignition -> Gazebo
    * [Pull request #273](https://github.com/gazebosim/gz-msgs/pull/273)

1. Add ACTOR and WORLD types to entity
    * [Pull request #275](https://github.com/gazebosim/gz-msgs/pull/275)

1. Bash completion for flags
    * [Pull request #254](https://github.com/gazebosim/gz-msgs/pull/254)

1. Remove dead files
    * [Pull request #255](https://github.com/gazebosim/gz-msgs/pull/255)

1. Fix typo REGRESION/REGRESSION
    * [Pull request #247](https://github.com/gazebosim/gz-msgs/pull/247)

1. Generator: Add missing std namespace to string arguments
    * [Pull request #242](https://github.com/gazebosim/gz-msgs/pull/242)

### Gazebo Msgs 5.9.0 (2022-03-16)

1. Backport NavSat message for ROS bridge compatibility
    * [Pull request #231](https://github.com/gazebosim/gz-msgs/pull/231)

1. Focal CI: static checkers and doxygen linters
    * [Pull request #230](https://github.com/gazebosim/gz-msgs/pull/230)

1. Add PointCloudPacked iterators
    * [Pull request #210](https://github.com/gazebosim/gz-msgs/pull/210)
    * [Pull request #218](https://github.com/gazebosim/gz-msgs/pull/218)

### Gazebo Msgs 5.8.1 (2021-10-29)

1. Utility\_TEST: adjust expectations for 32-bit arch (Backport of #120)
    * [Pull request #120](https://github.com/gazebosim/gz-msgs/pull/120)
    * [Pull request #196](https://github.com/gazebosim/gz-msgs/pull/196)

1. Fix trivial typo in command line help message
    * [Pull request #191](https://github.com/gazebosim/gz-msgs/pull/191)

1. 🥳 Update gz-tools issue on README
    * [Pull request #184](https://github.com/gazebosim/gz-msgs/pull/184)

### Gazebo Msgs 5.8.0 (2021-08-06)

1. Adds PerformanceSensorMetrics proto message.
    * [Pull request #172](https://github.com/gazebosim/gz-msgs/pull/172)

1. Detect gz instead of using cmake module to check for gz-tools
    * [Pull request #166](https://github.com/gazebosim/gz-msgs/pull/166)

1. Remove tools/code\_check and update codecov
    * [Pull request #164](https://github.com/gazebosim/gz-msgs/pull/164)

### Gazebo Msgs 5.7.0 (2021-03-17)

1. Add gz version of nav\_msgs/OccupancyGrid (backport #138)
    * [Pull request 143](https://github.com/gazebosim/gz-msgs/pull/143)
    * [Pull request 143](https://github.com/gazebosim/gz-msgs/pull/138)

1. Master branch updates
    * [Pull request 141](https://github.com/gazebosim/gz-msgs/pull/141)

1. Add windows installation; move installation in README to tutorial
    * [Pull request 126](https://github.com/gazebosim/gz-msgs/pull/126)

### Gazebo Msgs 5.6.0 (2020-12-28)

1. Add JointTrajectory message.
    * [Pull request 106](https://github.com/gazebosim/gz-msgs/pull/106)
    * [Pull request 117](https://github.com/gazebosim/gz-msgs/pull/117)

### Gazebo Msgs 5.5.0 (2020-12-21)

1. Add statistics message.
    * [Pull request 110](https://github.com/gazebosim/gz-msgs/pull/110)

### Gazebo Msgs 5.4.0 (2020-11-12)

1. Add CMake options to support cross-compilation.
    * [Pull request 60](https://github.com/gazebosim/gz-msgs/pull/60)

1. Add Focal CI.
    * [Pull request 73](https://github.com/gazebosim/gz-msgs/pull/73)

1. Include autogenerated code for the gz\_create\_docs function.
    * [Pull request 76](https://github.com/gazebosim/gz-msgs/pull/76)
    * [Pull request 78](https://github.com/gazebosim/gz-msgs/pull/78)
    * [Pull request 79](https://github.com/gazebosim/gz-msgs/pull/79)

1. Use modern CMake targets when including gz-math.
    * [Pull request 82](https://github.com/gazebosim/gz-msgs/pull/82)

1. Bump versions in readme.
    * [Pull request 88](https://github.com/gazebosim/gz-msgs/pull/88)

1. Bump readme math version.
    * [Pull request 90](https://github.com/gazebosim/gz-msgs/pull/90)

1. Improve fork experience.
    * [Pull request 94](https://github.com/gazebosim/gz-msgs/pull/94)

1. Add dependency conversion for model.config's.
    * [Pull request 91](https://github.com/gazebosim/gz-msgs/pull/91)

1. Resolved updated codecheck issues.
    * [Pull request 96](https://github.com/gazebosim/gz-msgs/pull/96)

### Gazebo Msgs 5.3.0 (2020-05-20)

1. Use modern CMake targets when including gz-math.
    * [Pull request 82](https://github.com/gazebosim/gz-msgs/pull/82)

1. Included autogenerated code to the gz_create_docs function.
    * [Pull request 78](https://github.com/gazebosim/gz-msgs/pull/78)

1. Support multiple messages in a proto file.
    * [Pull request 70](https://github.com/gazebosim/gz-msgs/pull/70)

1. Add CMake options to support cross-compilation.
    * [Pull request 60](https://github.com/gazebosim/gz-msgs/pull/60)

1. Added the `EntityFactory_V` message.
    * [Pull request 58](https://github.com/gazebosim/gz-msgs/pull/58)

### Gazebo Msgs 5.2.0 (2020-05-14)

1. Handle empty xml elements
    * [BitBucket pull request 172](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/172)

1. Ignore deprecation warnings on generated code
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/170)

1. Add SdfGeneratorConfig message containing configuration options for generating SDFormat from currently loaded worlds
    * [BitBucket pull request 174](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/174)

1. BitBucket to GitHub migration
    * [Pull request](https://github.com/gazebosim/gz-msgs/pull/39)
    * [Pull request](https://github.com/gazebosim/gz-msgs/pull/40)
    * [Pull request](https://github.com/gazebosim/gz-msgs/pull/43)

### Gazebo Msgs 5.1.0

1. Add ConvertPixelFormatType functions to Utility.hh
    * [BitBucket pull request 169](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/169)

### Gazebo Msgs 5.0.0 (2019-12-09)

1. Add support for sdformat frame semantics
    * [BitBucket pull request 160](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/160)

1. Add emissive_map field to material.proto
    * [BitBucket pull request 155](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/155)

1. Add discovery message
    * [BitBucket pull request 159](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/159)

1. Deprecate all double fields in joint_cmd and pid, replace with Double
    * [BitBucket pull request 162](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/162)

1. Factory relative to
    * [BitBucket pull request 163](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/163)

1. Added `fuel_metadata.proto`, `version.proto`, `version_range.proto`, and `versioned_name.proto` message files. Added conversion functions to/from the model.config/metadata.pbtxt formats.
    * [BitBucket pull request 157](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/157)


## Gazebo Msgs 4.x

### Gazebo Msgs 4.9.0 (2020-05-20)

1. Included autogenerated code to the gz_create_docs function.
    * [Pull request 79](https://github.com/gazebosim/gz-msgs/pull/79)

1. Add Focal CI.
    * [Pull request 73](https://github.com/gazebosim/gz-msgs/pull/73)

1. Added the `EntityFactory_V` message.
    * [Pull request 58](https://github.com/gazebosim/gz-msgs/pull/58)

### Gazebo Msgs 4.8.0 (2020-04-09)

1. Add SdfGeneratorConfig message containing configuration options for generating SDFormat from currently loaded worlds
    * [BitBucket pull request 174](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/174)

1. Fix linking order of Protocol Buffers static libraries on GCC
    * [BitBucket pull request 151](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/151)

1. Ignore deprecation warnings on generated code
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/170)

### Gazebo Msgs 4.7.0 (2019-08-22)

1. Add video record message
    * [BitBucket pull request 149](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/149)

### Gazebo Msgs 4.6.0 (2019-08-16)

1. Add Actor message
    * [BitBucket pull request 147](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/147)

### Gazebo Msgs 4.5.0 (2019-07-19)

1. Add battery state message
    * [BitBucket pull request 145](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/145)

### Gazebo Msgs 4.4.0 (2019-07-16)

1. Added SerializedEntityMap, SerializedStateMap, and SerializedStepMap
   messages which match SerializedEntity, SerializedState, and SerializedStep messages except that the former uses `map` data structures.
    * [BitBucket pull request 143](https://github.com/gazebosim/gz-common/pull-requests/143)

### Gazebo Msgs 4.2.0

1. Added odometry message
    * [BitBucket pull request 137](https://github.com/gazebosim/gz-common/pull-requests/137)

### Gazebo Msgs 4.1.0

1. Added camera info message
    * [BitBucket pull request 136](https://github.com/gazebosim/gz-common/pull-requests/136)

### Gazebo Msgs 4.0.0 (2019-05-20)

1. Add world stats to packet message.
    * [BitBucket pull request 125](https://github.com/gazebosim/gz-common/pull-requests/125)

1. Deprecating `raysensor.proto`, and replacing with `lidar_sensor.proto`.
    * [BitBucket pull request 124](https://github.com/gazebosim/gz-common/pull-requests/124)

1. Added orientation reference frame information to `imu_sensor.proto`.
    * [BitBucket pull request 121](https://github.com/gazebosim/gz-common/pull-requests/121)

1. New actuators `actuators.proto` message.
    * [BitBucket pull request 123](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/123)
    * [BitBucket pull request 129](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/129)

1. Added Physically Based Rendering (PBR) message to `material.proto`
    * [BitBucket pull request 120](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/120)

1. Corrected `BAYER_RGGR8` to `BAYER_BGGR8` in `PixelFormatType` located in
`proto/gz/msgs/image.proto`.
    * [BitBucket pull request 118](https://github.com/gazebosim/gz-common/pull-requests/118)

1. Adding `dynamic_bias_stddev` and `dynamic_bias_correlation_time` sensor noise parameters.
    * [BitBucket pull request 117](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/117)

1. Added `fluid_pressure.proto` and `air_pressure_sensor.proto`.
    * [BitBucket pull request 116](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/116)

1. Added `step_size` to `WorldStatistics` and `SerializedStep`
    * [BitBucket pull request 122](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/122)

1. Added an `altimeter_sensor.proto` message that encapsulates information
   about an altimeter sensor.
    * [BitBucket pull request 114](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/114)

1. Added a `magnetometer_sensor.proto` message that encapsulates information
   about  a magnetometer sensor.
    * [BitBucket pull request 113](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/113)

1. image: `pixel_format` field is deprecated, use `pixel_format_type` with enum `PixelFormatType` instead.
    * [BitBucket pull request 106](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/106)
    * [BitBucket pull request 95](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/95)

1. Updated `model.proto` with more documentation. Position, velocity, force,
   and acceleration information for joint have been moved to the `axis.proto`
   message, see the Migration guide. The `gz.hh` header file is no longer
   installed.
    * [BitBucket pull request 104](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/104)

1. Change serialized field from string to bytes.
    * [BitBucket pull request 102](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/102)

1. Install ruby files into versioned folder
    * [BitBucket pull request 109](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/109)
    * [Issue 24](https://github.com/gazebosim/gz-msgs/issues/24)
    * [Issue 27](https://github.com/gazebosim/gz-msgs/issues/27)

## Gazebo Msgs 3.x

### Gazebo Msgs 3.x.x

1. Install ruby files into versioned folder
    * [BitBucket pull request 111](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/111)
    * [Issue 24](https://github.com/gazebosim/gz-msgs/issues/24)
    * [Issue 27](https://github.com/gazebosim/gz-msgs/issues/27)

1. Fix linking order of Protocol Buffers static libraries on GCC
    * [BitBucket pull request 151](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/151)

### Gazebo Msgs 3.2.0

1. Added Pose_V, Time, and Clock fields to packet.proto
    * [BitBucket pull request 108](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/108)

### Gazebo Msgs 3.1.0

1. Added serialized messages: SerializedState, SerializedEntity, SerializedComponent
    * [BitBucket pull request 99](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/99)

1. image.proto: add `PixelFormatType` enum, copied from `gz::common::Image::PixelFormatType`
    * [BitBucket pull request 97](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/97)


### Gazebo Msgs 3.0.0

1. axis\_aligned\_box: new message and conversion functions
    * [BitBucket pull request 91](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/91)

1. contacts: use Entity instead of string to specify entities in contact
    * [BitBucket pull request 90](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/90)

1. entity: new message to uniquely identify an entity
    * [BitBucket pull request 87](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/87)

1. entity\_factory: refactor message, remove edit\_name field
    * [BitBucket pull request 86](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/86)
    * [Issue 25](https://github.com/gazebosim/gz-msgs/issues/25)

1. Add twist message
    * [BitBucket pull request 83](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/83)

1. Update light msgs fields
    * [BitBucket pull request 78](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/78)

1. Combine docs
    * [BitBucket pull request 75](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/75)

1. configure.bat: don't need to list recursive dependencies
    * [BitBucket pull request 74](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/74)

1. Update gtest
    * [BitBucket pull request 73](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/73)

1. Upgrade to protobuf3
    * [BitBucket pull request 69](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/69)

1. Fix ruby message generation.
    * [BitBucket pull request 47](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/47)


## Gazebo Msgs 2.x

### Gazebo Msgs 2.x.x

1. Fix linking order of Protocol Buffers static libraries on GCC
    * [BitBucket pull request 151](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/151)

### Gazebo Msgs 2.1.0

1. Upgrade to gz-math6
    * [BitBucket pull request 138](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/138)


### Gazebo Msgs 2.0.0

1. sonar: add geometry field
    * [BitBucket pull request 82](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/82)

1. log\_control, log\_status: add record\_resources bool to match osrf/gazebo PR 3008
    * [BitBucket pull request 76](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/76)
    * [osrf/gazebo Pull request 3008](https://osrf-migration.github.io/gazebo-gh-pages/#!/osrf/gazebo/pull-requests/3008)

1. Added inline versioned namespace. Generator.hh is no longer installed
    * [BitBucket pull request 59](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/59)

1. Upgrade to gz-cmake1
    * [BitBucket pull request 55](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/55)

1. Extend Factory class
    * [BitBucket pull request 61](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/61)
    * [BitBucket pull request 66](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/66)

1. Add real\_time\_factor field to WorldStatistics
    * [BitBucket pull request 67](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-msgs/pull-requests/67)
