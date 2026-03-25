## Gazebo Plugin 3.x

### Gazebo Plugin 3.1.0 (2025-05-14)

1. Migrate bazel build setup to use bzlmod
    * [Pull request #176](https://github.com/gazebosim/gz-plugin/pull/176)
    * [Pull request #177](https://github.com/gazebosim/gz-plugin/pull/177)

1. ci: run cppcheck, cpplint, doxygen on noble
    * [Pull request #172](https://github.com/gazebosim/gz-plugin/pull/172)

1. used codespell to fix spellcheck errors (#168)
    * [Pull request #169](https://github.com/gazebosim/gz-plugin/pull/#169)

1. Update README.md - fix windows badge
    * [Pull request #166](https://github.com/gazebosim/gz-plugin/pull/166)

### Gazebo Plugin 3.0.1 (2025-02-12)

1. Require cmake 3.10.2 for consistency
    * [Pull request #162](https://github.com/gazebosim/gz-plugin/pull/162)

1. Fix typo in documentation (#150)
    * [Pull request #151](https://github.com/gazebosim/gz-plugin/pull/151)

### Gazebo Plugin 3.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 2.0.3 and earlier.

1. Miscellaneous documentation fixes
    * [Pull request #153](https://github.com/gazebosim/gz-plugin/pull/153)
    * [Pull request #150](https://github.com/gazebosim/gz-plugin/pull/150)

1. Update gz-plugin3 badge URLs
    * [Pull request #148](https://github.com/gazebosim/gz-plugin/pull/148)

1. Ionic changelog
    * [Pull request #147](https://github.com/gazebosim/gz-plugin/pull/147)

1. Enable 24.04 CI, require cmake 3.22.1
    * [Pull request #146](https://github.com/gazebosim/gz-plugin/pull/146)

1. Add package.xml
    * [Pull request #139](https://github.com/gazebosim/gz-plugin/pull/139)

1. Add bazel deps to pass layering_check
    * [Pull request #143](https://github.com/gazebosim/gz-plugin/pull/143)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #135](https://github.com/gazebosim/gz-plugin/pull/135)

1. Use HIDE_SYMBOLS_BY_DEFAULT
    * [Pull request #130](https://github.com/gazebosim/gz-plugin/pull/130)

1. Bumps in ionic: gz-plugin3
    * [Pull request #127](https://github.com/gazebosim/gz-plugin/pull/127)

1. Remove ignition
    * [Pull request #119](https://github.com/gazebosim/gz-plugin/pull/119)


## Gazebo Plugin 2.x

### Gazebo Plugin 2.0.4 (2025-01-30)

1. Fix typo in documentation
    * [Pull request #151](https://github.com/gazebosim/gz-plugin/pull/151)

1. Add package.xml
    * [Pull request #139](https://github.com/gazebosim/gz-plugin/pull/139)

1. Add bazel deps to pass layering_check
    * [Pull request #143](https://github.com/gazebosim/gz-plugin/pull/143)

### Gazebo Plugin 2.0.3 (2024-04-08)

1. Use relative install path for gz tool data
    * [Pull request #140](https://github.com/gazebosim/gz-plugin/pull/140)

1. Remove @mxgrey as codeowner and assign maintainership to @ahcorde
    * [Pull request #137](https://github.com/gazebosim/gz-plugin/pull/137)

1. Update CI badges in README
    * [Pull request #134](https://github.com/gazebosim/gz-plugin/pull/134)

1. Infrastructure
    * [Pull request #132](https://github.com/gazebosim/gz-plugin/pull/132)

1. Enable and fix gz test on windows
    * [Pull request #128](https://github.com/gazebosim/gz-plugin/pull/128)
    * [Pull request #131](https://github.com/gazebosim/gz-plugin/pull/131)

1. Do not install CMakeLists.txt in loader/register
    * [Pull request #123](https://github.com/gazebosim/gz-plugin/pull/123)

### Gazebo Plugin 2.0.2 (2023-09-26)

1. Add explicit dependency on plugin library
    * [Pull request #124](https://github.com/gazebosim/gz-plugin/pull/124)

1. Documentation fixes
    * [Pull request #122](https://github.com/gazebosim/gz-plugin/pull/122)
    * [Pull request #112](https://github.com/gazebosim/gz-plugin/pull/112)

1. Infrastructure
    * [Pull request #121](https://github.com/gazebosim/gz-plugin/pull/121)
    * [Pull request #120](https://github.com/gazebosim/gz-plugin/pull/120)
    * [Pull request #115](https://github.com/gazebosim/gz-plugin/pull/115)

1. Update parameter name in DemangleSymbol to match declaration
    * [Pull request #117](https://github.com/gazebosim/gz-plugin/pull/117)

1. Inline functions in plugin_unload.hh
    * [Pull request #118](https://github.com/gazebosim/gz-plugin/pull/118)

1. Rename COPYING to LICENSE
    * [Pull request #114](https://github.com/gazebosim/gz-plugin/pull/114)

1. Bazel build rules for garden
    * [Pull request #113](https://github.com/gazebosim/gz-plugin/pull/113)


### Gazebo Plugin 2.0.1

1. Forward port 1.4.0.
    * [Pull request #109](https://github.com/gazebosim/gz-plugin/pull/109)

### Gazebo Plugin 2.0.0

1. Add option to use `RTLD_NODELETE` when loading a library.
    * [Pull request #102](https://github.com/gazebosim/gz-plugin/pull/102)

1. Improve install instructions.
    * [Pull request #106](https://github.com/gazebosim/gz-plugin/pull/106)

1. Version bumps for Garden and deprecations.
    * [Pull request #74](https://github.com/gazebosim/gz-plugin/pull/74)
    * [Pull request #76](https://github.com/gazebosim/gz-plugin/pull/76)
    * [Pull request #77](https://github.com/gazebosim/gz-plugin/pull/77)

1. Ignition to Gazebo rename
    * [Pull request #78](https://github.com/gazebosim/gz-plugin/pull/78)
    * [Pull request #79](https://github.com/gazebosim/gz-plugin/pull/79)
    * [Pull request #82](https://github.com/gazebosim/gz-plugin/pull/82)
    * [Pull request #83](https://github.com/gazebosim/gz-plugin/pull/83)
    * [Pull request #86](https://github.com/gazebosim/gz-plugin/pull/86)
    * [Pull request #87](https://github.com/gazebosim/gz-plugin/pull/87)
    * [Pull request #88](https://github.com/gazebosim/gz-plugin/pull/88)
    * [Pull request #89](https://github.com/gazebosim/gz-plugin/pull/89)
    * [Pull request #90](https://github.com/gazebosim/gz-plugin/pull/90)
    * [Pull request #93](https://github.com/gazebosim/gz-plugin/pull/93)
    * [Pull request #104](https://github.com/gazebosim/gz-plugin/pull/104)

1. CLI tool updates.
    * [Pull request #55](https://github.com/gazebosim/gz-plugin/pull/55)
    * [Pull request #65](https://github.com/gazebosim/gz-plugin/pull/65)
    * [Pull request #69](https://github.com/gazebosim/gz-plugin/pull/69)
    * [Pull request #80](https://github.com/gazebosim/gz-plugin/pull/80)
    * [Pull request #92](https://github.com/gazebosim/gz-plugin/pull/92)

1. Update GoogleTest to latest version.
    * [Pull request #91](https://github.com/gazebosim/gz-plugin/pull/91)

1. Clean unused lambda capture warning.
    * [Pull request #95](https://github.com/gazebosim/gz-plugin/pull/95)

1. Support static plugins.
    * [Pull request #97](https://github.com/gazebosim/gz-plugin/pull/97)

## Gazebo Plugin 1.x

### Gazebo Plugin 1.4.0 (2022-11-11)

1. ign -> gz Migrate Ignition Headers : gz-plugin
    * [Pull request #101](https://github.com/gazebosim/gz-plugin/pull/101)

### Gazebo Plugin 1.3.0 (2022-08-15)

1. Remove redundant namespace references
    * [Pull request #100](https://github.com/gazebosim/gz-plugin/pull/100)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #96](https://github.com/gazebosim/gz-plugin/pull/96)

1. Ignition -> Gazebo
    * [Pull request #94](https://github.com/gazebosim/gz-plugin/pull/94)

1. Bash completion for flags
    * [Pull request #81](https://github.com/gazebosim/gz-plugin/pull/81)

1. Add LICENSE file
    * [Pull request #72](https://github.com/gazebosim/gz-plugin/pull/72)

1. Add Ubuntu Jammy CI
    * [Pull request #68](https://github.com/gazebosim/gz-plugin/pull/68)

1. Install ruby commands on Windows
    * [Pull request #63](https://github.com/gazebosim/gz-plugin/pull/63)

### Gazebo Plugin 1.2.1 (2021-09-24)

1. Add Doxygen documentation checks to CI
    * [Pull request #59](https://github.com/gazebosim/gz-plugin/pull/59)

1. Infrastructure
    * [Pull request #58](https://github.com/gazebosim/gz-plugin/pull/58)
    * [Pull request #52](https://github.com/gazebosim/gz-plugin/pull/52)
    * [Pull request #51](https://github.com/gazebosim/gz-plugin/pull/51)

1. Find gz program instead of gz-tools
    * [Pull request #57](https://github.com/gazebosim/gz-plugin/pull/57)

1. Remove gz-tools from CMakeLists.txt. Not used
    * [Pull request #56](https://github.com/gazebosim/gz-plugin/pull/56)

### Gazebo Plugin 1.2.0 (2021-03-30)

1. Infrastructure
    * [Pull request #45](https://github.com/gazebosim/gz-plugin/pull/45)
    * [Pull request #44](https://github.com/gazebosim/gz-plugin/pull/44)
    * [Pull request #43](https://github.com/gazebosim/gz-plugin/pull/43)
    * [Pull request #36](https://github.com/gazebosim/gz-plugin/pull/36)
    * [Pull request #28](https://github.com/gazebosim/gz-plugin/pull/28)
    * [Pull request #26](https://github.com/gazebosim/gz-plugin/pull/26)
    * [Pull request #15](https://github.com/gazebosim/gz-plugin/pull/15)
    * [Pull request #10](https://github.com/gazebosim/gz-plugin/pull/10)
    * [Pull request #14](https://github.com/gazebosim/gz-plugin/pull/14)
    * [Pull request #8](https://github.com/gazebosim/gz-plugin/pull/8)
    * [Pull request #47](https://github.com/gazebosim/gz-plugin/pull/47)

1. Include windows instructions in examples README.md and fix examples to compile them on Windows
    * [Pull request #48](https://github.com/gazebosim/gz-plugin/pull/48)

1. Fix downstream Windows builds
    * [Pull request #38](https://github.com/gazebosim/gz-plugin/pull/38)

1. Add `gz plugin --info` to print plugin info
    * [Pull request #32](https://github.com/gazebosim/gz-plugin/pull/32)

1. Add Windows installation
    * [Pull request #34](https://github.com/gazebosim/gz-plugin/pull/34)

1. Add plugin name accessor
    * [Pull request #29](https://github.com/gazebosim/gz-plugin/pull/29)

### Gazebo Plugin 1.1.0 (2019-11-21)

1. Add SelectSpecializers to allow gz-physics to improve compilation time.
    * [BitBucket pull request 32](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-physics/pull-requests/32)

### Gazebo Plugin 1.0.0 (2019-02-05)

1. Initial release

