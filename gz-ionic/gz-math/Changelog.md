## Gazebo Math 8.x

### Gazebo Math 8.2.1 (2026-01-15)

1. Bazel Infrastructure
    * [Pull request #713](https://github.com/gazebosim/gz-math/pull/713)
    * [Pull request #710](https://github.com/gazebosim/gz-math/pull/710)
    * [Pull request #708](https://github.com/gazebosim/gz-math/pull/708)
    * [Pull request #683](https://github.com/gazebosim/gz-math/pull/683)
    * [Pull request #681](https://github.com/gazebosim/gz-math/pull/681)
    * [Pull request #651](https://github.com/gazebosim/gz-math/pull/651)
    * [Pull request #680](https://github.com/gazebosim/gz-math/pull/680)

1. Correct calculation in ProjectPointToPlane
    * [Pull request #702](https://github.com/gazebosim/gz-math/pull/702)

1. Update tutorial build step
    * [Pull request #626](https://github.com/gazebosim/gz-math/pull/626)

1. python: suppress Pose3 deprecation warnings
    * [Pull request #682](https://github.com/gazebosim/gz-math/pull/682)

1. Fix spelling in docs
    * [Pull request #670](https://github.com/gazebosim/gz-math/pull/670)

1. Unify Python3_Development_FOUND checks
    * [Pull request #663](https://github.com/gazebosim/gz-math/pull/663)

### Gazebo Math 8.2.0 (2025-05-12)

1. Fix bazel build errors with clang
    * [Pull request #678](https://github.com/gazebosim/gz-math/pull/678)

1. Use 24.04 for doxygen CI
    * [Pull request #676](https://github.com/gazebosim/gz-math/pull/676)

1. Use Bazel Central Registry version of eigen
    * [Pull request #675](https://github.com/gazebosim/gz-math/pull/675)

1. Test building python bindings in Ubuntu CI
    * [Pull request #672](https://github.com/gazebosim/gz-math/pull/672)

1. Docs/spell check
    * [Pull request #669](https://github.com/gazebosim/gz-math/pull/669)

1. feat: Add AxisAlignedBox conversion helpers
    * [Pull request #667](https://github.com/gazebosim/gz-math/pull/667)

1. bazel: Build pybind module and test
    * [Pull request #661](https://github.com/gazebosim/gz-math/pull/661)

### Gazebo Math 8.1.1 (2025-02-12)

1. Unify `Python3_Development_FOUND` checks
    * [Pull request #662](https://github.com/gazebosim/gz-math/pull/662)

1. Change to globbed bazel targets
    * [Pull request #652](https://github.com/gazebosim/gz-math/pull/652)

1. SignalStats.cc: add missing includes
    * [Pull request #659](https://github.com/gazebosim/gz-math/pull/659)

1. Encapsulate std::cerr under a single point of control
    * [Pull request #656](https://github.com/gazebosim/gz-math/pull/656)

1. ci.yml: run cppcheck, cpplint on noble
    * [Pull request #655](https://github.com/gazebosim/gz-math/pull/655)

### Gazebo Math 8.1.0 (2024-11-15)

1. Build gz-math with bzlmod
    * [Pull request #648](https://github.com/gazebosim/gz-math/pull/648)
    * [Pull request #646](https://github.com/gazebosim/gz-math/pull/646)
    * [Pull request #644](https://github.com/gazebosim/gz-math/pull/644)
    * [Pull request #589](https://github.com/gazebosim/gz-math/pull/589)

1. Add CI support for bzlmod
    * [Pull request #643](https://github.com/gazebosim/gz-math/pull/643)
    * [Pull request #637](https://github.com/gazebosim/gz-math/pull/637)

1. Specify template for Vector2 in Vector2_TEST
    * [Pull request #645](https://github.com/gazebosim/gz-math/pull/645)

1. Python bindings: get version from package.xml
    * [Pull request #642](https://github.com/gazebosim/gz-math/pull/642)

1. Permit building python bindings separately from gz-math library
    * [Pull request #640](https://github.com/gazebosim/gz-math/pull/640)
    * [Pull request #636](https://github.com/gazebosim/gz-math/pull/636)

1. Merge all changes from 7.5.1
    * [Pull request #623](https://github.com/gazebosim/gz-math/pull/623)

### Gazebo Math 8.0.0 (2024-09-25)

1. Miscellaneous documentation fixes
    * [Pull request #632](https://github.com/gazebosim/gz-math/pull/632)
    * [Pull request #631](https://github.com/gazebosim/gz-math/pull/631)
    * [Pull request #630](https://github.com/gazebosim/gz-math/pull/630)
    * [Pull request #628](https://github.com/gazebosim/gz-math/pull/628)

1. Fix Vector examples
    * [Pull request #629](https://github.com/gazebosim/gz-math/pull/629)

1. Added CoordinateVector3 and used it in SphericalCoordinates
    * [Pull request #616](https://github.com/gazebosim/gz-math/pull/616)

1. Clamping color channel values to within [0, 1]
    * [Pull request #613](https://github.com/gazebosim/gz-math/pull/613)

1. Use ImplPtr, Implement rule of zero
    * [Pull request #608](https://github.com/gazebosim/gz-math/pull/608)
    * [Pull request #614](https://github.com/gazebosim/gz-math/pull/614)

1. Fix flaky Stopwatch test
    * [Pull request #610](https://github.com/gazebosim/gz-math/pull/610)

1. Line\*.hh: remove unneeded use of static variable
    * [Pull request #607](https://github.com/gazebosim/gz-math/pull/607)

1. Fix potential unsafe initialization in the Graph class
    * [Pull request #606](https://github.com/gazebosim/gz-math/pull/606)
    * [Pull request #609](https://github.com/gazebosim/gz-math/pull/609)
    * [Pull request #612](https://github.com/gazebosim/gz-math/pull/612)

1. Remove APIs deprecated in gz-math7
    * [Pull request #601](https://github.com/gazebosim/gz-math/pull/601)

1. Deprecate math::clock alias
    * [Pull request #600](https://github.com/gazebosim/gz-math/pull/600)

1. Clean up nested namespaces
    * [Pull request #603](https://github.com/gazebosim/gz-math/pull/603)

1. Find python directly instead of using GzPython
    * [Pull request #588](https://github.com/gazebosim/gz-math/pull/588)

1. Adding cone primitives.
    * [Pull request #594](https://github.com/gazebosim/gz-math/pull/594)

1. Enable 24.04 CI, remove distutils dependency
    * [Pull request #587](https://github.com/gazebosim/gz-math/pull/587)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #573](https://github.com/gazebosim/gz-math/pull/573)

1. Use HIDE_SYMBOLS_BY_DEFAULT, fix MovingWindowFilter
    * [Pull request #566](https://github.com/gazebosim/gz-math/pull/566)

1. Dependency version bumps in ionic: use gz-cmake4, gz-utils3
    * [Pull request #562](https://github.com/gazebosim/gz-math/pull/562)

1. Remove ignition headers and macros
    * [Pull request #541](https://github.com/gazebosim/gz-math/pull/541)
    * [Pull request #602](https://github.com/gazebosim/gz-math/pull/602)

1. Bump major version to 8
    * [Pull request #478](https://github.com/gazebosim/gz-math/pull/478)

## Gazebo Math 7.x

### Gazebo Math 7.5.2 (2025-01-30)

1. Backport bazel BUILD updates from gz-math8
    * [Pull request #651](https://github.com/gazebosim/gz-math/pull/651)

1. Permit building python bindings separately from gz-math library
    * [Pull request #640](https://github.com/gazebosim/gz-math/pull/640)

### Gazebo Math 7.5.1 (2024-08-23)

1. Fix tolerance for axis-angle conversion of quaternions
    * [Pull request #620](https://github.com/gazebosim/gz-math/pull/620)

1. Added box examples in C++ and Python
    * [Pull request #584](https://github.com/gazebosim/gz-math/pull/584)

1. Check for 0 stddev when generating a random number using normal distribution
    * [Pull request #615](https://github.com/gazebosim/gz-math/pull/615)

1. Clean up nested namespaces
    * [Pull request #603](https://github.com/gazebosim/gz-math/pull/603)

### Gazebo Math 7.5.0 (2024-06-18)

1. Backport: Adding cone primitives.
    * [Pull request #594](https://github.com/gazebosim/gz-math/pull/594)

1. Enable 24.04 CI on harmonic
    * [Pull request #590](https://github.com/gazebosim/gz-math/pull/590)

1. Add package.xml
    * [Pull request #581](https://github.com/gazebosim/gz-math/pull/581)

1. bazel: correctly export license
    * [Pull request #586](https://github.com/gazebosim/gz-math/pull/586)

1. Add missing eigen3.hh header for bazel build
    * [Pull request #585](https://github.com/gazebosim/gz-math/pull/585)

1. Expose non-const reference to edges
    * [Pull request #580](https://github.com/gazebosim/gz-math/pull/580)

### Gazebo Math 7.4.0 (2024-03-14)

1. Added MecanumDriveOdometry Python wrapper
    * [Pull request #549](https://github.com/gazebosim/gz-math/pull/549)

1. Update CI badges in README
    * [Pull request #571](https://github.com/gazebosim/gz-math/pull/571)

1. Infrastructure
    * [Pull request #569](https://github.com/gazebosim/gz-math/pull/569)

1. Suppress warnings on MSVC
    * [Pull request #564](https://github.com/gazebosim/gz-math/pull/564)

1. Remove the use of numeric_limits in appendToStream test
    * [Pull request #553](https://github.com/gazebosim/gz-math/pull/553)

1. Replace CMake Python variables with new ones from FindPython3 module
    * [Pull request #402](https://github.com/gazebosim/gz-math/pull/402)

1. Fix `Matrix3_TEST.py` on Windows with conda-forge dependencies
    * [Pull request #561](https://github.com/gazebosim/gz-math/pull/561)

1. Fix small typo cppgetstarted.md
    * [Pull request #560](https://github.com/gazebosim/gz-math/pull/560)

1. Update Ubuntu Binary installation since apt-key is deprecated
    * [Pull request #559](https://github.com/gazebosim/gz-math/pull/559)

1. Update file tree in README to point out pybind11
    * [Pull request #558](https://github.com/gazebosim/gz-math/pull/558)

1. Update tutorial/color.md
    * [Pull request #557](https://github.com/gazebosim/gz-math/pull/557)

1. ign->gz in README.md
    * [Pull request #556](https://github.com/gazebosim/gz-math/pull/556)

1. Update example_triangle.md
    * [Pull request #555](https://github.com/gazebosim/gz-math/pull/555)

### Gazebo Math 7.3.0 (2023-08-29)

1. Adds a validity check for Sessions created using the `TimeVaryingVolumetricGrid`
    * [Pull request #551](https://github.com/gazebosim/gz-math/pull/551)

1. Fixes for testing in non standard architectures
    * [Pull request #546](https://github.com/gazebosim/gz-math/pull/546)

1. MecanumDriveOdometry to handle odometry estimation of Mecanum wheeled models
    * [Pull request #486](https://github.com/gazebosim/gz-math/pull/486)

1. Fix documentation on Plane.hh
    * [Pull request #544](https://github.com/gazebosim/gz-math/pull/544)

1. Infrastructure
    * [Pull request #543](https://github.com/gazebosim/gz-math/pull/543)
    * [Pull request #547](https://github.com/gazebosim/gz-math/pull/547)
    * [Pull request #545](https://github.com/gazebosim/gz-math/pull/545)

1. Build examples from CMake rather than executable
    * [Pull request #482](https://github.com/gazebosim/gz-math/pull/482)

1. Added `std::optional MassMatrix()` functions for Box, Cylinder & Sphere
    * [Pull request #538](https://github.com/gazebosim/gz-math/pull/538)

### Gazebo Math 7.2.0 (2023-06-14)

1. Add tests to increase test coverage
    * [Pull request #533](https://github.com/gazebosim/gz-math/pull/533)

1. Forward ports
    * [Pull request #530](https://github.com/gazebosim/gz-math/pull/530)
    * [Pull request #526](https://github.com/gazebosim/gz-math/pull/526)
    * [Pull request #522](https://github.com/gazebosim/gz-math/pull/522)
    * [Pull request #520](https://github.com/gazebosim/gz-math/pull/520)

1. Disable pybind11 on windows by default
    * [Pull request #529](https://github.com/gazebosim/gz-math/pull/529)

1. Add option to skip pybind11 and SWIG
    * [Pull request #480](https://github.com/gazebosim/gz-math/pull/480)

1. Custom PID error rate
    * [Pull request #525](https://github.com/gazebosim/gz-math/pull/525)

1. Garden bazel support
    * [Pull request #523](https://github.com/gazebosim/gz-math/pull/523)

1. Rename COPYING to LICENSE
    * [Pull request #521](https://github.com/gazebosim/gz-math/pull/521)

1. Infrastructure
    * [Pull request #519](https://github.com/gazebosim/gz-math/pull/519)

1. Fix link of installation tutorial to point to 7.1 version
    * [Pull request #518](https://github.com/gazebosim/gz-math/pull/518)

### Gazebo Math 7.1.0 (2022-11-15)

1. Adds bounds retrieval for TimeVarying grid class.
    * [Pull request #508](https://github.com/gazebosim/gz-math/pull/508)

### Gazebo Math 7.0.2 (2022-09-26)

1. Update to disable tests failing on arm64
    * [Pull request #512](https://github.com/gazebosim/gz-math/pull/512)

### Gazebo Math 7.0.1 (2022-09-23)

1. Disable tests failing on arm64
    * [Pull request #510](https://github.com/gazebosim/gz-math/pull/510)

### Gazebo Math 7.0.0 (2022-09-22)

1. Deprecated `Angle::Degree(double)` and `Angle::Radian(double)`. Use `Angle::SetDegree(double)` and `Angle::SetRadian(double)` instead.
    * [BitBucket pull request 326](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/326)

1. Added Equal functions with a tolerance parameter to Pose3 and Quaternion.
    * [BitBucket pull request 319](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/319)

1. Updates per issue [#101](https://github.com/gazebosim/gz-math/issues/101).
    * Helpers: [BitBucket pull request 330](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/330),
      [BitBucket pull request 323](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/323)
    * Matrix3: [BitBucket pull request 328](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/328)
    * Pose: [BitBucket pull request 329](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/329)
    * Quaternion: [BitBucket pull request 327](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/327),
      [BitBucket pull request 338](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/338)

1. Removed deprecations.
    * [Pull request #377](https://github.com/gazebosim/gz-math/pull/377)
    * [Pull request #344](https://github.com/gazebosim/gz-math/pull/344)
    * [BitBucket pull request 322](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/322)
    * [BitBucket pull request 320](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/320)

1. Updated Temperature class documentation, script interface, and added examples.
    * [BitBucket pull request 339](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/339)

1. Prevent -0 with out stream operator
    * [Pull request #206](https://github.com/gazebosim/gz-math/pull/206)

1. Use `<ostream>` and `<istream>` narrower includes, not `<iostream>`
    * [Pull request #287](https://github.com/gazebosim/gz-math/pull/287)

1. Evict large function definitions from the Helpers.hh header file.
    * [Pull request #288](https://github.com/gazebosim/gz-math/pull/288)

1. Defer regex construction to avoid static initialization.
    * [Pull request #289](https://github.com/gazebosim/gz-math/pull/289)

1. Defer Material::Predefined construction to avoid static initialization.
    * [Pull request #290](https://github.com/gazebosim/gz-math/pull/290)

1. Resolve cppcheck errors by adding explicit constructors and consts.
    * [Pull request #291](https://github.com/gazebosim/gz-math/pull/291)

1. Remove virtual from destructors of copyable classes.
    * [Pull request #293](https://github.com/gazebosim/gz-math/pull/293)

1. Use constexpr for simple static constants.
    * [Pull request #283](https://github.com/gazebosim/gz-math/pull/283)

1. Fix output stream operator
    * [Pull request #376](https://github.com/gazebosim/gz-math/pull/376)
    * [Pull request #454](https://github.com/gazebosim/gz-math/pull/454)
    * [Pull request #499](https://github.com/gazebosim/gz-math/pull/499)

1. Tutorial updates
    * [Pull request #505](https://github.com/gazebosim/gz-math/pull/505)
    * [Pull request #502](https://github.com/gazebosim/gz-math/pull/502)
    * [Pull request #501](https://github.com/gazebosim/gz-math/pull/501)
    * [Pull request #500](https://github.com/gazebosim/gz-math/pull/500)
    * [Pull request #496](https://github.com/gazebosim/gz-math/pull/496)
    * [Pull request #495](https://github.com/gazebosim/gz-math/pull/495)
    * [Pull request #493](https://github.com/gazebosim/gz-math/pull/493)
    * [Pull request #492](https://github.com/gazebosim/gz-math/pull/492)
    * [Pull request #147](https://github.com/gazebosim/gz-math/pull/147)
    * [BitBucket pull request 341](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/341)

1. Fixed quaternion from euler example
    * [Pull request #494](https://github.com/gazebosim/gz-math/pull/494)

1. Rename the python library as gz.math7
    * [Pull request #503](https://github.com/gazebosim/gz-math/pull/503)

1. Add option to skip pybind11 and SWIG
    * [Pull request #480](https://github.com/gazebosim/gz-math/pull/480)

1. Python bindings
    * [Pull request #497](https://github.com/gazebosim/gz-math/pull/497)
    * [Pull request #488](https://github.com/gazebosim/gz-math/pull/488)
    * [Pull request #400](https://github.com/gazebosim/gz-math/pull/400)
    * [Pull request #380](https://github.com/gazebosim/gz-math/pull/380)
    * [Pull request #379](https://github.com/gazebosim/gz-math/pull/379)
    * [Pull request #280](https://github.com/gazebosim/gz-math/pull/280)

1. Add a Time Interpolation API
    * [Pull request #456](https://github.com/gazebosim/gz-math/pull/456)

1. Add fluid added mass to inertial
    * [Pull request #459](https://github.com/gazebosim/gz-math/pull/459)

1. VolumetricGridLookupField: generalize index type, add constructor
    * [Pull request #475](https://github.com/gazebosim/gz-math/pull/475)

1. Drop unnecessary moving window filter pimpl
    * [Pull request #294](https://github.com/gazebosim/gz-math/pull/294)

1. Migrate ign -> gz
    * [Pull request #466](https://github.com/gazebosim/gz-math/pull/466)
    * [Pull request #463](https://github.com/gazebosim/gz-math/pull/463)
    * [Pull request #462](https://github.com/gazebosim/gz-math/pull/462)
    * [Pull request #445](https://github.com/gazebosim/gz-math/pull/445)
    * [Pull request #441](https://github.com/gazebosim/gz-math/pull/441)
    * [Pull request #437](https://github.com/gazebosim/gz-math/pull/437)
    * [Pull request #430](https://github.com/gazebosim/gz-math/pull/430)
    * [Pull request #427](https://github.com/gazebosim/gz-math/pull/427)
    * [Pull request #420](https://github.com/gazebosim/gz-math/pull/420)
    * [Pull request #419](https://github.com/gazebosim/gz-math/pull/419)
    * [Pull request #413](https://github.com/gazebosim/gz-math/pull/413)

1. Fix output stream operator
    * [Pull request #376](https://github.com/gazebosim/gz-math/pull/376)
    * [Pull request #454](https://github.com/gazebosim/gz-math/pull/454)
    * [Pull request #499](https://github.com/gazebosim/gz-math/pull/499)

1. Compute distance between points on a general celestial body
    * [Pull request #434](https://github.com/gazebosim/gz-math/pull/434)
    * [Pull request #452](https://github.com/gazebosim/gz-math/pull/452)

1. Update GoogleTest to latest version
    * [Pull request #435](https://github.com/gazebosim/gz-math/pull/435)

1. Reduce compilation time
    * [Pull request #433](https://github.com/gazebosim/gz-math/pull/433)

1. Pose3: deprecate +, -, and -= operators
    * [Pull request #381](https://github.com/gazebosim/gz-math/pull/381)
    * [Pull request #438](https://github.com/gazebosim/gz-math/pull/438)

1. Document `Pose::operator*()`
    * [Pull request #170](https://github.com/gazebosim/gz-math/pull/170)

1. Subtraction operator for Inertial class
    * [Pull request #432](https://github.com/gazebosim/gz-math/pull/432)

1. Add linear interpolation methods.
    * [Pull request #412](https://github.com/gazebosim/gz-math/pull/412)

1. Infrastructure
    * [Pull request #440](https://github.com/gazebosim/gz-math/pull/440)
    * [Pull request #418](https://github.com/gazebosim/gz-math/pull/418)
    * [Pull request #414](https://github.com/gazebosim/gz-math/pull/414)
    * [Pull request #410](https://github.com/gazebosim/gz-math/pull/410)
    * [Pull request #409](https://github.com/gazebosim/gz-math/pull/409)
    * [Pull request #386](https://github.com/gazebosim/gz-math/pull/386)
    * [Pull request #364](https://github.com/gazebosim/gz-math/pull/364)
    * [Pull request #296](https://github.com/gazebosim/gz-math/pull/296)
    * [Pull request #149](https://github.com/gazebosim/gz-math/pull/149)
    * [Pull request #118](https://github.com/gazebosim/gz-math/pull/118)
    * [Pull request #113](https://github.com/gazebosim/gz-math/pull/113)
    * [Pull request #111](https://github.com/gazebosim/gz-math/pull/111)

1. Add PiecewiseScalarField3 class
    * [Pull request #398](https://github.com/gazebosim/gz-math/pull/398)

1. Add AdditivelySeparableScalarField3 class
    * [Pull request #397](https://github.com/gazebosim/gz-math/pull/397)

1. Add Polynomial3 class
    * [Pull request #393](https://github.com/gazebosim/gz-math/pull/393)

1. Add Interval and Region3 classes
    * [Pull request #388](https://github.com/gazebosim/gz-math/pull/388)
    * [Pull request #390](https://github.com/gazebosim/gz-math/pull/390)
    * [Pull request #396](https://github.com/gazebosim/gz-math/pull/396)

1. Add missing headers
    * [Pull request #385](https://github.com/gazebosim/gz-math/pull/385)
    * [Pull request #342](https://github.com/gazebosim/gz-math/pull/342)
    * [Pull request #327](https://github.com/gazebosim/gz-math/pull/327)
    * [Pull request #162](https://github.com/gazebosim/gz-math/pull/162)
    * [Pull request #119](https://github.com/gazebosim/gz-math/pull/119)

1. [math7] Tidy up unused variables
    * [Pull request #372](https://github.com/gazebosim/gz-math/pull/372)

1. Use ImplPtr from ign-utils where relevant
    * [Pull request #299](https://github.com/gazebosim/gz-math/pull/299)

1. Modified cmake target name for Ruby interfaces
    * [Pull request #285](https://github.com/gazebosim/gz-math/pull/285)

1. Make API / naming consistent for Frustum, Edge, Angle
    * [Pull request #329](https://github.com/gazebosim/gz-math/pull/329)

1. Reduce Stopwatch test sleep times
    * [Pull request #326](https://github.com/gazebosim/gz-math/pull/326)

1. Speed up Helpers::Pair test
    * [Pull request #295](https://github.com/gazebosim/gz-math/pull/295)

1. Make Vector2 API more consistent with Vector3
    * [Pull request #130](https://github.com/gazebosim/gz-math/pull/130)
    * [Issue #71](https://github.com/gazebosim/gz-math/issues/71)

1. Clean up a few new GCC9 warnings
    * [BitBucket pull request 336](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/336)

## Gazebo Math 6.x

## Gazebo Math 6.15.1 (2024-01-05)

1. Replace CMake Python variables with new ones from FindPython3 module
    * [Pull request #402](https://github.com/gazebosim/gz-math/pull/402)

1. Suppress warnings on MSVC
    * [Pull request #564](https://github.com/gazebosim/gz-math/pull/564)

1. Infrastructure
    * [Pull request #569](https://github.com/gazebosim/gz-math/pull/569)

## Gazebo Math 6.15.0 (2023-09-01)

1. Fixes for testing in non standard architectures
    * [Pull request #546](https://github.com/gazebosim/gz-math/pull/546)

1. MecanumDriveOdometry to handle odometry estimation of Mecanum wheeled models
    * [Pull request #486](https://github.com/gazebosim/gz-math/pull/486)

1. Infrastructure
    * [Pull request #547](https://github.com/gazebosim/gz-math/pull/547)

## Gazebo Math 6.14.0 (2023-04-14)

1. Disable pybind11 on windows by default
    * [Pull request #529](https://github.com/gazebosim/gz-math/pull/529)

1. Add option to skip pybind11 and SWIG
    * [Pull request #480](https://github.com/gazebosim/gz-math/pull/480)

1. Custom PID error rate
    * [Pull request #525](https://github.com/gazebosim/gz-math/pull/525)

1. Infrastructure
    * [Pull request #521](https://github.com/gazebosim/gz-math/pull/521)
    * [Pull request #519](https://github.com/gazebosim/gz-math/pull/519)

## Gazebo Math 6.13.0 (2022-08-31)

1. Support migration and migrate headers
    * [Pull request #483](https://github.com/gazebosim/gz-math/pull/483)

1. Remove redundant namespace references
    * [Pull request #479](https://github.com/gazebosim/gz-math/pull/479)

## Gazebo Math 6.12.0 (2022-07-27)

1. Added Helper function isTimeString to Python
    * [Pull request #471](https://github.com/gazebosim/gz-math/pull/471)

1. Ignition -> Gazebo
    * [Pull request #467](https://github.com/gazebosim/gz-math/pull/467)
    * [Pull request #468](https://github.com/gazebosim/gz-math/pull/468)
    * [Pull request #474](https://github.com/gazebosim/gz-math/pull/474)

1. Added `*.pyc` to `.gitignore`
    * [Pull request #461](https://github.com/gazebosim/gz-math/pull/461)

1. Alphabetize examples
    * [Pull request #457](https://github.com/gazebosim/gz-math/pull/457)

1. Add `PiecewiseScalarField3` class
    * [Pull request #398](https://github.com/gazebosim/gz-math/pull/398)

1. Add `AdditivelySeparableScalarField3` class
    * [Pull request #397](https://github.com/gazebosim/gz-math/pull/397)

1. Add `Polynomial3` class
    * [Pull request #393](https://github.com/gazebosim/gz-math/pull/393)
    * [Pull request #451](https://github.com/gazebosim/gz-math/pull/451)
    * [Pull request #396](https://github.com/gazebosim/gz-math/pull/396)

1. Add `Region3` class
    * [Pull request #390](https://github.com/gazebosim/gz-math/pull/390)
    * [Pull request #450](https://github.com/gazebosim/gz-math/pull/450)

1. Add `Interval` class
    * [Pull request #388](https://github.com/gazebosim/gz-math/pull/388)
    * [Pull request #449](https://github.com/gazebosim/gz-math/pull/449)
    * [Pull request #396](https://github.com/gazebosim/gz-math/pull/396)

1. Add Matrix6 class
    * [Pull request #455](https://github.com/gazebosim/gz-math/pull/455)
    * [Pull request #469](https://github.com/gazebosim/gz-math/pull/469)

1. Backport newest appendToStream functions
    * [Pull request #453](https://github.com/gazebosim/gz-math/pull/453)

1. Use pytest to generate junit xml files for python tests
    * [Pull request #446](https://github.com/gazebosim/gz-math/pull/446)

1. Add Aditya as codeowner
    * [Pull request #443](https://github.com/gazebosim/gz-math/pull/443)

## Gazebo Math 6.11.0 (2022-05-11)

1. MassMatrix3: fix bug in PrincipalAxesOffset tolerances
    * [Pull request #424](https://github.com/gazebosim/gz-math/pull/424)

1. Fix return policies for some member functions
    * [Pull request #422](https://github.com/gazebosim/gz-math/pull/422)

1. Added Ellipsoid Python interface
    * [Pull request #404](https://github.com/gazebosim/gz-math/pull/404)

1. Added Capsule Python interface
    * [Pull request #403](https://github.com/gazebosim/gz-math/pull/403)

1. Fixes for tests on i386: relax SphericalCoordinates and workaround for negative zero
    * [Pull request #374](https://github.com/gazebosim/gz-math/pull/374)

1. Added helper function to check if a string represents a time
    * [Pull request #389](https://github.com/gazebosim/gz-math/pull/389)

1. Reduce pybind11 compilation memory
    * [Pull request #382](https://github.com/gazebosim/gz-math/pull/382)
    * [Pull request #373](https://github.com/gazebosim/gz-math/pull/373)
    * [Pull request #371](https://github.com/gazebosim/gz-math/pull/371)

## Gazebo Math 6.10.0 (2022-01-26)

1. Use const instead of constexpr in Ellipsoid constructor
    * [Pull request #366](https://github.com/gazebosim/gz-math/pull/366)

1. Refactor finding pybind11
    * [Pull request #360](https://github.com/gazebosim/gz-math/pull/360)

1. Fix Focal on Jenkins
    * [Pull request #364](https://github.com/gazebosim/gz-math/pull/364)

1. kmeans example in C++ and Python
    * [Pull request #356](https://github.com/gazebosim/gz-math/pull/356)

1. Small fixed in doxygen
    * [Pull request #355](https://github.com/gazebosim/gz-math/pull/355)

1. Added Python Getting started tutorial
    * [Pull request #362](https://github.com/gazebosim/gz-math/pull/362)

1. Move SWIG interfaces from Python to Ruby
    * [Pull request #354](https://github.com/gazebosim/gz-math/pull/354)

1. Added pybind11 interfaces for various classes
    1. SphericalCoordinates
        * [Pull request #357](https://github.com/gazebosim/gz-math/pull/357)
    1. Vector3Stats
        * [Pull request #351](https://github.com/gazebosim/gz-math/pull/351)
    1. SignalStats
        * [Pull request #343](https://github.com/gazebosim/gz-math/pull/343)
    1. Sphere
        * [Pull request #352](https://github.com/gazebosim/gz-math/pull/352)
    1. Frustum
        * [Pull request #353](https://github.com/gazebosim/gz-math/pull/353)
    1. Plane
        * [Pull request #346](https://github.com/gazebosim/gz-math/pull/346)
    1. Cylinder
        * [Pull request #348](https://github.com/gazebosim/gz-math/pull/348)
    1. OrientedBox
        * [Pull request #276](https://github.com/gazebosim/gz-math/pull/276)
        * [Pull request #350](https://github.com/gazebosim/gz-math/pull/350)
    1. Inertial
        * [Pull request #349](https://github.com/gazebosim/gz-math/pull/349)
    1. Matrix4
        * [Pull request #337](https://github.com/gazebosim/gz-math/pull/337)
    1. PID
        * [Pull request #323](https://github.com/gazebosim/gz-math/pull/323)
        * [Pull request #361](https://github.com/gazebosim/gz-math/pull/361)
    1. Temperature
        * [Pull request #330](https://github.com/gazebosim/gz-math/pull/330)
    1. DiffDriveOdometry (with examples)
        * [Pull request #314](https://github.com/gazebosim/gz-math/pull/314)
    1. MassMatrix3
        * [Pull request #345](https://github.com/gazebosim/gz-math/pull/345)
    1. AxisAlignedBox
        * [Pull request #338](https://github.com/gazebosim/gz-math/pull/338)
        * [Pull request #281](https://github.com/gazebosim/gz-math/pull/281)
    1. GaussMarkovProcess (with examples)
        * [Pull request #315](https://github.com/gazebosim/gz-math/pull/315)
    1. RotationSpline
        * [Pull request #339](https://github.com/gazebosim/gz-math/pull/339)
    1. Material
        * [Pull request #340](https://github.com/gazebosim/gz-math/pull/340)
    1. Kmeans
        * [Pull request #341](https://github.com/gazebosim/gz-math/pull/341)
    1. Triangle3
        * [Pull request #335](https://github.com/gazebosim/gz-math/pull/335)
    1. Pose3
        * [Pull request #334](https://github.com/gazebosim/gz-math/pull/334)
    1. Triangle
        * [Pull request #333](https://github.com/gazebosim/gz-math/pull/333)
    1. Spline
        * [Pull request #332](https://github.com/gazebosim/gz-math/pull/332)
    1. Filter
        * [Pull request #336](https://github.com/gazebosim/gz-math/pull/336)
    1. SemanticVersion
        * [Pull request #331](https://github.com/gazebosim/gz-math/pull/331)
    1. Matrix3
        * [Pull request #325](https://github.com/gazebosim/gz-math/pull/325)
    1. MovingWindowFilter
        * [Pull request #321](https://github.com/gazebosim/gz-math/pull/321)
    1. Line3
        * [Pull request #317](https://github.com/gazebosim/gz-math/pull/317)
    1. Quaternion
        * [Pull request #324](https://github.com/gazebosim/gz-math/pull/324)
        * [Pull request #361](https://github.com/gazebosim/gz-math/pull/361)
    1. StopWatch
        * [Pull request #319](https://github.com/gazebosim/gz-math/pull/319)
    1. RollingMean
        * [Pull request #322](https://github.com/gazebosim/gz-math/pull/322)
    1. Line2
        * [Pull request #316](https://github.com/gazebosim/gz-math/pull/316)
    1. Color
        * [Pull request #318](https://github.com/gazebosim/gz-math/pull/318)
    1. Helpers
        * [Pull request #313](https://github.com/gazebosim/gz-math/pull/313)
    1. Rand (with examples)
        * [Pull request #312](https://github.com/gazebosim/gz-math/pull/312)
    1. Angle
        * [Pull request #311](https://github.com/gazebosim/gz-math/pull/311)
    1. Vector2, Vector3 and Vector4
        * [Pull request #280](https://github.com/gazebosim/gz-math/pull/280)

1. Fix Color::HSV() incorrect hue output
    * [Pull request #320](https://github.com/gazebosim/gz-math/pull/320)

1. Add example and modify document for class Color
    * [Pull request #304](https://github.com/gazebosim/gz-math/pull/304)

1. Document that euler angles should be in radians for quaternion constructor
    * [Pull request #298](https://github.com/gazebosim/gz-math/pull/298)

1. Fix windows warnings in Vector2, 3 and 4
    * [Pull request #284](https://github.com/gazebosim/gz-math/pull/284)

1. Modified cmake target name for Ruby interfaces
    * [Pull request #285](https://github.com/gazebosim/gz-math/pull/285)

1. Frustrum Python interface
    * [Pull request #278](https://github.com/gazebosim/gz-math/pull/278)

1. quaternion_from_euler example: input degrees
    * [Pull request #282](https://github.com/gazebosim/gz-math/pull/282)

1. Internal URL fixed (paragraph 266)
    * [Pull request #279](https://github.com/gazebosim/gz-math/pull/279)

1. Added tutorials for vector, angle, triangle and rotation
    * [Pull request #249](https://github.com/gazebosim/gz-math/pull/249)

1. Inertial Python interface
    * [Pull request #275](https://github.com/gazebosim/gz-math/pull/275)

1. Box Python interfaces
    * [Pull request #273](https://github.com/gazebosim/gz-math/pull/273)

1. DiffDriveOdometry Python interface
    * [Pull request #265](https://github.com/gazebosim/gz-math/pull/265)

1. Sphere Python interface
    * [Pull request #277](https://github.com/gazebosim/gz-math/pull/277)

1. Plane Python interfaces
    * [Pull request #272](https://github.com/gazebosim/gz-math/pull/272)

1. Cylinder Python interface
    * [Pull request #274](https://github.com/gazebosim/gz-math/pull/274)

1. Added SphericalCoordinates Python interface
    * [Pull request #263](https://github.com/gazebosim/gz-math/pull/263)

1. MassMatrix3 Python interface
    * [Pull request #260](https://github.com/gazebosim/gz-math/pull/260)

1. AxisAlignedBox Python interface
    * [Pull request #262](https://github.com/gazebosim/gz-math/pull/262)

1. AxisAlignedBox: deprecate unimplemented methods
    * [Pull request #261](https://github.com/gazebosim/gz-math/pull/261)

## Gazebo Math 6.9.2 (2021-10-14)

1. Added StopWatch Python Interface
    * [Pull request #264](https://github.com/gazebosim/gz-math/pull/264)

1. Fix clang warnings.
    * [Pull request #267](https://github.com/gazebosim/gz-math/pull/267)

1. Fixed Helpers Python templates
    * [Pull request #266](https://github.com/gazebosim/gz-math/pull/266)

1. Add Helpers Python interface
    * [Pull request #251](https://github.com/gazebosim/gz-math/pull/251)

1. Add Python interface to Triangle3
    * [Pull request #247](https://github.com/gazebosim/gz-math/pull/247)

1. Adds python interface to MaterialType and Material.
    * [Pull request #234](https://github.com/gazebosim/gz-math/pull/234)

1. Remove Cylinder::SetLength const method
    * [Pull request #259](https://github.com/gazebosim/gz-math/pull/259)

## Gazebo Math 6.9.1 (2021-09-30)

1. Avoid assertAlmostEqual for python strings
    * [Pull request #255](https://github.com/gazebosim/gz-math/pull/255)

1. Pose3_TEST.py: use 0.01 (not 0) in string test
    * [Pull request #257](https://github.com/gazebosim/gz-math/pull/257)

## Gazebo Math 6.9.0 (2021-09-28)

1. Volume below a plane for spheres and boxes
    * [Pull request #219](https://github.com/gazebosim/gz-math/pull/219)

1. 🌐 Spherical coordinates: bug fix, docs and sanity checks
    * [Pull request #235](https://github.com/gazebosim/gz-math/pull/235)

1. Add Vector(2|3|4)<T>::NaN to easily create invalid vectors
    * [Pull request #222](https://github.com/gazebosim/gz-math/pull/222)

1. Add options to install python/ruby in system standard paths
    * [Pull request #236](https://github.com/gazebosim/gz-math/pull/236)

1. Add eigen utils to convert mesh 3d vertices to oriented box
    * [Pull request #224](https://github.com/gazebosim/gz-math/pull/224)

1. Python interface

    1. Adds python interface to RollingMean, Color and Spline
        * [Pull request #218](https://github.com/gazebosim/gz-math/pull/218)

    1. Adds python interface for Kmeans and Vector3Stats
        * [Pull request #232](https://github.com/gazebosim/gz-math/pull/232)

    1. Adds python interface to PID and SemanticVersion.
        * [Pull request #229](https://github.com/gazebosim/gz-math/pull/229)

    1. Adds python interface to triangle.
        * [Pull request #231](https://github.com/gazebosim/gz-math/pull/231)

    1. Adds Line2, Line3, SignalStats, Temperature python interface
        * [Pull request #220](https://github.com/gazebosim/gz-math/pull/220)

    1. Python interface: Renames methods to match PEP8 style
        * [Pull request #226](https://github.com/gazebosim/gz-math/pull/226)

    1. Adds python interface to Filter, MovingWindowFilter, RotationSpline.
        * [Pull request #230](https://github.com/gazebosim/gz-math/pull/230)

    1. Adds python interface to Quaternion, Pose3, Matrix3 and Matrix4
        * [Pull request #221](https://github.com/gazebosim/gz-math/pull/221)

    1. Basic setup for Python interface using SWIG
        * [Pull request #216](https://github.com/gazebosim/gz-math/pull/216)
        * [Pull request #223](https://github.com/gazebosim/gz-math/pull/223)
        * [Pull request #208](https://github.com/gazebosim/gz-math/pull/208)
        * [Pull request #239](https://github.com/gazebosim/gz-math/pull/239)

1. 👩‍🌾 Don't use std::pow with integers in Vectors and handle sqrt
    * [Pull request #207](https://github.com/gazebosim/gz-math/pull/207)

1. Relax expectations about zero in SpeedLimiter_TEST to make ARM happy
    * [Pull request #204](https://github.com/gazebosim/gz-math/pull/204)

1. Infrastructure
    * [Pull request #242](https://github.com/gazebosim/gz-math/pull/242)
    * [Pull request #217](https://github.com/gazebosim/gz-math/pull/217)
    * [Pull request #211](https://github.com/gazebosim/gz-math/pull/211)
    * [Pull request #209](https://github.com/gazebosim/gz-math/pull/209)
    * [Pull request #227](https://github.com/gazebosim/gz-math/pull/227)
    * [Pull request #225](https://github.com/gazebosim/gz-math/pull/225)
    * [Pull request #252](https://github.com/gazebosim/gz-math/pull/252)
    * [Pull request #253](https://github.com/gazebosim/gz-math/pull/253)

## Gazebo Math 6.8.0 (2021-03-30)

1. Add speed limiter class
    * [Pull request #194](https://github.com/gazebosim/gz-math/pull/194)

1. Bazel Updates for math6
    * [Pull request #171](https://github.com/gazebosim/gz-math/pull/171)

1. Add Equal tolerance method to Quaternion
    * [Pull request #196](https://github.com/gazebosim/gz-math/pull/196)

1. Fix broken link in MassMatrix3.hh
    * [Pull request #197](https://github.com/gazebosim/gz-math/pull/197)

1. Add instructions to build and run examples
    * [Pull request #192](https://github.com/gazebosim/gz-math/pull/192)

1. Infrastructure and documentation
    * [Pull request #189](https://github.com/gazebosim/gz-math/pull/189)
    * [Pull request #193](https://github.com/gazebosim/gz-math/pull/193)
    * [Pull request #195](https://github.com/gazebosim/gz-math/pull/195)
    * [Pull request #201](https://github.com/gazebosim/gz-math/pull/201)

1. Remove unnecessary copy constructor declaration from Box
    * [Pull request 187](https://github.com/gazebosim/gz-math/pull/187)

1. Windows installation via conda-forge
    * [Pull request 185](https://github.com/gazebosim/gz-math/pull/185)

1.  Add rule-of-five members for Angle
    * [Pull request 186](https://github.com/gazebosim/gz-math/pull/186)

1. Ellipsoid: new shape class with inertia calculation method
    * [Pull request 182](https://github.com/gazebosim/gz-math/pull/182)

1. Avoid moving a return value, it might prevent (N)RVO
    * [Pull request 183](https://github.com/gazebosim/gz-math/pull/183)

1. Properly handle stream errors when reading math objects
    * [Pull request 180](https://github.com/gazebosim/gz-math/pull/180)
    * [Pull request 181](https://github.com/gazebosim/gz-math/pull/181)

## Gazebo Math 6.7.0 (2020-11-23)

1. Capsule: new shape class with inertia calculation method
    * [Pull request 163](https://github.com/gazebosim/gz-math/pull/163)

1. Add missing header to Color.hh
    * [Pull request 162](https://github.com/gazebosim/gz-math/pull/162)

1. Improve tests of `Vector2`, `Vector3`, `Vector4`, `Quaternion`, and `Pose3`
    * [Pull request 172](https://github.com/gazebosim/gz-math/pull/172)
    * [Pull request 173](https://github.com/gazebosim/gz-math/pull/173)
    * [Pull request 174](https://github.com/gazebosim/gz-math/pull/174)
    * [Issue 76](https://github.com/gazebosim/gz-math/issues/76)

1. Pose3: document `operator*`
    * [Pull request 170](https://github.com/gazebosim/gz-math/pull/170)

1. Quaternion: add Normalized() method
    * [Pull request 169](https://github.com/gazebosim/gz-math/pull/169)

1. Vector2: add Round(), Rounded() methods
    * [Pull request 166](https://github.com/gazebosim/gz-math/pull/166)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Add test for printing `inf` `Vector3`
    * [Pull request 168](https://github.com/gazebosim/gz-math/pull/168)
    * [Issue 64](https://github.com/gazebosim/gz-math/issues/64)

## Gazebo Math 6.6.0 (2020-09-16)

1. Add chrono duration helper functions
    * [Pull request 158](https://github.com/gazebosim/gz-math/pull/158)

## Gazebo Math 6.5.0 (2020-09-04)

1. Add string to time function
    * [Pull request 152](https://github.com/gazebosim/gz-math/pull/152)

1. Added functions to convert between time_point and secNsec
    * [Pull request 150](https://github.com/gazebosim/gz-math/pull/150)

1. Fix GZ_MATH_XXX_VERSION
    * [Pull request 151](https://github.com/gazebosim/gz-math/pull/151)

1. Add Max and Min function to Vector2.hh
    * [Pull request 133](https://github.com/gazebosim/gz-math/pull/133)
    * [Pull request 148](https://github.com/gazebosim/gz-math/pull/148)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Round, Rounded, Correct, Distance(x, y, z, w) and operator< addition to Vector 4
    * [Pull request 146](https://github.com/gazebosim/gz-math/pull/146)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Sum and normalized functions for Vector4
    * [Pull request 140](https://github.com/gazebosim/gz-math/pull/140)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Vector4 Ruby tests - Vector4.i and Vector4_TEST.rb
    * [Pull request 137](https://github.com/gazebosim/gz-math/pull/137)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Vector3 to vector4 functions
    * [Pull request 132](https://github.com/gazebosim/gz-math/pull/132)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Update vector2 functions from vector3
    * [Pull request 130](https://github.com/gazebosim/gz-math/pull/130)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Add Abs, Dot and AbsDot and respective tests to Vector4
    * [Pull request 135](https://github.com/gazebosim/gz-math/pull/135)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Create abs, absDot and Correct functions for Vector2d
    * [Pull request 143](https://github.com/gazebosim/gz-math/pull/143)
    * [Issue 71](https://github.com/gazebosim/gz-math/issues/71)

1. Document Ruby tests
    * [Pull request 145](https://github.com/gazebosim/gz-math/pull/145)

1. Add header for numeric_limits allowing build on ubuntu 16.04
    * [Pull request 119](https://github.com/gazebosim/gz-math/pull/119)

1. Add setter/getter for Pose's each element
    * [Pull request 125](https://github.com/gazebosim/gz-math/pull/125)
    * [Issue 35](https://github.com/gazebosim/gz-math/issues/35)

1. Implement AxisAlignedBox Volume function
    * [Pull request 126](https://github.com/gazebosim/gz-math/pull/126)

1. Add operator + for AxisAlignedBox and Vector3.
    * [Pull request 122](https://github.com/gazebosim/gz-math/pull/122)

1. Make alpha optional when parsing a Color from an input stream.
    * [Pull request 106](https://github.com/gazebosim/gz-math/pull/106)

1. GitHub actions CI and workflow updates
    * [Pull request 117](https://github.com/gazebosim/gz-math/pull/117)
    * [Pull request 139](https://github.com/gazebosim/gz-math/pull/139)
    * [Pull request 110](https://github.com/gazebosim/gz-math/pull/110)

1. Added a Gauss-Markov Process class.
    * [BitBucket pull request 342](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/342)

1. Fix bug in Inertial addition of off-diagonal moment of inertia terms with pose offsets.
    * [BitBucket pull request 344](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/344)

1. Create FAKE_INSTALL target so example compilation can be tested without doing a real install.
    * Angle: [BitBucket pull request 335](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/335)

1. Updating documentation.
    * Angle: [BitBucket pull request 325](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/325)

1. GraphAlgorithms: add ToUndirected(DirectedGraph) that copies to an UndirectedGraph.
    * [BitBucket pull request 332](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/332)

1. Doxygen fixes for graph classes
    * [BitBucket pull request 331](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/331)

### Gazebo Math 6.4.0

1. Added a function that rounds up a number to the nearest multiple of
   another number.
    * [BitBucket pull request 318](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/318)

### Gazebo Math 6.3.0

1.  Added Odometry class that computes odometry for a two wheeled vehicle.
    * [BitBucket pull request 313](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/313)

1.  Added RollingMean class.
    * [BitBucket pull request 314](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/314)

### Gazebo Math 6.2.0

1.  eigen3: Use linear() instead of rotation() to prevent computation of SVD
    * [BitBucket pull request 311](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/311)

1. Change definition of Pose3 `*` operator to fix multiplication order
    * [BitBucket pull request 301](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/301)
    * [Issue 60](https://github.com/gazebosim/gz-math/issues/60)

### Gazebo Math 6.1.0

1. eigen3: add conversion functions for Eigen::AlignedBox3d <=> gz::math::AxisAlignedBox
    * [BitBucket pull request 302](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/302)

### Gazebo Math 6.0.0

1. Helper function that converts from `std::chrono::steady_clock::duration` to
   {seconds, nanoseconds}.
    * [BitBucket pull request XXX](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/XXX)

1. Upgrade to c++17.
    * [BitBucket pull request 268](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/268)

## Gazebo Math 5.x

### Gazebo Math 5.x.x

### Gazebo Math 5.1.0 (2019-09-11)

1. GraphAlgorithms: add ToUndirected(DirectedGraph) that copies to an UndirectedGraph.
    * [BitBucket pull request 332](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/332)

1. Doxygen fixes for graph classes
    * [BitBucket pull request 331](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/331)

1.  eigen3: Use linear() instead of rotation() to prevent computation of SVD
    * [BitBucket pull request 312](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/312)

1. Change definition of Pose3 `*` operator to fix multiplication order
    * [BitBucket pull request 301](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/301)
    * [Issue 60](https://github.com/gazebosim/gz-math/issues/60)

1. eigen3: add conversion functions for Eigen::AlignedBox3d <=> gz::math::AxisAlignedBox
    * [BitBucket pull request 302](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/302)


### Gazebo Math 5.0.0 (2018-12-12)

1. Added a Stopwatch class
    * [BitBucket pull request 279](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/279)

1. Added material properties to OrientedBox
    * [BitBucket pull request 269](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/269)

1. Added a Cylinder class.
    * [BitBucket pull request 250](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/250)

1. The `Box` class has been changed to a templatized class that is not
   axis-aligned. The previous `Box` functionality is now in the
   `AxisAlignedBox` class.
    * [BitBucket pull request 257](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/257)

1. Added eigen3 component with functions for converting between Eigen and gz-math types.
    * [BitBucket pull request 256](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/256)

1. Added a `MassMatrix3::SetFromCylinder` function that uses a `Material`
to specify a density.
    * [BitBucket pull request 248](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/248)

1. Added a Sphere class.
    * [BitBucket pull request 255](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/255)

1. Added a `MassMatrix3::SetFromSphere` function that uses a `Material` to
specify a density.
    * [BitBucket pull request 247](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/247)

1. Added a `MassMatrix3::SetFromBox` function that uses a `Material` to specify
   a density.
    * [BitBucket pull request 246](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/246)

1. Deprecated mutator functions in MassMatrix3 that lacked a `Set` prefix.
    * [BitBucket pull request 262](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/262)

1. Updated the MassMatrix3::ValidMoments(), MassMatrix3::IsValid(), MassMatrix3::IsPositive(),
 and Inertial::SetMassMatrix functions to accept a tolerance parameter.
    * [BitBucket pull request 264](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/264)

1. Add MassMatrix3::IsNearPositive and use it in MassMatrix3::IsValid, use >= instead of >
   in MassMatrix3::ValidMoments
    * [BitBucket pull request 278](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/278)

## Gazebo Math 4.x

### Gazebo Math 4.x.x

1. Add Graph::EdgeFromVertices function that return an edge, if one exists,
   between two vertices.
    * [BitBucket pull request 254](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/254)

1. Added multiply assign operator to Matrix4.
    * [BitBucket pull request 252](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/252)


1. Add Plane copy constructor and fix cppcheck on artful
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/230)

1. Added MovingWindowFilter, a copy from Gazebo Common. This version will
   replace the version found in Gazebo Common.
    * [BitBucket pull request 239](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/239)

1. Added a Material class, which holds information about materials like wood,
   steel, and iron.
    * [BitBucket pull request 243](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/243)

### Gazebo Math 4.0.0 (2017-12-26)

1. Use std::stoi and std::stod in math::parse* functions to reduce code
    * [BitBucket pull request 224](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/224)
    * [Issue 50](https://github.com/gazebosim/gz-math/issues/50)

1. Fixing const-correctness for operator* of Pose3
    * [BitBucket pull request 205](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/205)

1. Deprecate Matrix4::Translate and replace by Matrix4::SetTranslation
    * [BitBucket pull request 222](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/222)

1. Use ignition-cmake to simplify build scripts
    * [BitBucket pull request 200](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/200)

1. Make constructor SemanticVersion(string) explicit
    * [BitBucket pull request 203](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/203)

1. Switch to C++14
    * [BitBucket pull request 180](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/180)

1. Removed the box 'extent' field. The default constructor now sets a box's
   corners to extrema in order to indicate an uninitialized box.
    * [BitBucket pull request 172](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/172)
    * [Issue 72](https://github.com/gazebosim/gz-math/issues/72)
    * [Issue 53](https://github.com/gazebosim/gz-math/issues/53)

1. Added graph utilities:
    1. Added a Vertex class:
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/170)
    1. Added an Edge class:
    * [BitBucket pull request 174](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/174)
    1. Added a Graph class:
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/175)
    1. Added a GraphAlgorithms class:
    * [BitBucket pull request 177](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/177)
    1. Added a function to calculate connected components in undirected
       graphs:
    * [BitBucket pull request 190](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/190)
    1. Improved the performance of `graph::InDegree()` and `graph::IncidentsTo()`.
    * [BitBucket pull request 188](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/188)
    * [Issue 79](https://github.com/gazebosim/gz-math/issues/79)

1. Added Inline Versioned Namespace
    * [BitBucket pull request 216](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/216/)

## Gazebo Math 3.x

### Gazebo Math 3.x.x



### Gazebo Math 3.3.0 (2017-11-27)

1. Fixed frustum falsely saying it contained AABB in some cases
    * [BitBucket pull request 193](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/193)
    * [Issue 78](https://github.com/gazebosim/gz-math/issues/78)

1. Create consistent bracket operators across all Vector# types
    * [BitBucket pull request 181](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/181)

1. Change name to the generic BUILDING_DLL macro to avoid conflicts
    * [BitBucket pull request 173](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/173)

1. Fix some compiler warnings
    * [BitBucket pull request 196](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/196)

1. Suppress gtest warnings
    * [BitBucket pull request 199](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/199)

1. Move private headers to src folder
    * [BitBucket pull request 198](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/198)

1. Update configure.bat
    * [BitBucket pull request 206](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/206)

### Gazebo Math 3.2.0 (2017-05-15)

1. Construct on first use in Rand class
    * [BitBucket pull request 165](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/165)

1. Extended Spline API: derivative interpolation, arc length calculation
   and tangent forcing.
    * [BitBucket pull request 162](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/162)

### Gazebo Math 3.1.0 (2017-04-11)

1. Added signum functions to Helpers.hh.
    * Contribution from Martin Pecka
    * [BitBucket pull request 153](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/153)

### Gazebo Math 3.0.0 (2017-01-05)

1. Deprecate many IGN_* macros in favor of static const variables in Helpers.hh
    * [BitBucket pull request 138](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/138)
    * [BitBucket pull request 137](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/137)

1. Removed exceptions. Return values should be evaluated to determine if
   errors have occurred.
    * [BitBucket pull request 132](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/132)

1. Added `operator=(const Quaternion<T> &_q)` to `Matrix3`.
    * [BitBucket pull request 111](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/111)

1. Fix xenial cppcheck
    * [BitBucket pull request xxx](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/xxx)

1. Require cmake 2.8.12
    * [BitBucket pull request 76](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/76)

1. Migrate to relocatable CMake package.
   Contribution from Silvio Traversaro.
    * [BitBucket pull request 67](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/67)

1. Fix logic of installation of CMake configuration files in Windows.
   Contribution from Silvio Traversaro.
    * [BitBucket pull request 63](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/63)

## Gazebo Math 2.x



## Gazebo Math 2.9 (2017-11-22)

1. Fixed frustum falsely saying it contained AABB in some cases
    * [BitBucket pull request 193](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/193)

1. Added Color
    * [BitBucket pull request 150](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/150)

1. Backport updated configure.bat to gz-math2 and fix cppcheck warnings
    * [BitBucket pull request 207](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/207)

### Gazebo Math 2.8

### Gazebo Math 2.8.0

1. Added OrientedBox
    * [BitBucket pull request 146](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/146)

1. Added an assignment operator to the Frustum class.
    * [BitBucket pull request 144](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/144)

### Gazebo Math 2.7

### Gazebo Math 2.7.0

1. Add static const variables as alternative to macros in Helpers.hh
    * [BitBucket pull request 137](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/137)

1. Add new methods for floating numbers: lessOrEqual and greaterOrEqual
    * [BitBucket pull request 134](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/134)

### Gazebo Math 2.6

### Gazebo Math 2.6.0

1. Added copy constructor, equality operators and assignment operators to
    SphericalCoordinates class.
    * [BitBucket pull request 131](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/131)

1. Fix Euler angle conversion of quaternions near singularities
    * [BitBucket pull request 129](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/129)

1. Backport triangle3, helper functions, equality helper to work with 387 fp unit
   (Contribution from Rich Mattes).
    * [BitBucket pull request 125](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/125)
    * [BitBucket pull request 58](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/58)
    * [BitBucket pull request 56](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/56)

1. Added Matrix4<T>::LookAt
    * [BitBucket pull request 124](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/124)

1. Set Inertial Rotations
    * [BitBucket pull request 121](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/121)

1. Added SemanticVersion class
    * [BitBucket pull request 120](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/120)

### Gazebo Math 2.5

### Gazebo Math 2.5.0

1. Added PID class
    * [BitBucket pull request 117](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/117)

1. Added SphericalCoordinate class
    * [BitBucket pull request 108](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/108)

### Gazebo Math 2.4

#### Gazebo Math 2.4.1

1. Combine inertial properties of different objects, returning the equivalent
   inertial properties as if the objects were welded together.
    * [BitBucket pull request 115](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/115)

#### Gazebo Math 2.4.0

1. New MassMatrix3 class
    * [BitBucket pull request 112](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/112)
1. MassMatrix3 helper functions
    * [BitBucket pull request 110](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/110)
1. Added Temperature class
    * A contribution from Shintaro Noda
    * [BitBucket pull request 113](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/113)

### Gazebo Math 2.3.0

1. Added simple volumes formulas
    * [BitBucket pull request 84](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/84)
1. Add Length and SquaredLength for Vector2 with test
    * [BitBucket pull request 73](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/73)
1. Add Equal function with numerical tolerance argument
    * [BitBucket pull request 75](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/75)
1. First part of MassMatrix3 class, mostly accessors and modifiers
    * [BitBucket pull request 77](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/77)
1. Add Transpose methods for Matrix3,4 with test
    * [BitBucket pull request 74](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/74)
1. Multiplication improvements for Vector/Matrix classes
    * [BitBucket pull request 69](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/69)
1. Scalar +,- operators for Vector[234]
    * [BitBucket pull request 71](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/71)
1. Add Determinant method for Matrix[34]
    * [BitBucket pull request 72](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/72)
1. Fixes for compiling and running tests on Windows 7/Visual Studio 2013
   Contribution from Silvio Traversaro.
    * [BitBucket pull request 62](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-math/pull-requests/62)
