## Gazebo Launch 8.x

### Gazebo Launch 8.0.2 (2025-10-08)

1. Fix compatibility with protobuf v30 (cpp 6.30.0)
    * [Pull request #299](https://github.com/gazebosim/gz-launch/pull/299)

### Gazebo Launch 8.0.1 (2025-02-12)

1. Fix websocket proto request handling
    * [Pull request #281](https://github.com/gazebosim/gz-launch/pull/281)

### Gazebo Launch 8.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 7.1.0 and earlier.

1. Fix crash and update launch file
    * [Pull request #277](https://github.com/gazebosim/gz-launch/pull/277)

1. Miscellaneous documentation fixes
    * [Pull request #276](https://github.com/gazebosim/gz-launch/pull/276)
    * [Pull request #275](https://github.com/gazebosim/gz-launch/pull/275)
    * [Pull request #274](https://github.com/gazebosim/gz-launch/pull/274)

1. Enable ubuntu noble on github actions, require cmake 3.22.1
    * [Pull request #272](https://github.com/gazebosim/gz-launch/pull/272)

1. Remove deprecations
    * [Pull request #271](https://github.com/gazebosim/gz-launch/pull/271)

1. Fix libwebsocket shutdown behavior
    * [Pull request #267](https://github.com/gazebosim/gz-launch/pull/267)

1. Fix msvc loss of data warning
    * [Pull request #266](https://github.com/gazebosim/gz-launch/pull/266)

1. Remove indirect dependencies
    * [Pull request #265](https://github.com/gazebosim/gz-launch/pull/265)

1. Supress msvc dll exported interface warning
    * [Pull request #263](https://github.com/gazebosim/gz-launch/pull/263)

1. Fix windows warnings
    * [Pull request #199](https://github.com/gazebosim/gz-launch/pull/199)
    * [Pull request #250](https://github.com/gazebosim/gz-launch/pull/250)

1. Disable failing Manager_Test on windows
    * [Pull request #264](https://github.com/gazebosim/gz-launch/pull/264)

1. Disable failing windows tests
    * [Pull request #255](https://github.com/gazebosim/gz-launch/pull/255)

1. Add package.xml, remove dependency on python3-yaml, and disable failing windows tests
    * [Pull request #249](https://github.com/gazebosim/gz-launch/pull/249)

1. Define GZ_LAUNCH_VERSION_NAMESPACE in config.hh
    * [Pull request #247](https://github.com/gazebosim/gz-launch/pull/247)

1. Bumps in Ionic: gz-launch8
    * [Pull request #236](https://github.com/gazebosim/gz-launch/pull/236)
    * [Pull request #237](https://github.com/gazebosim/gz-launch/pull/237)

## Gazebo Launch 7.x

### Gazebo Launch 7.1.0 (2024-04-11)

1. Use relative install paths for plugin shared libraries and gz-tools data
    * [Pull request #253](https://github.com/gazebosim/gz-launch/pull/253)

1. Fix bug where address of local variable was returned
    * [Pull request #252](https://github.com/gazebosim/gz-launch/pull/252)

1. Add optional binary relocatability
    * [Pull request #218](https://github.com/gazebosim/gz-launch/pull/218)

1. Fix windows warnings
    * [Pull request #199](https://github.com/gazebosim/gz-launch/pull/199)
    * [Pull request #250](https://github.com/gazebosim/gz-launch/pull/250)

1. Update CI badges in README
    * [Pull request #239](https://github.com/gazebosim/gz-launch/pull/239)

1. Infrastructure
    * [Pull request #238](https://github.com/gazebosim/gz-launch/pull/238)

### Gazebo Launch 7.0.0 (2023-09-29)

1. Documentation fixes
    * [Pull request #230](https://github.com/gazebosim/gz-launch/pull/230)

1. Infrastructure
    * [Pull request #229](https://github.com/gazebosim/gz-launch/pull/229)

1. ign -> gz
    * [Pull request #223](https://github.com/gazebosim/gz-launch/pull/223)

1. Bump Harmonic dependencies: fuel-tools, gui, physics, rendering, sdformat , sensors, sim, transport, and msgs
    * [Pull request #210](https://github.com/gazebosim/gz-launch/pull/210)
    * [Pull request #224](https://github.com/gazebosim/gz-launch/pull/224)
    * [Pull request #207](https://github.com/gazebosim/gz-launch/pull/207)

1. ⬆️  Bump main to 7.0.0~pre1
    * [Pull request #189](https://github.com/gazebosim/gz-launch/pull/189)

## Gazebo Launch 6.x

### Gazebo Launch 6.1.0 (2023-09-26)

1. Infrastructure
    * [Pull request #227](https://github.com/gazebosim/gz-launch/pull/227)
    * [Pull request #226](https://github.com/gazebosim/gz-launch/pull/226)
    * [Pull request #213](https://github.com/gazebosim/gz-launch/pull/213)

1. Rename COPYING to LICENSE
    * [Pull request #212](https://github.com/gazebosim/gz-launch/pull/212)

1. Small cleanup fixes
    * [Pull request #211](https://github.com/gazebosim/gz-launch/pull/211)

1. Add pause and stop to Websocket Server
    * [Pull request #187](https://github.com/gazebosim/gz-launch/pull/187)

1. Fix gz_test when using a CMake generator different from make
    * [Pull request #204](https://github.com/gazebosim/gz-launch/pull/204)
    * [Pull request #205](https://github.com/gazebosim/gz-launch/pull/205)

1. Return a message on asset error
    * [Pull request #197](https://github.com/gazebosim/gz-launch/pull/197)

1. Remove redundant namespace references
    * [Pull request #190](https://github.com/gazebosim/gz-launch/pull/190)

### Gazebo Launch 6.0.0

1. Fix macOs compiler error.
    * [Pull request #161](https://github.com/gazebosim/gz-launch/pull/161)

1. Update to latest gtest.
    * [Pull request #174](https://github.com/gazebosim/gz-launch/pull/174)

1. Ignition to Gazebo renaming
    * [Pull request #162](https://github.com/gazebosim/gz-launch/pull/162)
    * [Pull request #163](https://github.com/gazebosim/gz-launch/pull/163)
    * [Pull request #165](https://github.com/gazebosim/gz-launch/pull/165)
    * [Pull request #166](https://github.com/gazebosim/gz-launch/pull/166)
    * [Pull request #168](https://github.com/gazebosim/gz-launch/pull/168)
    * [Pull request #169](https://github.com/gazebosim/gz-launch/pull/169)
    * [Pull request #170](https://github.com/gazebosim/gz-launch/pull/170)
    * [Pull request #171](https://github.com/gazebosim/gz-launch/pull/171)
    * [Pull request #172](https://github.com/gazebosim/gz-launch/pull/172)
    * [Pull request #173](https://github.com/gazebosim/gz-launch/pull/173)
    * [Pull request #176](https://github.com/gazebosim/gz-launch/pull/176)
    * [Pull request #188](https://github.com/gazebosim/gz-launch/pull/188)

1. Fix `msgs` header usage.
    * [Pull request #196](https://github.com/gazebosim/gz-launch/pull/196)

1. Version bumps and removal of deprecations
    * [Pull request #144](https://github.com/gazebosim/gz-launch/pull/144)
    * [Pull request #145](https://github.com/gazebosim/gz-launch/pull/145)
    * [Pull request #150](https://github.com/gazebosim/gz-launch/pull/150)
    * [Pull request #156](https://github.com/gazebosim/gz-launch/pull/156)

## Gazebo Launch 5.x

### Gazebo Launch 5.3.0 (2023-06-14)

1. Forward ports
    * [Pull request #214](https://github.com/gazebosim/gz-launch/pull/214)

1. Infrastructure
    * [Pull request #213](https://github.com/gazebosim/gz-launch/pull/213)

1. Rename COPYING to LICENSE
    * [Pull request #212](https://github.com/gazebosim/gz-launch/pull/212)

1. Small cleanup fixes
    * [Pull request #211](https://github.com/gazebosim/gz-launch/pull/211)

1. Add pause and stop to Websocket Server
    * [Pull request #187](https://github.com/gazebosim/gz-launch/pull/187)

1. Return a message on asset error
    * [Pull request #197](https://github.com/gazebosim/gz-launch/pull/197)

1. Remove redundant namespace references
    * [Pull request #190](https://github.com/gazebosim/gz-launch/pull/190)

### Gazebo Launch 5.2.0 (2022-08-16)

1. Add code coverage ignore file
    * [Pull request #179](https://github.com/gazebosim/gz-launch/pull/179)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #181](https://github.com/gazebosim/gz-launch/pull/181)
    * [Pull request #182](https://github.com/gazebosim/gz-launch/pull/182)

1. fix `ign_TEST` for Fortress
    * [Pull request #180](https://github.com/gazebosim/gz-launch/pull/180)

1. Ignition -> Gazebo
    * [Pull request #178](https://github.com/gazebosim/gz-launch/pull/178)

1. Bash completion for flags
    * [Pull request #167](https://github.com/gazebosim/gz-launch/pull/167)

1. Adds ability to get a file from a running Gazebo instance
    * [Pull request #164](https://github.com/gazebosim/gz-launch/pull/164)

1. Add Ubuntu Jammy CI
    * [Pull request #154](https://github.com/gazebosim/gz-launch/pull/154)

1. Depend on `python3-yaml` instead of `python-yaml`
    * [Pull request #153](https://github.com/gazebosim/gz-launch/pull/153)

## Gazebo Launch 5.x


### Gazebo Launch 5.1.0 (2022-03-21)
1. Use exec instead of popen to run gz-launch binary
    * [Pull request #151](https://github.com/gazebosim/gz-launch/pull/151)

1. Expose headless_rendering sdf tag
    * [Pull request #148](https://github.com/gazebosim/gz-launch/pull/148)

1. Remove spammy CMake message
    * [Pull request #142](https://github.com/gazebosim/gz-launch/pull/142)

### Gazebo Launch 5.0.0 (2021-10-01)

1. Port gz-launch to Windows
    * [Pull request #120](https://github.com/gazebosim/gz-launch/pull/120)

1. Add standalone executables
    * [Pull request #121](https://github.com/gazebosim/gz-launch/pull/121)
    * [Pull request #131](https://github.com/gazebosim/gz-launch/pull/131)

1. Remove unused includes (fix build)
    * [Pull request #119](https://github.com/gazebosim/gz-launch/pull/119)

1. Bumps dependencies for fortress
    * [Pull request #110](https://github.com/gazebosim/gz-launch/pull/110)

1. Infrastructure
    * [Pull request #113](https://github.com/gazebosim/gz-launch/pull/113)
    * [Pull request #106](https://github.com/gazebosim/gz-launch/pull/106)

## Gazebo Launch 4.x

### Gazebo Launch 4.X.X (20XX-XX-XX)

### Gazebo Launch 4.1.0 (2021-10-14)

1. All changes included in Gazebo Launch 3.4.2.

### Gazebo Launch 4.0.0 (2021-03-30)

1. Bump in edifice: gz-common4
    * [Pull request #90](https://github.com/gazebosim/gz-launch/pull/90)

1. Bump in edifice: sdformat11
    * [Pull request #87](https://github.com/gazebosim/gz-launch/pull/87)

1. Bump in edifice: gz-msgs7
    * [Pull request #86](https://github.com/gazebosim/gz-launch/pull/86)

1. Bump in edifice: gz-rendering5
    * [Pull request #69](https://github.com/gazebosim/gz-launch/pull/69)

## Gazebo Launch 3.x

### Gazebo Launch 3.4.2 (2021-10-14)

1. All changes included in Gazebo Launch 2.2.2.

### Gazebo Launch 3.4.1 (2021-07-15)

1. Generate a better error websocket error code for `max_connections`.
    * [Pull request 123](https://github.com/gazebosim/gz-launch/pull/123)

### Gazebo Launch 3.4.0 (2021-06-09)

1. Extend websocket server to support message limits and throttling.
    * [Pull request 116](https://github.com/gazebosim/gz-launch/pull/116)

### Gazebo Launch 3.3.0 (2021-05-06)

1. Convert depth and thermal image data to RGB before sending over websockets
    * [Pull request 112](https://github.com/gazebosim/gz-launch/pull/112)

### Gazebo Launch 3.2.1 (2021-04-19)

1. Add header to image msg.
    * [Pull request 109](https://github.com/gazebosim/gz-launch/pull/109)

### Gazebo Launch 3.2.0 (2021-04-12)

1. Support unsubscribing from a topic in the websocket server.
    * [Pull request 107](https://github.com/gazebosim/gz-launch/pull/107)

1. Support particle_emitters in the websocket server.
    * [Pull request 104](https://github.com/gazebosim/gz-launch/pull/104)

1. Support getting topic names and message types in the websocket server.
    * [Pull request 102](https://github.com/gazebosim/gz-launch/pull/102)

1. Image streaming over websocket.
    * [Pull request 97](https://github.com/gazebosim/gz-launch/pull/97)

1. Treat GZ_LAUNCH_CONFIG_PATH as a path list.
    * [Pull request 93](https://github.com/gazebosim/gz-launch/pull/93)


### Gazebo Launch 3.1.1 (2021-01-08)

1. All changes up to and including those in version 2.2.1.

### Gazebo Launch 3.1.0 (2020-12-10)

1. All changes up to and including those in version 2.2.0.

### Gazebo Launch 3.0.0 (2020-09-30)

1. Migration from BitBucket to GitHub
    * [Pull request 13](https://github.com/gazebosim/gz-launch/pull/13)
    * [Pull request 16](https://github.com/gazebosim/gz-launch/pull/16)
    * [Pull request 28](https://github.com/gazebosim/gz-launch/pull/28)

1. Add PKGCONFIG information to gz-tools gz_find_package
    * [Pull request 44](https://github.com/gazebosim/gz-launch/pull/44)

1. Depend on gz-msgs6, gz-transport9
    * [Pull request 39](https://github.com/gazebosim/gz-launch/pull/39)

1. Depend on gz-sim4, gz-gui4
    * [BitBucket pull request 64](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/64)

## Gazebo Launch 2.x

### Gazebo Launch 2.3.1 (2024-01-05)

1. Small cleanup fixes
    * [Pull request #211](https://github.com/gazebosim/gz-launch/pull/211)

1. Infrastructure
    * [Pull request #238](https://github.com/gazebosim/gz-launch/pull/238)
    * [Pull request #213](https://github.com/gazebosim/gz-launch/pull/213)
    * [Pull request #212](https://github.com/gazebosim/gz-launch/pull/212)

### Gazebo Launch 2.3.0 (2022-08-15)

1. Remove redundant namespace references
    * [Pull request #190](https://github.com/gazebosim/gz-launch/pull/190)

1. Add code coverage ignore file
    * [Pull request #179](https://github.com/gazebosim/gz-launch/pull/179)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #181](https://github.com/gazebosim/gz-launch/pull/181)
    * [Pull request #182](https://github.com/gazebosim/gz-launch/pull/182)

1. Ignition -> Gazebo
    * [Pull request #178](https://github.com/gazebosim/gz-launch/pull/178)

1. Bash completion for flags
    * [Pull request #167](https://github.com/gazebosim/gz-launch/pull/167)

### Gazebo Launch 2.2.2 (2021-10-11)

1. Master branch updates.
    * [Pull request 98](https://github.com/gazebosim/gz-launch/pull/98)

1. Treat `GZ_LAUNCH_CONFIG_PATH` as a path list.
    * [Pull request 93](https://github.com/gazebosim/gz-launch/pull/93)

1. Remove tools/code_check and update codecov.
    * [Pull request 115](https://github.com/gazebosim/gz-launch/pull/115)

1. Update gtest for Windows compilation.
    * [Pull request 122](https://github.com/gazebosim/gz-launch/pull/122)

1. Remove bitbucket-pipelines.yml.
    * [Pull request 128](https://github.com/gazebosim/gz-launch/pull/128)

### Gazebo Launch 2.2.1 (2021-01-08)

1.  Fix env parsing by placing it before executable parsing.
    * [Pull request 81](https://github.com/gazebosim/gz-launch/pull/81)
    * [Pull request 82](https://github.com/gazebosim/gz-launch/pull/82)

### Gazebo Launch 2.2.0 (2020-10-14)

1. All changes up to and including those in version 1.10.0

1. Added a tutorial.
    * [Pull request 48](https://github.com/gazebosim/gz-launch/pull/48)

### Gazebo Launch 2.1.0 (2020-05-21)

1. Merged gz-launch1 forward.
    * [Pull request 41](https://github.com/gazebosim/gz-launch/pull/31)

### Gazebo Launch 2.0.0

1. Use Gazebo Citadel dependencies.
    * [BitBucket pull request 47](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/47)
    * [BitBucket pull request 50](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/50)
    * [BitBucket pull request 53](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/53)

1. Allow specifying a custom window title instead of "Gazebo"
    * [BitBucket pull request 31](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/31)

1. Unversioned lib name for cmds
    * [BitBucket pull request 55](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/55)

## Gazebo Launch 1.x

### Gazebo Launch 1.X.X (20XX-XX-XX)

### Gazebo Launch 1.10.1 (2020-12-23)

1. Fix codecheck
   * [Pull Request 67](https://github.com/gazebosim/gz-launch/pull/67)
   * [Pull Request 70](https://github.com/gazebosim/gz-launch/pull/70)

1. Fix race condition in websocket server
   * [Pull Request 68](https://github.com/gazebosim/gz-launch/pull/68)

### Gazebo Launch 1.10.0 (2020-09-25)

* Modernize Github Actions CI.
   * [Pull request 42](https://github.com/gazebosim/gz-launch/pull/42)

1. Add PKGCONFIG information to gz-tools gz_find_package
   * [Pull Request 44](https://github.com/gazebosim/gz-launch/pull/44)

1. Fix factory.gzlaunch launch file
   * [Pull Request 55](https://github.com/gazebosim/gz-launch/pull/55)

1. Use random name for manager semaphore
   * [Pull Request 57](https://github.com/gazebosim/gz-launch/pull/57)

1. Add support for specifying topics to record
   * [Pull Request 54](https://github.com/gazebosim/gz-launch/pull/54)

1. Fix race condition in websocket server.
   * [Pull Request 68](https://github.com/gazebosim/gz-launch/pull/68)

### Gazebo Launch 1.9.0 (2020-08-13)

1. Added HTTP handling support to websocket server and a metrics HTTP endpoint
   to monitor websocket server status.
   * [Pull Request 49](https://github.com/gazebosim/gz-launch/pull/49)

### Gazebo Launch 1.8.0 (2020-07-28)

1. Added `<max_connections>` to the websocket server that supports
   specifying the maximum allowed websocket connections.
    * [Pull Request 40](https://github.com/gazebosim/gz-launch/pull/40)

### Gazebo Launch 1.7.1 (2020-06-23)

1. Improve websocket performance by throttling the busy loop, and fix empty SSL
   XML elements.
    * [Pull Request 37](https://github.com/gazebosim/gz-launch/pull/37)

### Gazebo Launch 1.7.0 (2020-06-16)

1. Added SSL to websocket server.
    * [Pull Request 34](https://github.com/gazebosim/gz-launch/pull/34)

### Gazebo Launch 1.6.0 (2020-06-11)

1. Improved websockets by: adding simple authentication, access to
   protobuf message definitions, access to scene and world information, and
   definition of custom message framing.
    * [Pull Request 22](https://github.com/gazebosim/gz-launch/pull/22)
    * [Pull Request 21](https://github.com/gazebosim/gz-launch/pull/21)
    * [Pull Request 17](https://github.com/gazebosim/gz-launch/pull/17)
    * [Pull Request 33](https://github.com/gazebosim/gz-launch/pull/33)

### Gazebo Launch 1.5.0 (2020-05-20)

1. Added support for spawning multiple entities in the same simulation step.
    * [Pull Request 30](https://github.com/gazebosim/gz-launch/pull/30)

### Gazebo Launch 1.4.2 (2020-05-18)

1.  Use the new GUI API of gz-sim. This adds support for saving worlds to SDFormat from the GUI.
    * [Pull Request 19](https://github.com/gazebosim/gz-launch/pull/19)

### Gazebo Launch 1.4.1 (2019-12-05)

1. Unversioned lib name for cmds
    * [BitBucket pull request 56](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/56)

### Gazebo Launch 1.4.0 (2019-11-26)

1. Default GUI config for Launch
    * [BitBucket pull request 51](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/51)

### Gazebo Launch 1.3.0 (2019-11-13)

1. Add filepath to ERB so that constants like `__FILE__` in `.gzlaunch` files work as expected
    * [BitBucket pull request 48](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/48)

1.  Add backward support to gz-launch to capture backtraces.
    * [BitBucket pull request 41](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/41)

### Gazebo Launch 1.2.3 (2019-09-09)

1. Add QML import path to IgnGazebo modules.
    * [BitBucket pull request 42](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/42)

### Gazebo Launch 1.2.2

1. Add support for console logging.
    * [BitBucket pull request 40](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/40)

1. Support setting custom window icons.
    * [BitBucket pull request 39](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/39)

### Gazebo Launch 1.2.1

1. Eliminate potential deadlock from SIGCHLD signal handler
    * [BitBucket pull request 36](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/36)

### Gazebo Launch 1.2.0

1. Support for custom random seed in the SimServer plugin.
    * [BitBucket pull request 33](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/33)

1. Allow specifying a custom window title
    * [BitBucket pull request 32](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/32)

### Gazebo Launch 1.1.0 (2019-06-11)

1. Added command line parameters of the form <name>:=<value>. These parameters are passed to ERB.
    * [BitBucket pull request 27](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/27)

### Gazebo Launch 1.0.1 (2019-05-22)

1. Fix SimFactory set performer topic
    * [BitBucket pull request 26](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/26)

### Gazebo Launch 1.0.0 (2019-05-21)

1. Enable logging with `<record>` tag in gz launch file.
    * [BitBucket pull request 23](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/23)

1. Fix gazebo gui to use GuiRunner.
    * [BitBucket pull request 22](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/22)

1. Fix parsing positional argument to gz tool.
    * [BitBucket pull request 21](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/21)

1. Added world stats to Websocket server.
    * [BitBucket pull request 20](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/20)

1. Change project name to ignition-launch1.
    * [BitBucket pull request 19](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/19)

1. Depend on gazebo2, gui2, msgs4, transport7.
    * [BitBucket pull request 17](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/17)

1. Added Websocket server.
    * [BitBucket pull request 15](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/15)
    * [BitBucket pull request 18](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/18)

## Gazebo Launch 0.x

1. Install examples folder.
    * [BitBucket pull request 14](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-launch/pull-requests/14)

### Gazebo Launch 0.2.0
