## Gazebo Transport 14.X

### Gazebo Transport 14.2.0 (2025-10-21)

1. Fix deadlock in service requests when running local callbacks
    * [Pull request #671](https://github.com/gazebosim/gz-transport/pull/671)

1. Initialize move constructor
    * [Pull request #718](https://github.com/gazebosim/gz-transport/pull/718)

1. Set GZ_IP for gz_src_TEST
    * [Pull request #723](https://github.com/gazebosim/gz-transport/pull/723)

1. Add null-check for `gz param` parameters in three functions to prevent crashes
    * [Pull request #709](https://github.com/gazebosim/gz-transport/pull/709)

1. Fix small typo in services tutorial
    * [Pull request #703](https://github.com/gazebosim/gz-transport/pull/703)

1. Replace DebugString() with PrintToString()
    * [Pull request #685](https://github.com/gazebosim/gz-transport/pull/685)

1. Fix compatibility with protobuf v30 (cpp 6.30.0) regression
    * [Pull request #672](https://github.com/gazebosim/gz-transport/pull/672)

1. BugFix :  Failure to catch after throwing an error causes a crash
    * [Pull request #662](https://github.com/gazebosim/gz-transport/pull/662)

1. Clean up namespaces
    * [Pull request #653](https://github.com/gazebosim/gz-transport/pull/653)
    * [Pull request #649](https://github.com/gazebosim/gz-transport/pull/649)
    * [Pull request #642](https://github.com/gazebosim/gz-transport/pull/642)
    * [Pull request #637](https://github.com/gazebosim/gz-transport/pull/637)

1. [Bazel] Bump libzmq dep to 4.3.5.bcr.3
    * [Pull request #636](https://github.com/gazebosim/gz-transport/pull/636)

1. Add move constructor / assignment operator to Node::Subscriber
    * [Pull request #625](https://github.com/gazebosim/gz-transport/pull/625)

1. Allow programmatic configuration of unicast relays.
    * [Pull request #498](https://github.com/gazebosim/gz-transport/pull/498)

1. Fix compatibility with protobuf v30 (cpp 6.30.0)
    * [Pull request #615](https://github.com/gazebosim/gz-transport/pull/615)

1. Cast nanoseconds to int64_t
    * [Pull request #607](https://github.com/gazebosim/gz-transport/pull/607)

1. Support cmake 4.0
    * [Pull request #603](https://github.com/gazebosim/gz-transport/pull/603)

1. Set GZ_IP=127.0.0.1 in tests needing network
    * [Pull request #595](https://github.com/gazebosim/gz-transport/pull/595)

1. Remove subscribers after publisher ends
    * [Pull request #587](https://github.com/gazebosim/gz-transport/pull/587)

1. Remove unused MessagePublisherPrivate forward declaration.
    * [Pull request #570](https://github.com/gazebosim/gz-transport/pull/570)

1. Code cleanup
    * [Pull request #578](https://github.com/gazebosim/gz-transport/pull/578)

1. Update maintainer email
    * [Pull request #579](https://github.com/gazebosim/gz-transport/pull/79)

1. Unhide cmake warning when Python3 Development is not found
    * [Pull request #565](https://github.com/gazebosim/gz-transport/pull/565)

1. Add compatibility with protobuf 28
    * [Pull request #541](https://github.com/gazebosim/gz-transport/pull/541)

### Gazebo Transport 14.1.0 (2025-05-13)

1. Migrate bazel build setup to use bzlmod
    * [Pull request #628](https://github.com/gazebosim/gz-transport/pull/628)
    * [Pull request #629](https://github.com/gazebosim/gz-transport/pull/629)
    * [Pull request #631](https://github.com/gazebosim/gz-transport/pull/631)

1. Adds new Node::CreateSubscriber API
    * [Pull request #608](https://github.com/gazebosim/gz-transport/pull/608)
    * [Pull request #625](https://github.com/gazebosim/gz-transport/pull/625)

1. Fix compatibility with protobuf v30 (cpp 6.30.0)
    * [Pull request #618](https://github.com/gazebosim/gz-transport/pull/618)

1. ci: run cppcheck, cpplint, doxygen on noble
    * [Pull request #617](https://github.com/gazebosim/gz-transport/pull/617)

1. Cast nanoseconds to int64_t (#607)
    * [Pull request #611](https://github.com/gazebosim/gz-transport/pull/611)

1. Remove duplicate cmake_minimum_required
    * [Pull request #606](https://github.com/gazebosim/gz-transport/pull/606)

1. Support cmake 4.0
    * [Pull request #603](https://github.com/gazebosim/gz-transport/pull/603)

1. Set GZ_IP=127.0.0.1 in tests needing network
    * [Pull request #595](https://github.com/gazebosim/gz-transport/pull/595)

1. Remove subscribers after publisher ends
    * [Pull request #587](https://github.com/gazebosim/gz-transport/pull/587)

1. Code cleanup (backport #578)
    * [Pull request #583](https://github.com/gazebosim/gz-transport/pull/583)

1. Update maintainer email (#579)
    * [Pull request #580](https://github.com/gazebosim/gz-transport/pull/#580)

### Gazebo Transport 14.0.1 (2025-02-12)

1. Remove unused MessagePublisherPrivate forward declaration. (#570)
    * [Pull request #571](https://github.com/gazebosim/gz-transport/pull/571)

1. Unhide cmake warning when Python3 Development is not found
    * [Pull request #565](https://github.com/gazebosim/gz-transport/pull/565)

1. Permit building python bindings separately from main library
    * [Pull request #554](https://github.com/gazebosim/gz-transport/pull/554)

1. IP_RELAY -> GZ_RELAY
    * [Pull request #558](https://github.com/gazebosim/gz-transport/pull/558)

1. python bindings: get version from package.xml
    * [Pull request #556](https://github.com/gazebosim/gz-transport/pull/556)

1. Permit building python bindings separately from main library
    * [Pull request #554](https://github.com/gazebosim/gz-transport/pull/554)

1. Add compatibility with protobuf 28
    * [Pull request #541](https://github.com/gazebosim/gz-transport/pull/541)

1. Using Pathname to check for absolute path
    * [Pull request #550](https://github.com/gazebosim/gz-transport/pull/550)

1. Disable playback.ReplayStep for windows
    * [Pull request #517](https://github.com/gazebosim/gz-transport/pull/517)

1. Added Prerequisites section in python tutorial
    * [Pull request #516](https://github.com/gazebosim/gz-transport/pull/516)

### Gazebo Transport 14.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 13.4.0 and earlier.

1. Miscellaneous documentation fixes
    * [Pull request #536](https://github.com/gazebosim/gz-transport/pull/536)
    * [Pull request #539](https://github.com/gazebosim/gz-transport/pull/539)
    * [Pull request #537](https://github.com/gazebosim/gz-transport/pull/537)
    * [Pull request #534](https://github.com/gazebosim/gz-transport/pull/534)
    * [Pull request #532](https://github.com/gazebosim/gz-transport/pull/533)
    * [Pull request #532](https://github.com/gazebosim/gz-transport/pull/532)
    * [Pull request #528](https://github.com/gazebosim/gz-transport/pull/528)
    * [Pull request #527](https://github.com/gazebosim/gz-transport/pull/527)
    * [Pull request #526](https://github.com/gazebosim/gz-transport/pull/526)

1. Simplify the relay tutorial docker setup
    * [Pull request #530](https://github.com/gazebosim/gz-transport/pull/530)

1. Replace deprecated File.exists with File.exist and tweak tutorial
    * [Pull request #529](https://github.com/gazebosim/gz-transport/pull/529)

1. Update gz-transport14 badge URLs
    * [Pull request #524](https://github.com/gazebosim/gz-transport/pull/524)

1. Ionic Changelog
    * [Pull request #523](https://github.com/gazebosim/gz-transport/pull/523)

1. Replace IGN_IP with GZ_IP
    * [Pull request #515](https://github.com/gazebosim/gz-transport/pull/515)

1. Add an explicit dependency on cppzmq-dev
    * [Pull request #512](https://github.com/gazebosim/gz-transport/pull/512)

1. Enable 24.04 CI, require cmake 3.22.1
    * [Pull request #504](https://github.com/gazebosim/gz-transport/pull/504)

1. Allow programmatic configuration of unicast relays.
    * [Pull request #498](https://github.com/gazebosim/gz-transport/pull/498)

1. Delete copy ctor and copy-assignment for log::Batch
    * [Pull request #501](https://github.com/gazebosim/gz-transport/pull/501)

1. Remove python3-distutils since it's not needed on Jammy (#496)
    * [Pull request #496) (#500](https://github.com/gazebosim/gz-transport/pull/496)

1. Find Python3 directly, not with GzPython
    * [Pull request #472](https://github.com/gazebosim/gz-transport/pull/472)

1. gz_TEST: more output when tests fail
    * [Pull request #479](https://github.com/gazebosim/gz-transport/pull/479)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #467](https://github.com/gazebosim/gz-transport/pull/467)

1. Use HIDE_SYMBOLS_BY_DEFAULT
    * [Pull request #461](https://github.com/gazebosim/gz-transport/pull/461)

1. Bumps in Ionic: gz-transport14
    * [Pull request #455](https://github.com/gazebosim/gz-transport/pull/455)
    * [Pull request #456](https://github.com/gazebosim/gz-transport/pull/456)


## Gazebo Transport 13.X

### Gazebo Transport 13.4.1 (2025-01-30)

1. Add compatibility with protobuf 28
    * [Pull request #541](https://github.com/gazebosim/gz-transport/pull/541)

1. Permit building python bindings separately from main library
    * [Pull request #554](https://github.com/gazebosim/gz-transport/pull/554)

1. Disable `playback.ReplayStep` for windows
    * [Pull request #517](https://github.com/gazebosim/gz-transport/pull/517)

1. Added Prerequisites section in python tutorial
    * [Pull request #516](https://github.com/gazebosim/gz-transport/pull/516)

1. `IGN_IP` -> `GZ_IP` in comments and error messages
    * [Pull request #515](https://github.com/gazebosim/gz-transport/pull/515)

### Gazebo Transport 13.4.0 (2024-06-18)

1. Add frequency to topic CLI.
    * [Pull request #503](https://github.com/gazebosim/gz-transport/pull/503)

### Gazebo Transport 13.3.0 (2024-06-05)

1. Adding option to ignore local messages
    * [Pull request #506](https://github.com/gazebosim/gz-transport/pull/506)

1. Include Python tutorial in list of tutorials
    * [Pull request #499](https://github.com/gazebosim/gz-transport/pull/499)

1. Remove python3-distutils since it's not needed on Jammy
    * [Pull request #496](https://github.com/gazebosim/gz-transport/pull/496)

1. Add package.xml
    * [Pull request #485](https://github.com/gazebosim/gz-transport/pull/485)

### Gazebo Transport 13.2.0 (2024-04-09)

1. Use relative install path for gz tool data
    * [Pull request #492](https://github.com/gazebosim/gz-transport/pull/492)

1. No input service request from the command line
    * [Pull request #487](https://github.com/gazebosim/gz-transport/pull/487)

1. Use `std::shared_ptr` for `gz::transport::NodeShared`
    * [Pull request #484](https://github.com/gazebosim/gz-transport/pull/484)

1. Use a default timeout when requesting a service from CLI.
    * [Pull request #486](https://github.com/gazebosim/gz-transport/pull/486)

1. Fix test failures when run under `colcon test`
    * [Pull request #483](https://github.com/gazebosim/gz-transport/pull/483)

### Gazebo Transport 13.1.0 (2024-03-14)

1. Oneway service request from the command line
    * [Pull request #477](https://github.com/gazebosim/gz-transport/pull/477)

1. Re-enable tests of bash completion functions for gz
    * [Pull request #481](https://github.com/gazebosim/gz-transport/pull/481)

1. Find Python3 directly, not with GzPython
    * [Pull request #472](https://github.com/gazebosim/gz-transport/pull/472)

1. Fix issue #468
    * [Pull request #470](https://github.com/gazebosim/gz-transport/pull/470)

1. Test refactoring part 2
    * [Pull request #463](https://github.com/gazebosim/gz-transport/pull/463)

1. Update CI badges in README
    * [Pull request #465](https://github.com/gazebosim/gz-transport/pull/465)

1. Support for bazel on garden
    * [Pull request #399](https://github.com/gazebosim/gz-transport/pull/399)

1. Use subprocess rather than custom code
    * [Pull request #429](https://github.com/gazebosim/gz-transport/pull/429)

1. Infrastructure
    * [Pull request #460](https://github.com/gazebosim/gz-transport/pull/460)

1. Remove duplicated functionality from test_config
    * [Pull request #457](https://github.com/gazebosim/gz-transport/pull/457)
    * [Pull request #458](https://github.com/gazebosim/gz-transport/pull/458)

1. Make empty constructor as peer to explicit
    * [Pull request #453](https://github.com/gazebosim/gz-transport/pull/453)

1. Adds the subcommands for the log command for bash completion
    * [Pull request #451](https://github.com/gazebosim/gz-transport/pull/451)

1. Adds the python bindings tutorial
    * [Pull request #450](https://github.com/gazebosim/gz-transport/pull/450)

### Gazebo Transport 13.0.0 (2023-09-29)

1. Fix Docker in Harmonic
    * [Pull request #440](https://github.com/gazebosim/gz-transport/pull/440)

1. Documentation fixes
    * [Pull request #438](https://github.com/gazebosim/gz-transport/pull/438)
    * [Pull request #437](https://github.com/gazebosim/gz-transport/pull/437)
    * [Pull request #439](https://github.com/gazebosim/gz-transport/pull/439)
    * [Pull request #441](https://github.com/gazebosim/gz-transport/pull/441)

1. Protect remoteSubscribers with mutex.
    * [Pull request #432](https://github.com/gazebosim/gz-transport/pull/432)

1. Remove deprecations in Harmonic
    * [Pull request #426](https://github.com/gazebosim/gz-transport/pull/426)

1. ign -> gz
    * [Pull request #424](https://github.com/gazebosim/gz-transport/pull/424)

1. Infrastructure
    * [Pull request #428](https://github.com/gazebosim/gz-transport/pull/428)
    * [Pull request #423](https://github.com/gazebosim/gz-transport/pull/423)
    * [Pull request #427](https://github.com/gazebosim/gz-transport/pull/427)

1. Python Bindings for Publisher, Subscriber and Service Request features.
    * [Pull request #411](https://github.com/gazebosim/gz-transport/pull/411)
    * [Pull request #433](https://github.com/gazebosim/gz-transport/pull/433)
    * [Pull request #431](https://github.com/gazebosim/gz-transport/pull/431)
    * [Pull request #443](https://github.com/gazebosim/gz-transport/pull/443)

1. Fix topic/service list inconsistency
    * [Pull request #415](https://github.com/gazebosim/gz-transport/pull/415)

1. Show subscribers info when running topic info
    * [Pull request #384](https://github.com/gazebosim/gz-transport/pull/384)

1. List subscribed topics when running topic list
    * [Pull request #379](https://github.com/gazebosim/gz-transport/pull/379)

1. ⬆️  Bump main to 13.0.0~pre1
    * [Pull request #344](https://github.com/gazebosim/gz-transport/pull/344)

## Gazebo Transport 12.X

### Gazebo Transport 12.2.1 (2023-09-26)

1. Infrastructure
    * [Pull request #428](https://github.com/gazebosim/gz-transport/pull/428)
    * [Pull request #427](https://github.com/gazebosim/gz-transport/pull/427)

1. Fix topic/service list inconsistency
    * [Pull request #415](https://github.com/gazebosim/gz-transport/pull/415)

1. Backport Windows fix to ign-transport8
    * [Pull request #406](https://github.com/gazebosim/gz-transport/pull/406)

1. Fix unused-result warning
    * [Pull request #408](https://github.com/gazebosim/gz-transport/pull/408)

1. Fix compatibility with protobuf 22
    * [Pull request #405](https://github.com/gazebosim/gz-transport/pull/405)

1. Fix compiler warning and signedness issue
    * [Pull request #401](https://github.com/gazebosim/gz-transport/pull/401)

1. Fix filesystem headers for tests
    * [Pull request #402](https://github.com/gazebosim/gz-transport/pull/402)

1. Fix typos
    * [Pull request #403](https://github.com/gazebosim/gz-transport/pull/403)


### Gazebo Transport 12.2.0 (2023-04-19)

1. CI workflow: use checkout v3
    * [Pull request #394](https://github.com/gazebosim/gz-transport/pull/394)

1. Rename COPYING to LICENSE
    * [Pull request #392](https://github.com/gazebosim/gz-transport/pull/392)

1. Support clang and std::filesystem
    * [Pull request #390](https://github.com/gazebosim/gz-transport/pull/390)

1. Added Node::RequestRaw
    * [Pull request #351](https://github.com/gazebosim/gz-transport/pull/351)

1. Pass std::function by value to Node::Subscribe
    * [Pull request #382](https://github.com/gazebosim/gz-transport/pull/382)

1. Prevent invoking callbacks after a node unsubscribes to a topic
    * [Pull request #381](https://github.com/gazebosim/gz-transport/pull/381)

1. Suppress some Windows warnings
    * [Pull request #367](https://github.com/gazebosim/gz-transport/pull/367)

1. Fix include/ignition/.../parameters header files
    * [Pull request #374](https://github.com/gazebosim/gz-transport/pull/374)

1. Fix CLI configuration install path to ignition
    * [Pull request #372](https://github.com/gazebosim/gz-transport/pull/372)

### Gazebo Transport 12.1.0 (2023-01-11)

1. Ignition to Gazebo renaming.
    * [Pull request #347](https://github.com/gazebosim/gz-transport/pull/347)

1. Use new ignition-specific formatter.
    * [Pull request #296](https://github.com/gazebosim/gz-transport/pull/296)

1. Remove warnings in Garden on Ubuntu 22.04.
    * [Pull request #364](https://github.com/gazebosim/gz-transport/pull/364)

### Gazebo Transport 12.0.0 (2022-09-22)

1. Improved windows instructions
    * [Pull request #360](https://github.com/gazebosim/gz-transport/pull/360)

1. Improved instructions
    * [Pull request #359](https://github.com/gazebosim/gz-transport/pull/359)

1. Fix table in "Nodes and topics tutorial"
    * [Pull request #356](https://github.com/gazebosim/gz-transport/pull/356)

1. Update link
    * [Pull request #358](https://github.com/gazebosim/gz-transport/pull/358)

1. Tweak section title.
    * [Pull request #357](https://github.com/gazebosim/gz-transport/pull/357)

1. Fix Docker build and relay tutorial instructions
    * [Pull request #355](https://github.com/gazebosim/gz-transport/pull/355)

1. Fix bench example compilation.
    * [Pull request #352](https://github.com/gazebosim/gz-transport/pull/352)

1. Download examples matching the version number
    * [Pull request #354](https://github.com/gazebosim/gz-transport/pull/354)

1. Tweak topic statistics tutorial
    * [Pull request #353](https://github.com/gazebosim/gz-transport/pull/353)

1. Don't use ignition/msgs.hh
    * [Pull request #315](https://github.com/gazebosim/gz-transport/pull/315)

1. Remove left-over from Discovery_TEST
    * [Pull request #343](https://github.com/gazebosim/gz-transport/pull/343)

1. Suppress protobuf related Windows warnings
    * [Pull request #334](https://github.com/gazebosim/gz-transport/pull/334)

1. Remove problematic discovery test
    * [Pull request #336](https://github.com/gazebosim/gz-transport/pull/336)

1. ign -> gz Provisional Finale: Source Migration : gz-transport
    * [Pull request #329](https://github.com/gazebosim/gz-transport/pull/329)

1. ign -> gz migrations
    * [Pull request #328](https://github.com/gazebosim/gz-transport/pull/328)

1. Use auto for loop iterators
    * [Pull request #326](https://github.com/gazebosim/gz-transport/pull/326)

1. Update GoogleTest to latest version
    * [Pull request #316](https://github.com/gazebosim/gz-transport/pull/316)

1. ign -> gz Partial Docs Migration and Project Name Followups : gz-transport
    * [Pull request #325](https://github.com/gazebosim/gz-transport/pull/325)

1. Migrate ignition-transport
    * [Pull request #324](https://github.com/gazebosim/gz-transport/pull/324)

1. Rename CMake project to gz
    * [Pull request #320](https://github.com/gazebosim/gz-transport/pull/320)

1. ign -> gz Macro Migration : gz-transport
    * [Pull request #319](https://github.com/gazebosim/gz-transport/pull/319)

1. [ign -> gz] CMake functions
    * [Pull request #322](https://github.com/gazebosim/gz-transport/pull/322)

1. ign -> gz Environment Variable Migration
    * [Pull request #318](https://github.com/gazebosim/gz-transport/pull/318)

1. ign -> gz Namespace Migration : gz-transport
    * [Pull request #311](https://github.com/gazebosim/gz-transport/pull/311)

1. ign -> gz migration
    * [Pull request #310](https://github.com/gazebosim/gz-transport/pull/310)

1. Use ign-utils instead of ign-cmake utilities
    * [Pull request #307](https://github.com/gazebosim/gz-transport/pull/307)

1. Remove Bionic from future releases (Garden+)
    * [Pull request #297](https://github.com/gazebosim/gz-transport/pull/297)

1. Bumps in garden: ign-transport12 use ign-math7
    * [Pull request #285](https://github.com/gazebosim/gz-transport/pull/285)

1. Bumps in garden : ci_matching_branch/bump_garden_ign-transport12
    * [Pull request #280](https://github.com/gazebosim/gz-transport/pull/280)


## Gazebo Transport 11.X

### Gazebo Transport 11.4.1 (2023-09-01)

1. Fix topic/service list inconsistency
    * [Pull request #415](https://github.com/gazebosim/gz-transport/pull/415)

1. Backport Windows fix to ign-transport8
    * [Pull request #406](https://github.com/gazebosim/gz-transport/pull/406)

1. Fix unused-result warning
    * [Pull request #408](https://github.com/gazebosim/gz-transport/pull/408)

1. Fix compatibility with protobuf 22
    * [Pull request #405](https://github.com/gazebosim/gz-transport/pull/405)

1. Fix compiler warning and signedness issue
    * [Pull request #401](https://github.com/gazebosim/gz-transport/pull/401)

1. Rename COPYING to LICENSE
    * [Pull request #392](https://github.com/gazebosim/gz-transport/pull/392)

1. Infrastructure
    * [Pull request #391](https://github.com/gazebosim/gz-transport/pull/391)
    * [Pull request #394](https://github.com/gazebosim/gz-transport/pull/394)

1. Support clang and std::filesystem
    * [Pull request #390](https://github.com/gazebosim/gz-transport/pull/390)

### Gazebo Transport 11.4.0 (2023-03-08)

1. Added Node::RequestRaw
    * [Pull request #351](https://github.com/gazebosim/gz-transport/pull/351)

1. Suppress some Windows warnings.
    * [Pull request #367](https://github.com/gazebosim/gz-transport/pull/367)

1. All changes up to version 8.2.0.

### Gazebo Transport 11.3.2 (2022-12-08)

1. Fix include/ignition/.../parameters header files
    * [Pull request #374](https://github.com/gazebosim/gz-transport/pull/374)

### Gazebo Transport 11.3.1 (2022-12-01)

1. Fix CLI configuration install path to ignition
    * [Pull request #372](https://github.com/gazebosim/gz-transport/pull/372)

### Gazebo Transport 11.3.0 (2022-10-31)

1. Add parameters component
    * [Pull request #305](https://github.com/gazebosim/gz-transport/pull/305)

1. Fix build for Debian Bullseye
    * [Pull request #363](https://github.com/gazebosim/gz-transport/pull/363)

### Gazebo Transport 11.2.0 (2022-08-16)

1. Remove problematic discovery test
    * [Pull request #339](https://github.com/gazebosim/gz-transport/pull/339)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #332](https://github.com/gazebosim/gz-transport/pull/332)

1. Ignition -> Gazebo
    * [Pull request #330](https://github.com/gazebosim/gz-transport/pull/330)

1. Bash completion for flags
    * [Pull request #312](https://github.com/gazebosim/gz-transport/pull/312)
    * [Pull request #333](https://github.com/gazebosim/gz-transport/pull/333)

### Gazebo Transport 11.1.0 (2022-06-01)

1. Add option to output messages in JSON format
    * [Pull request #288](https://github.com/gazebosim/gz-transport/pull/288)

1. Use libexec to install lib binaries
    * [Pull request #279](https://github.com/gazebosim/gz-transport/pull/279)
    * [Pull request #314](https://github.com/gazebosim/gz-transport/pull/314)

1. Use `exec` instead of `popen` to run `ign-launch` binary
    * [Pull request #300](https://github.com/gazebosim/gz-transport/pull/300)

1. Focal CI: static checkers, doxygen linters, compiler warnings
    * [Pull request #298](https://github.com/gazebosim/gz-transport/pull/298)

1. Add Ubuntu Jammy CI
    * [Pull request #293](https://github.com/gazebosim/gz-transport/pull/293)

1. Corrected a typo in `topic_main.cc`
    * [Pull request #292](https://github.com/gazebosim/gz-transport/pull/292)

1. Remove no username error messages
    * [Pull request #286](https://github.com/gazebosim/gz-transport/pull/286)

1. Try `USER` variable to retrieve the username.
    * [Pull request #282](https://github.com/gazebosim/gz-transport/pull/282)

1. Documented the default value of `IGN_PARTITION`
    * [Pull request #281](https://github.com/gazebosim/gz-transport/pull/281)

1. Remove static on `registrationCb` and `unregistrationCb`.
    * [Pull request #273](https://github.com/gazebosim/gz-transport/pull/273)

1. Make zmq check for post 4.3.1 not to include 4.3.1
    * [Pull request #237](https://github.com/gazebosim/gz-transport/pull/237)

1. NetUtils: simplify logic in `determineInterfaces`
    * [Pull request #257](https://github.com/gazebosim/gz-transport/pull/257)

1. Fix Homebrew warning (backport from Fortress) (#268)
    * [Pull request #270](https://github.com/gazebosim/gz-transport/pull/270)

### Gazebo Transport 11.0.0 (2021-09-28)

1. Windows fix
    * [Pull request #250](https://github.com/gazebosim/gz-transport/pull/250)

1. Remove unnecessary copy and assignment operators
    * [Pull request #240](https://github.com/gazebosim/gz-transport/pull/240)

1. Depend on gz-msgs8
    * [Pull request #238](https://github.com/gazebosim/gz-transport/pull/238)

1. Infrastructure
    * [Pull request #236](https://github.com/gazebosim/gz-transport/pull/236)
    * [Pull request #242](https://github.com/gazebosim/gz-transport/pull/242)
    * [Pull request #264](https://github.com/gazebosim/gz-transport/pull/264)
    * [Pull request #265](https://github.com/gazebosim/gz-transport/pull/265)

## Gazebo Transport 10.X

### Gazebo Transport 10.2.0 (2022-03-25)

1. Use exec instead of popen to run gz-launch binary
    * [Pull request #300](https://github.com/gazebosim/gz-transport/pull/300)

1. Focal CI: static checkers, doxygen linters, compiler warnings
    * [Pull request #298](https://github.com/gazebosim/gz-transport/pull/298)

1. Add option to output messages in JSON format
    * [Pull request #288](https://github.com/gazebosim/gz-transport/pull/288)

1. Remove no username error messages
    * [Pull request #286](https://github.com/gazebosim/gz-transport/pull/286)

1. Documented the default value of `GZ_PARTITION`
    * [Pull request #281](https://github.com/gazebosim/gz-transport/pull/281)

1. Remove static on `registrationCb` and `unregistrationCb`.
    * [Pull request #273](https://github.com/gazebosim/gz-transport/pull/273)

1. Make zmq check for post 4.3.1 not to include 4.3.1
    * [Pull request #237](https://github.com/gazebosim/gz-transport/pull/237)

1. NetUtils: simplify logic in `determineInterfaces`
    * [Pull request #257](https://github.com/gazebosim/gz-transport/pull/257)

1. Fix Homebrew warning (backport from Fortress)
    * [Pull request #268](https://github.com/gazebosim/gz-transport/pull/268)

### Gazebo Transport 10.1.0 (2021-10-17)

1. Fix Homebrew warning (backport from Fortress).
   * [Github pull request 268](https://github.com/gazebosim/gz-transport/pull/268)

1. Use standalone executables with gz tool.
   * [Github pull request 216](https://github.com/gazebosim/gz-transport/pull/216)

1. Make zmq check for post 4.3.1 not to include 4.3.1
   * [Github pull request 237](https://github.com/gazebosim/gz-transport/pull/237)

1. Remove unnecessary copy and assignment operators (#241).
   * [Github pull request 241](https://github.com/gazebosim/gz-transport/pull/241)

### Gazebo Transport 10.0.0 (2021-03-30)

1. Depend on cli component of ignition-utils
    * [Pull request #229](https://github.com/gazebosim/gz-transport/pull/229)

1. Add instructions to build and run examples
    * [Pull request #222](https://github.com/gazebosim/gz-transport/pull/222)

1. Bump in edifice: gz-msgs7
    * [Pull request #213](https://github.com/gazebosim/gz-transport/pull/213)

1. Configurable IP address and port for discovery
    * [Pull request #200](https://github.com/gazebosim/gz-transport/pull/200)

1. Changes from Dome tutorial party
    * [Pull request #187](https://github.com/gazebosim/gz-transport/pull/187)

1. Infrastructure and documentation
    * [Pull request #186](https://github.com/gazebosim/gz-transport/pull/186)
    * [Pull request #232](https://github.com/gazebosim/gz-transport/pull/232)
    * [Pull request #233](https://github.com/gazebosim/gz-transport/pull/233)

## Gazebo Transport 9.X

### Gazebo Transport 9.X.X

### Gazebo Transport 9.1.1 (2021-01-05)

1. Add errno output for discovery.
   * [Github pull request 254](https://github.com/gazebosim/gz-transport/pull/254)

1. Consider all network interfaces when checking HOST option.
   * [Github pull request 245](https://github.com/gazebosim/gz-transport/pull/245)

1. Remove tools/code_check and update codecov.
   * [Github pull request 246](https://github.com/gazebosim/gz-transport/pull/246)

1. Remove deprecated test.
   * [Github pull request 239](https://github.com/gazebosim/gz-transport/pull/239)

1. Master branch updates.
   * [Github pull request 224](https://github.com/gazebosim/gz-transport/pull/224)

1. Add windows installation.
   * [Github pull request 214](https://github.com/gazebosim/gz-transport/pull/214)

### Gazebo Transport 9.1.0 (2021-01-05)

1. All changes up to version 8.2.0.

### Gazebo Transport 9.0.0 (2020-09-29)

1. Fix link in "development" tutorial.
   * [Github pull request 183](https://github.com/gazebosim/gz-transport/pull/183)

1. Remove contribute tutorial.
   * [Github pull request 182](https://github.com/gazebosim/gz-transport/pull/182)

1. Update link to the tutorials.
   * [Github pull request 181](https://github.com/gazebosim/gz-transport/pull/181)

1. Use private access modifier for pImpl variable in MsgIter class.
   * [Github pull request 179](https://github.com/gazebosim/gz-transport/pull/179)

1. Remove deprecations before 9.x.x release.
   * [Github pull request 175](https://github.com/gazebosim/gz-transport/pull/175)

1. Make CPPZMQ a PUBLIC dependency.
   * [Github pull request 173](https://github.com/gazebosim/gz-transport/pull/173)

1. Update codeowners.
   * [Github pull request 163](https://github.com/gazebosim/gz-transport/pull/163)

1. Removed markdown-header.
   * [Github pull request 161](https://github.com/gazebosim/gz-transport/pull/161)

1. Removed repeated installation instructions.
   * [Github pull request 160](https://github.com/gazebosim/gz-transport/pull/160)

1. Need ignition-msgs version 6
   * [Github pull request 149](https://github.com/gazebosim/gz-transport/pull/149/files)

1. Update link to repo.
   * [Github pull request 142](https://github.com/gazebosim/gz-transport/pull/142)

1. Fix download link.
   * [Github pull request 141](https://github.com/gazebosim/gz-transport/pull/141)

1. Workflow updates.
   * [Github pull request 134](https://github.com/gazebosim/gz-transport/pull/134)

1. Add .gitignore.
   * [Github pull request 128](https://github.com/gazebosim/gz-transport/pull/128)

1. Update BitBucket links.
   * [Github pull request 125](https://github.com/gazebosim/gz-transport/pull/125)

## Gazebo Transport 8.X

### Gazebo Transport 8.X.X

### Gazebo Transport 8.2.0 (2020-01-05)

1. All changes up to version 7.5.1.

1. Addition of topic statistics that can report number of dropped messages
   and publication, age, and reception statistics.
    * [Pull request 205](https://github.com/gazebosim/gz-transport/pull/205)

### Gazebo Transport 8.5.0 (2024-01-05)

1. Update github action workflows
    * [Pull request #460](https://github.com/gazebosim/gz-transport/pull/460)
    * [Pull request #391](https://github.com/gazebosim/gz-transport/pull/391)
    * [Pull request #392](https://github.com/gazebosim/gz-transport/pull/392)

1. Adds the subcommands for the log command
    * [Pull request #451](https://github.com/gazebosim/gz-transport/pull/451)

1. Fix topic/service list inconsistency
    * [Pull request #415](https://github.com/gazebosim/gz-transport/pull/415)

1. Backport Windows fix to ign-transport8
    * [Pull request #406](https://github.com/gazebosim/gz-transport/pull/406)

1. Fix compatibility with protobuf 22
    * [Pull request #405](https://github.com/gazebosim/gz-transport/pull/405)

1. Fix compiler warning and signedness issue
    * [Pull request #401](https://github.com/gazebosim/gz-transport/pull/401)

1. Support clang and std::filesystem
    * [Pull request #390](https://github.com/gazebosim/gz-transport/pull/390)

1. Pass std::function by value to Node::Subscribe
    * [Pull request #382](https://github.com/gazebosim/gz-transport/pull/382)

### Gazebo Transport 8.4.0 (2022-11-17)

1. ign -> gz : Remove redundant namespace references.
    * [Pull request #345](https://github.com/gazebosim/gz-transport/pull/345)

1. Backport Windows fix from main branch.
    * [Pull request #350](https://github.com/gazebosim/gz-transport/pull/350)

1. ign -> gz Migrate Ignition Headers : gz-transport.
    * [Pull request #347](https://github.com/gazebosim/gz-transport/pull/347)

### Gazebo Transport 8.3.0 (2022-07-27)

1. Ignition -> Gazebo
    * [Pull request #330](https://github.com/gazebosim/gz-transport/pull/330)

1. Bash completion for flags
    * [Pull request #312](https://github.com/gazebosim/gz-transport/pull/312)

1. Focal CI: static checkers, doxygen linters, compiler warnings
    * [Pull request #298](https://github.com/gazebosim/gz-transport/pull/298)

1. Remove no username error messages
    * [Pull request #286](https://github.com/gazebosim/gz-transport/pull/286)

1. Documented the default value of `GZ_PARTITION`
    * [Pull request #281](https://github.com/gazebosim/gz-transport/pull/281)

1. Remove static on `registrationCb` and `unregistrationCb`.
    * [Pull request #273](https://github.com/gazebosim/gz-transport/pull/273)

### Gazebo Transport 8.2.1 (2021-10-27)

1. Make zmq check for post 4.3.1 not to include 4.3.1
    * [Pull request #237](https://github.com/gazebosim/gz-transport/pull/237)
    * [Pull request #274](https://github.com/gazebosim/gz-transport/pull/274)

1. Fix Homebrew warning (backport from Fortress)
    * [Pull request #268](https://github.com/gazebosim/gz-transport/pull/268)

1. Infrastructure
    * [Pull request #246](https://github.com/gazebosim/gz-transport/pull/246)
    * [Pull request #224](https://github.com/gazebosim/gz-transport/pull/224)

1. Remove deprecated test
    * [Pull request #239](https://github.com/gazebosim/gz-transport/pull/239)

1. Add Windows Installation using conda-forge, and cleanup install docs
    * [Pull request #214](https://github.com/gazebosim/gz-transport/pull/214)

### Gazebo Transport 8.2.0 (2020-01-05)

1. All changes up to version 7.5.1.

1. Addition of topic statistics that can report number of dropped messages
   and publication, age, and reception statistics.
    * [Pull request 205](https://github.com/gazebosim/gz-transport/pull/205)

### Gazebo Transport 8.1.0 (2020-08-28)

1. Fix mem leak.
   * [Github pull request 174](https://github.com/gazebosim/gz-transport/pull/174)

1. One NodeShared per process.
    * [Github pull request 152](https://github.com/gazebosim/gz-transport/pull/152)

1. Remove Windows warnings.
    * [Github pull request 151](https://github.com/gazebosim/gz-transport/pull/151)

1. Remove warnings on Homebrew.
    * [Github pull request 150](https://github.com/gazebosim/gz-transport/pull/150)

1. Fix ByteSize deprecation warnings for Protobuf 3.1+.
    * [BitBucket pull request 423](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/423)

1. Improve compiler support for c++ filesystem.
    * [BitBucket pull request 420](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/420)

1. Support playback of corrupt log files.
    * [BitBucket pull request 398](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/398)
    * [BitBucket pull request 425](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/425)

1. Add signal handler to log playback.
    * [BitBucket pull request 399](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/399)

1. Added additional publishers and subscribers to the `bench` example program in order to simulate high network traffic conditions.
    * [BitBucket pull request 416](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/416)

1. Added topic subscription to the C interface.
    * [BitBucket pull request 385](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/385)
    * [BitBucket pull request 417](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/417)

1. Added fast log playback, where messages are published without waiting.
    * [BitBucket pull request 401](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/401)

### Gazebo Transport 8.0.0 (2019-12-10)

1. Upgrade to ignition-msgs5.
    * [BitBucket pull request 402](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/402)

1. Utilize protobuf messages for discovery.
    * [BitBucket pull request 403](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/403)

1. Ignore incompatible discovery messages and reduce console spam.
    * [BitBucket pull request 408](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/408)

1. Improve compiler support for c++ filesystem.
    * [BitBucket pull request 405](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/405)
    * [BitBucket pull request 406](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/406)

1. This release includes all changes up to 7.5.0.

## Gazebo Transport 7

### Gazebo Transport 7.5.1 (2020-12-23)

1. CI fixes
    * [Pull request 158](https://github.com/gazebosim/gz-transport/pull/158)
    * [Pull request 176](https://github.com/gazebosim/gz-transport/pull/176)

1. Fix codecheck
    * [Pull request 194](https://github.com/gazebosim/gz-transport/pull/194)

1. Prevent empty messages from spamming the console
    * [Pull request 164](https://github.com/gazebosim/gz-transport/pull/164)

### Gazebo Transport 7.5.0 (2020-07-29)

1. Disable flaky Mac OS X tests
    * [Pull request 176](https://github.com/gazebosim/gz-transport/pull/176)

1. Prevent empty messages from spamming the console
    * [Pull request 164](https://github.com/gazebosim/gz-transport/pull/164)

1. Modernize actions CI
    * [Pull request 158](https://github.com/gazebosim/gz-transport/pull/158)

1. Helper function to get a valid topic name
    * [Pull request 153](https://github.com/gazebosim/gz-transport/pull/153)

1. GitHub migration
    * [Pull request 132](https://github.com/gazebosim/gz-transport/pull/132)
    * [Pull request 123](https://github.com/gazebosim/gz-transport/pull/123)
    * [Pull request 126](https://github.com/gazebosim/gz-transport/pull/126)

1. Fix ZMQ and Protobuf warnings
    * [BitBucket pull request 442](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/442)
    * [BitBucket pull request 438](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/438)
    * [BitBucket pull request 439](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/439)
    * [Pull request 150](https://github.com/gazebosim/gz-transport/pull/150)
    * [Pull request 151](https://github.com/gazebosim/gz-transport/pull/151)

1. Handle `getpwduid_r` error cases. This addresses issue #118. Solution was
   created in pull request #441 by Poh Zhi-Ee.
    * [BitBucket pull request 444](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/444)

### Gazebo Transport 7.4.0 (2020-03-09)

1. Removed a `sleep` from NodeShared. The sleep was meant to guarantee
   message delivery during `connect`. This approach would fail if the delay
   between nodes was too large.
    * [BitBucket pull request 436](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/436)

1. Set default message buffer sizes to 1000, for both send and receive
   buffers.
    * [BitBucket pull request 433](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/433)

1. Added support for configuring message buffers via environment variables.
    * [BitBucket pull request 430](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/430)

### Gazebo Transport 7.3.0

1. Write to disk from a background thread in log recorder
    * [BitBucket pull request 428](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/428)

1. Restore original Playback::Start and add overload with new parameter to fix ABI.
    * [BitBucket pull request 427](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/427)

1. Improve compiler support for c++ filesystem.
    * [BitBucket pull request 422](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/422)

### Gazebo Transport 7.2.1

1. Updates to C interface subscription options.
    * [BitBucket pull request 417](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/417)

### Gazebo Transport 7.2.0

1. Support playback of corrupt log files.
    * [BitBucket pull request 398](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/398)

1. Add signal handler to log playback.
    * [BitBucket pull request 399](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/399)

1. Ignore incompatible discovery messages and reduce console spam.
    * [BitBucket pull request 409](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/409)

1. Added additional publishers and subscribers to the `bench` example program in order to simulate high network traffic conditions.
    * [BitBucket pull request 416](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/416)

1. Added topic subscription to the C interface.
    * [BitBucket pull request 385](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/385)

1. Added fast log playback, where messages are published without waiting.
    * [BitBucket pull request 401](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/401)

### Gazebo Transport 7.1.0

1. Added method for determining if a throttled publisher is ready to publish.
    * [BitBucket pull request 395](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/395)

1. Add intraprocess field to MessageInfo. The intraprocess field indicates whether the message is coming from a node within this process.
    * [BitBucket pull request 394](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/394)

### Gazebo Transport 7.0.0

1. Fix fast constructor-destructor deadlock race condition.
    * [BitBucket pull request 384](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/384)

1. Added ability to specify partition information on a node through the
   CIface.
    * [BitBucket pull request 378](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/378)

1. Added ability to advertise a topic through the CIface.
    * [BitBucket pull request 377](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/377)

1. Added a `-n` argument to the echo command line tool, where `-n` can be used
   to specify the number of messages to echo and then exit. Made the
   `gz.hh` header file private (not installed).
    * [BitBucket pull request 367](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/367)

1. Added start of C interface, currently it supports only pub/sub.
    * [BitBucket pull request 366](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/366)
    * [BitBucket pull request 370](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/370)
    * [BitBucket pull request 373](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/373)

1. Introduce `GZ_RELAY`.
    * [BitBucket pull request 364](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/364)

1. Upgrade to ignition-msgs4.
    * [BitBucket pull request 371](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/371)

## Gazebo Transport 6

### Gazebo Transport 6.X.X

1. Ignore EPERM and ENOBUFS errors during discovery, generalize cmake for gz tool files
    * [BitBucket pull request 380](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/380)
    * [Issue 106](https://github.com/gazebosim/gz-transport/issues/106)

1. Skip `cmd*.rb` generation on windows to fix build
    * [BitBucket pull request 363](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/363)
    * [Issue 94](https://github.com/gazebosim/gz-transport/issues/94)

### Gazebo Transport 6.0.0

1. Upgrade to proto3, c++17, ignition-cmake2 and ignition-msgs3.
    * [BitBucket pull request 312](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/312)

## Gazebo Transport 5

### Gazebo Transport 5.X.X

1. Added support for alternative clock sources during log recording.
    * [BitBucket pull request 340](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/340)

1. Exposed Log and log Playback time information.
    * [BitBucket pull request 342](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/342)

1. Added the ability to Seek within the log playback, which makes possible to
   jump to any valid time point of the reproduction.
    * [BitBucket pull request 341](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/341)

1. Added the ability to Step the advance of the playback from within the log
   replayer.
    * [BitBucket pull request 339](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/339)

1. Added the ability to Pause/Resume playback from the log replayer.
    * [BitBucket pull request 334](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/334)

1. Added support for topic remapping when running "gz log playback". Note that
   the string ":=" is not allowed now as part of a partition, namespace or topic
   anymore.
    * [BitBucket pull request 331](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/331)

1. Added the ability to remap topic names.
    * [BitBucket pull request 330](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/330)

1. Prevent the log recorder from subscribing to topics that have already
   been added.
    * [BitBucket pull request 329](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/329)

1. Added log::Recorder::Topics that returns the set of added topics.
    * [BitBucket pull request 328](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/328)

1. Added log::Recorder::Filename that returns the name of the log file.
    * [BitBucket pull request 327](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/327)

1. Added a logging tutorial
    * [BitBucket pull request 311](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/311)

1. Added a migration guide for helping with the transition between major
   versions
    * [BitBucket pull request 310](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/310)

1. Converted gz-transport-log into a component
    * [BitBucket pull request 298](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/298)

1. Added inline versioned namespace to the log library
    * [BitBucket pull request 303](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/303)

1. Added inline versioned namespace to the main library
    * [BitBucket pull request 301](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/301)

1. Added --force option to 'gz log record'
    * [BitBucket pull request 325](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/325)

## Gazebo Transport 4

### Gazebo Transport 4.X.X

1. Ignore subinterfaces when using determineInterfaces().
    * [BitBucket pull request 314](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/314)

1. Refactored Playback to return a PlaybackHandle from Start()
    * [BitBucket pull request 302](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/302)

1. Added command line tool for the logging features
    * [BitBucket pull request 276](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/276)

1. Added examples using logging features
    * [BitBucket pull request 279](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/279)

1. Added integration tests for recording
    * [BitBucket pull request 275](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/275)

1. Added ability to play back gz transport topics
    * [BitBucket pull request 274](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/274)

1. Added ability to record gz transport topics
    * [BitBucket pull request 273](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/273)

1. Added ability to query log messages by topic name and time received
    * [BitBucket pull request 272](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/272)

1. Added ability to get all messages from a log file
    * [BitBucket pull request 271](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/271)

1. Added ability to insert messages into a sqlite3 based log file
    * [BitBucket pull request 270](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/270)

1. Added TopicUtils::DecomposeFullyQualifiedTopic()
    * [BitBucket pull request 269](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/269)

### Gazebo Transport 4.0.0 (2018-01-XX)

1. Basic authentication for topics.
    * [BitBucket pull request 236](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/236)

1. Upgrade to gz-cmake.
    * [BitBucket pull request 239](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/239)

1. Added a benchmark program to test latency and throughput.
    * [BitBucket pull request 225](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/225)

1. Support publication and receipt of raw serialized data.
    * [BitBucket pull request 251](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/251)

1. Use zero copy when publishing messages.
    * [BitBucket pull request 229](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/229)

1. Added publishing and receiving messages as raw bytes
    * [BitBucket pull request 251](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/251)

1. Updated service responder callbacks to return a boolean value. The
   existing functions have been deprecated.
    * [BitBucket pull request 260](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/260)
    * [BitBucket pull request 228](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/228)

1. Hide ZMQ from public interfaces
    * [BitBucket pull request 224](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/224)

## Gazebo Transport 3

### Gazebo Transport 3.X.X



### Gazebo Transport 3.1.0 (2017-11-29)

1. Documentation improvements
    * [BitBucket pull request 199](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/199)
    * [BitBucket pull request 200](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/200)
    * [BitBucket pull request 203](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/203)
    * [BitBucket pull request 206](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/206)
    * [BitBucket pull request 197](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/197)
    * [BitBucket pull request 219](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/219)
    * [Issue 63](https://github.com/gazebosim/gz-transport/issues/63)
    * [Issue 67](https://github.com/gazebosim/gz-transport/issues/67)

1. Workaround for the ghost Msbuild warning in Jenkins plugin
    * [BitBucket pull request 205](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/205)

1. Added tests for gz.cc
    * [BitBucket pull request 209](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/209)

1. Remove manual setting of flags for dynamic linking of the Windows CRT library
    * [BitBucket pull request 210](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/210)

1. Add BUILD_TESTING CMake option and tests target
    * [BitBucket pull request 208](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/208)

1. Remove unused statement from Header::Unpack
    * [BitBucket pull request 212](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/212)

1. Port cmake fixes from sdformat
    * [BitBucket pull request 213](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/213)

1. Clean up DefaultFlags.cmake
    * [BitBucket pull request 214](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/214)

1. Add the new const methods to overloaded bool operator
    * [BitBucket pull request 217](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/217)

1. SubscriptionHandler.hh fix std::move compiler warning
    * [BitBucket pull request 222](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/222)

1. Fix gz topic|service fails on MacOS X if system integrity protection is enabled
    * [BitBucket pull request 227](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/227)
    * [Issue 72](https://github.com/gazebosim/gz-transport/issues/72)

### Gazebo Transport 3.0.0

1. Added optional message throttling when publishing messages.
    * [BitBucket pull request 194](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/194)

1. Support for an optional MessageInfo parameter in the user callbacks for
   receiving messages. This parameter provides some information about the
   message received (e.g.: topic name).
    * [BitBucket pull request 191](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/191)

1. Added `Node::Publisher::HasConnections` function that can be used to
   check if a Publisher has subscribers.
    * [BitBucket pull request 190](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/190)

1. Add gz topic --echo command line tool.
    * [BitBucket pull request 189](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/189)

1. Support a generic callback signature for receiving messages of any type.
    * [BitBucket pull request 188](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/188)

1. Node::Unadvertise(), Node::Publish() and Node::TopicsAdvertised() removed.
   Node::Advertise() returns a Node::Publisher object that can be used for
   publishing messages. When this object runs out of scope the topic is
   unadvertised.
    * [BitBucket pull request 186](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/186)
    * [BitBucket pull request 185](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/185)
    * [BitBucket pull request 184](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/184)

1. Create AdvertiseMessageOptions and AdvertiseServiceOptions classes.
    * [BitBucket pull request 184](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/184)

1. Subscription options added. The first option is to provide the ability to
   set the received message rate on the subscriber side.
    * [BitBucket pull request 174](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/174)

1. Added gz service --req <args ...> for requesting services using the command line.
    * [BitBucket pull request 172](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/172)

1. Do not allow to advertise a topic that is currently advertised on the same node.
   See [issue #54](https://github.com/gazebosim/gz-transport/issues/54)
    * [BitBucket pull request 169](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/169)

1. ZeroMQ updated from 3.2.4 to 4.0.4 on Windows.
    * [BitBucket pull request 171](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/171)

## Gazebo Transport 2.x

1. Fix issue #55.
    * [BitBucket pull request 183](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/183)

1. Protobuf3 support added.
    * [BitBucket pull request 181](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/181)

1. ZeroMQ updated from 3.2.4 to 4.0.4 on Windows.
    * [BitBucket pull request 171](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/171)

1. Copyright added to `tools/code_check.sh` and `tools/cpplint_to_cppcheckxml.py`
    * [BitBucket pull request 168](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/168)

1. Fix case where `std::bad_function_call` could be thrown.
    * [BitBucket pull request 317](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/317)

### Gazebo Transport 2.0.0

1. Move ZMQ initialization from constructor to separate function in
   NodeShared.
    * [BitBucket pull request 166](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/166)

1. `Node::Advertise` returns a publisher id that can be used to publish messages, as an alternative to remembering topic strings.
    * [BitBucket pull request 129](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/129)

## Gazebo Transport 1.x

### Gazebo Transport 1.2.0

1. Removed duplicate code in NetUtils, and improved speed of DNS lookup
    * [BitBucket pull request 128](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-transport/pull-requests/128)
