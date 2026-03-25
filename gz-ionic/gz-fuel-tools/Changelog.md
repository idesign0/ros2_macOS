## Gazebo Fuel Tools 10.x

### Gazebo Fuel Tools 10.1.0 (2025-06-06)

1. Migrate bazel build setup to use bzlmod
    * [Pull request #466](https://github.com/gazebosim/gz-fuel-tools/pull/466)
    * [Pull request #467](https://github.com/gazebosim/gz-fuel-tools/pull/467)

1. ci: run cppcheck, cpplint, doxygen on noble
    * [Pull request #462](https://github.com/gazebosim/gz-fuel-tools/pull/462)

1. Fix return value when download fails
    * [Pull request #459](https://github.com/gazebosim/gz-fuel-tools/pull/459)

1. Fix code documentation spelling (#458)
    * [Pull request #460](https://github.com/gazebosim/gz-fuel-tools/pull/460)

### Gazebo Fuel Tools 10.0.1 (2025-02-12)

1. Fixed cmd on Windows
    * [Pull request #446](https://github.com/gazebosim/gz-fuel-tools/pull/446)

### Gazebo Fuel Tools 10.0.0 (2024-09-25)

1. **Baseline:** this includes all changes from 9.1.0 and earlier.

1. Miscellaneous documentation fixes
    * [Pull request #445](https://github.com/gazebosim/gz-fuel-tools/pull/445)
    * [Pull request #444](https://github.com/gazebosim/gz-fuel-tools/pull/444)
    * [Pull request #443](https://github.com/gazebosim/gz-fuel-tools/pull/443)
    * [Pull request #442](https://github.com/gazebosim/gz-fuel-tools/pull/442)

1. Update badges to point to gz-fuel-tools10
    * [Pull request #439](https://github.com/gazebosim/gz-fuel-tools/pull/439)

1. Ionic changelog
    * [Pull request #438](https://github.com/gazebosim/gz-fuel-tools/pull/438)

1. Require cmake 3.22.1
    * [Pull request #436](https://github.com/gazebosim/gz-fuel-tools/pull/436)

1. Add missing URI header files
    * [Pull request #435](https://github.com/gazebosim/gz-fuel-tools/pull/435)

1. Use Ubuntu 24.04 in GitHub Actions CI workflow
    * [Pull request #427](https://github.com/gazebosim/gz-fuel-tools/pull/427)

1. Remove HIDE_SYMBOLS_BY_DEFAULT: replace by a default configuration in gz-cmake.
    * [Pull request #399](https://github.com/gazebosim/gz-fuel-tools/pull/399)

1. Use HIDE_SYMBOLS_BY_DEFAULT
    * [Pull request #389](https://github.com/gazebosim/gz-fuel-tools/pull/389)

1. Split ServerConfig and ClientConfig across multiple files
    * [Pull request #380](https://github.com/gazebosim/gz-fuel-tools/pull/380)

1. Bumps in Ionic: gz-fuel-tools10
    * [Pull request #385](https://github.com/gazebosim/gz-fuel-tools/pull/385)
    * [Pull request #386](https://github.com/gazebosim/gz-fuel-tools/pull/386)

1. Remove deprecated ignition files
    * [Pull request #379](https://github.com/gazebosim/gz-fuel-tools/pull/379)

## Gazebo Fuel Tools 9.x

### Gazebo Fuel Tools 9.1.0 (2024-08-06)

1. Make `CollectionIdentifier::UniqueName` consistent
    * [Pull request #430](https://github.com/gazebosim/gz-fuel-tools/pull/430)

1. Add Url accessor to Identifiers
    * [Pull request #429](https://github.com/gazebosim/gz-fuel-tools/pull/429)

1. Migrate curl_formadd from form API to mime API (deprecated in Ubuntu Noble)
    * [Pull request #415](https://github.com/gazebosim/gz-fuel-tools/pull/415)

1. Add package.xml
    * [Pull request #408](https://github.com/gazebosim/gz-fuel-tools/pull/408)

1. CLI for creating config.yaml
    * [Pull request #413](https://github.com/gazebosim/gz-fuel-tools/pull/413)

1. Clean all ASAN reported memory problems (curl related)
    * [Pull request #416](https://github.com/gazebosim/gz-fuel-tools/pull/416)

1. Add Private function to world identifier
    * [Pull request #414](https://github.com/gazebosim/gz-fuel-tools/pull/414)

1. Use config.yaml file from default cache location, if it exists
    * [Pull request #410](https://github.com/gazebosim/gz-fuel-tools/pull/410)

### Gazebo Fuel Tools 9.0.3 (2024-04-09)

1. Use relative install path for gz tool data
    * [Pull request #409](https://github.com/gazebosim/gz-fuel-tools/pull/409)

### Gazebo Fuel Tools 9.0.2 (2024-03-18)

1. Fix `LocalCache` so that models/worlds downloaded via fuel.ignitionrobotics.org can be found in the cache
    * [Pull request #406](https://github.com/gazebosim/gz-fuel-tools/pull/406)

### Gazebo Fuel Tools 9.0.1 (2024-03-14)

1. Tidy nested namespaces
    * [Pull request #396](https://github.com/gazebosim/gz-fuel-tools/pull/396)

1. Update CI badges in README
    * [Pull request #393](https://github.com/gazebosim/gz-fuel-tools/pull/393)

1. Create directories and more output on fail
    * [Pull request #392](https://github.com/gazebosim/gz-fuel-tools/pull/392)

1. Disable tests that are known to fail on Windows
    * [Pull request #387](https://github.com/gazebosim/gz-fuel-tools/pull/387)

1. Update GitHub action workflows
    * [Pull request #388](https://github.com/gazebosim/gz-fuel-tools/pull/388)
    * [Pull request #390](https://github.com/gazebosim/gz-fuel-tools/pull/390)

1. Re-enabling Windows tests
    * [Pull request #376](https://github.com/gazebosim/gz-fuel-tools/pull/376)

### Gazebo Fuel Tools 9.0.0 (2023-09-29)

1. Added script to update assets to gz
    * [Pull request #377](https://github.com/gazebosim/gz-fuel-tools/pull/377)

1. Documentation fixes
    * [Pull request #371](https://github.com/gazebosim/gz-fuel-tools/pull/371)
    * [Pull request #372](https://github.com/gazebosim/gz-fuel-tools/pull/372)
    * [Pull request #375](https://github.com/gazebosim/gz-fuel-tools/pull/375)

1. ign -> gz
    * [Pull request #364](https://github.com/gazebosim/gz-fuel-tools/pull/364)
    * [Pull request #375](https://github.com/gazebosim/gz-fuel-tools/pull/375)

1. Remove GZ_FUEL_INITIAL_CONFIG_PATH macro
    * [Pull request #363](https://github.com/gazebosim/gz-fuel-tools/pull/363)

1. Infrastructure
    * [Pull request #323](https://github.com/gazebosim/gz-fuel-tools/pull/323)
    * [Pull request #374](https://github.com/gazebosim/gz-fuel-tools/pull/374)

1. Update package dependencies for harmonic (gz-msgs10)
    * [Pull request #326](https://github.com/gazebosim/gz-fuel-tools/pull/326)

1. ⬆️  Bump main to 9.0.0~pre1
    * [Pull request #283](https://github.com/gazebosim/gz-fuel-tools/pull/283)

## Gazebo Fuel Tools 8.x

### Gazebo Fuel Tools 8.1.0 (2023-08-21)

1. Use `pull_request_target` for triage workflow
    * [Pull request #367](https://github.com/gazebosim/gz-fuel-tools/pull/367)

1. Fix Github project automation for new project board
    * [Pull request #366](https://github.com/gazebosim/gz-fuel-tools/pull/366)

1. Zip: use non-deprecated methods
    * [Pull request #360](https://github.com/gazebosim/gz-fuel-tools/pull/360)

1. Deprecated non-relocatable macro
    * [Pull request #352](https://github.com/gazebosim/gz-fuel-tools/pull/352)

1. Support for bazel in Garden
    * [Pull request #328](https://github.com/gazebosim/gz-fuel-tools/pull/328)

1. Minor cleanup
    * [Pull request #357](https://github.com/gazebosim/gz-fuel-tools/pull/357)

1. 7.3.0 Release
    * [Pull request #359](https://github.com/gazebosim/gz-fuel-tools/pull/359)

1. Add bash completion
    * [Pull request #329](https://github.com/gazebosim/gz-fuel-tools/pull/329)

1. Reflect pagination of RESTful requests in iterator API
    * [Pull request #350](https://github.com/gazebosim/gz-fuel-tools/pull/350)

1. Disable all LocalCache Tests for Windows
    * [Pull request #351](https://github.com/gazebosim/gz-fuel-tools/pull/351)

1. Support link referral download
    * [Pull request #333](https://github.com/gazebosim/gz-fuel-tools/pull/333)

### Gazebo Fuel Tools 8.0.2 (2023-03-30)

1. Support link referral download
    * [Pull request #333](https://github.com/gazebosim/gz-fuel-tools/pull/333)

1. Allow Curl redirect on HTTP requests.
    * [Pull request #331](https://github.com/gazebosim/gz-fuel-tools/pull/331)

1. CI and license updates.
    * [Pull request #334](https://github.com/gazebosim/gz-fuel-tools/pull/334)
    * [Pull request #335](https://github.com/gazebosim/gz-fuel-tools/pull/335)

1. Change backpack model version 2->3.
    * [Pull request #319](https://github.com/gazebosim/gz-fuel-tools/pull/319)

1. Fix model downloads for ignitionrobotics.org URIs.
    * [Pull request #318](https://github.com/gazebosim/gz-fuel-tools/pull/318)

1. Fix build when `JSON_USE_EXCEPTION == 0`
    * [Pull request #332](https: //github.com/gazebosim/gz-fuel-tools/pull/332)

### Gazebo Fuel Tools 8.0.1

1. Forward ports.
    * [Pull request #306](https://github.com/gazebosim/gz-fuel-tools/pull/306)
    * [Pull request #310](https://github.com/gazebosim/gz-fuel-tools/pull/310)

1. Remove fuel.ignitionrobotics.org from ClientConfig.
    * [Pull request #293](https://github.com/gazebosim/gz-fuel-tools/pull/293)

1. Clean up a few Windows warnings.
    * [Pull request #308](https://github.com/gazebosim/gz-fuel-tools/pull/308)

### Gazebo Fuel Tools 8.0.0

1. Update test infrastructure
    * [Pull request #245](https://github.com/gazebosim/gz-fuel-tools/pull/245)
    * [Pull request #246](https://github.com/gazebosim/gz-fuel-tools/pull/246)
    * [Pull request #269](https://github.com/gazebosim/gz-fuel-tools/pull/269)
    * [Pull request #270](https://github.com/gazebosim/gz-fuel-tools/pull/270)

1. Ignition to Gazebo rename.
    * [Pull request #248](https://github.com/gazebosim/gz-fuel-tools/pull/248)
    * [Pull request #251](https://github.com/gazebosim/gz-fuel-tools/pull/251)
    * [Pull request #256](https://github.com/gazebosim/gz-fuel-tools/pull/256)
    * [Pull request #259](https://github.com/gazebosim/gz-fuel-tools/pull/259)
    * [Pull request #262](https://github.com/gazebosim/gz-fuel-tools/pull/262)
    * [Pull request #264](https://github.com/gazebosim/gz-fuel-tools/pull/264)
    * [Pull request #267](https://github.com/gazebosim/gz-fuel-tools/pull/267)
    * [Pull request #268](https://github.com/gazebosim/gz-fuel-tools/pull/268)
    * [Pull request #272](https://github.com/gazebosim/gz-fuel-tools/pull/272)
    * [Pull request #275](https://github.com/gazebosim/gz-fuel-tools/pull/275)
    * [Pull request #282](https://github.com/gazebosim/gz-fuel-tools/pull/282)
    * [Pull request #287](https://github.com/gazebosim/gz-fuel-tools/pull/287)
    * [Pull request #295](https://github.com/gazebosim/gz-fuel-tools/pull/295)
    * [Pull request #296](https://github.com/gazebosim/gz-fuel-tools/pull/296)
    * [Pull request #297](https://github.com/gazebosim/gz-fuel-tools/pull/297)

1. Bumps in versions for Garden and removal of deprecations
    * [Pull request #224](https://github.com/gazebosim/gz-fuel-tools/pull/224)
    * [Pull request #229](https://github.com/gazebosim/gz-fuel-tools/pull/229)
    * [Pull request #239](https://github.com/gazebosim/gz-fuel-tools/pull/239)
    * [Pull request #247](https://github.com/gazebosim/gz-fuel-tools/pull/247)

1. Improve documentation
    * [Pull request #298](https://github.com/gazebosim/gz-fuel-tools/pull/298)
    * [Pull request #299](https://github.com/gazebosim/gz-fuel-tools/pull/299)
    * [Pull request #300](https://github.com/gazebosim/gz-fuel-tools/pull/300)

## Gazebo Fuel Tools 7.x

### Gazebo Fuel Tools 7.3.0 (2023-06-13)

1. Forward merges
    * [Pull request #355](https://github.com/gazebosim/gz-fuel-tools/pull/355)

1. Add bash completion
    * [Pull request #329](https://github.com/gazebosim/gz-fuel-tools/pull/329)

1. Reflect pagination of RESTful requests in iterator API
    * [Pull request #350](https://github.com/gazebosim/gz-fuel-tools/pull/350)

1. Support link referral download
    * [Pull request #333](https://github.com/gazebosim/gz-fuel-tools/pull/333)

### Gazebo Fuel Tools 7.2.2 (2023-03-29)

1. Support link referral download
    * [Pull request #333](https://github.com/gazebosim/gz-fuel-tools/pull/333)

### Gazebo Fuel Tools 7.2.1 (2023-03-22)

1. Allow Curl redirect on HTTP requests.
    * [Pull request #331](https://github.com/gazebosim/gz-fuel-tools/pull/331)

1. CI and license updates.
    * [Pull request #334](https://github.com/gazebosim/gz-fuel-tools/pull/334)
    * [Pull request #335](https://github.com/gazebosim/gz-fuel-tools/pull/335)

1. Change backpack model version 2->3.
    * [Pull request #319](https://github.com/gazebosim/gz-fuel-tools/pull/319)

### Gazebo Fuel Tools 7.2.0 (2021-11-17)

1. Forward port 4.7.0.
    * [Pull request #306](https://github.com/gazebosim/gz-fuel-tools/pull/306)

### Gazebo Fuel Tools 7.1.0 (2021-08-16)

1. Ignition to Gazebo server rename effort
    * [Pull request #263](https://github.com/gazebosim/gz-fuel-tools/pull/263)

1. Remove redundant namespace references
    * [Pull request #284](https://github.com/gazebosim/gz-fuel-tools/pull/284)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #278](https://github.com/gazebosim/gz-fuel-tools/pull/278)

1. Ignition -> Gazebo docs
    * [Pull request #276](https://github.com/gazebosim/gz-fuel-tools/pull/276)

1. Remove unused files (`bitbucket-pipelines`, `docs`)
    * [Pull request #273](https://github.com/gazebosim/gz-fuel-tools/pull/273)

1. Disable failing windows tests
    * [Pull request #266](https://github.com/gazebosim/gz-fuel-tools/pull/266)

1. Disable staging-fuel.ignitionrobotics.org test
    * [Pull request #257](https://github.com/gazebosim/gz-fuel-tools/pull/257)

1. Add missing header guard for `Interface.hh`
    * [Pull request #249](https://github.com/gazebosim/gz-fuel-tools/pull/249)

1. SDF from path helper
    * [Pull request #233](https://github.com/gazebosim/gz-fuel-tools/pull/233)

1.  Add Ubuntu Jammy CI
    * [Pull request #236](https://github.com/gazebosim/gz-fuel-tools/pull/236)

1. Fix trailing slashes in failing tests
    * [Pull request #237](https://github.com/gazebosim/gz-fuel-tools/pull/237)

1. Print error when unzipping fails to save a file
    * [Pull request #235](https://github.com/gazebosim/gz-fuel-tools/pull/235)

1. More checks when manipulating file system in tests
    * [Pull request #227](https://github.com/gazebosim/gz-fuel-tools/pull/227)

1. Move test cache to build folder
    * [Pull request #222](https://github.com/gazebosim/gz-fuel-tools/pull/222)

1. Improve and fix `ResultType` tests
    * [Pull request #225](https://github.com/gazebosim/gz-fuel-tools/pull/225)

1. Fix `cmdfuel` ruby script generation on Windows with MSVC
    * [Pull request #219](https://github.com/gazebosim/gz-fuel-tools/pull/219)

1. `FuelClient.cc`: include `<deque>`
    * [Pull request #213](https://github.com/gazebosim/gz-fuel-tools/pull/213)

1. APIs for retrieving models in parallel
    * [Pull request #199](https://github.com/gazebosim/gz-fuel-tools/pull/199)

### Gazebo Fuel Tools 7.0.0 (2021-09-28)

1. Depend on gz-msgs8
    * [Pull request #182](https://github.com/gazebosim/gz-fuel-tools/pull/182)

1. Infrastructure
    * [Pull request #184](https://github.com/gazebosim/gz-fuel-tools/pull/184)
    * [Pull request #181](https://github.com/gazebosim/gz-fuel-tools/pull/181)

## Gazebo Fuel Tools 6.x

### Gazebo Fuel Tools 6.2.0 (2022-03-25)

1. Fix trailing slashes in failing tests
    * [Pull request #237](https://github.com/ignitionrobotics/ign-fuel-tools/pull/237)

1. Print error when unzipping fails to save a file
    * [Pull request #235](https://github.com/ignitionrobotics/ign-fuel-tools/pull/235)

1. More checks when manipulating file system in tests
    * [Pull request #227](https://github.com/ignitionrobotics/ign-fuel-tools/pull/227)

1. Move test cache to build folder
    * [Pull request #222](https://github.com/ignitionrobotics/ign-fuel-tools/pull/222)

1. Improve and fix ResultType tests
    * [Pull request #225](https://github.com/ignitionrobotics/ign-fuel-tools/pull/225)

1. `FuelClient.cc`: `include <deque>`
    * [Pull request #213](https://github.com/ignitionrobotics/ign-fuel-tools/pull/213)

1. APIs for retrieving models in parallel
    * [Pull request #199](https://github.com/ignitionrobotics/ign-fuel-tools/pull/199)

### Gazebo Fuel Tools 6.1.0 (2021-10-15)

1. Detect gz instead of using cmake module to check for gz-tools
    * [Pull request #191](https://github.com/gazebosim/gz-fuel-tools/pull/191)

1. Added fuel update command
    * [Pull request #185](https://github.com/gazebosim/gz-fuel-tools/pull/185)

1. Fixed windows download
    * [Pull request #178](https://github.com/gazebosim/gz-fuel-tools/pull/178)

1. Owner upload
    * [Pull request #179](https://github.com/gazebosim/gz-fuel-tools/pull/179)

1. Infrastructure
    * [Pull request #187](https://github.com/gazebosim/gz-fuel-tools/pull/187)
    * [Pull request #196](https://github.com/gazebosim/gz-fuel-tools/pull/196)

### Gazebo Fuel Tools 6.0.0 (2021-03-30)

1. Depend on gz-common4
    * [Pull request #163](https://github.com/gazebosim/gz-fuel-tools/pull/163)

1. Depend on gz-msgs7
    * [Pull request #158](https://github.com/gazebosim/gz-fuel-tools/pull/158)

1. Unset DELETE method on Windows
    * [Pull request #161](https://github.com/gazebosim/gz-fuel-tools/pull/161)

1. Edifice deprecations
    * [Pull request #159](https://github.com/gazebosim/gz-fuel-tools/pull/159)

1. Include windows instructions in examples README.md
    * [Pull request #176](https://github.com/gazebosim/gz-fuel-tools/pull/176)

## Gazebo Fuel Tools 5.x

### Gazebo Fuel Tools 5.2.0 (2021-12-20)

1. FuelClient.cc: include `<deque>`
    * [Pull request #213](https://github.com/gazebosim/gz-fuel-tools/pull/213)

1. APIs for retrieving models in parallel
    * [Pull request #199](https://github.com/gazebosim/gz-fuel-tools/pull/199)

1. Detect `gz` instead of using cmake module to check for gz-tools
    * [Pull request #191](https://github.com/gazebosim/gz-fuel-tools/pull/191)

1. Added `gz fuel update` command
    * [Pull request #185](https://github.com/gazebosim/gz-fuel-tools/pull/185)

1. Fixed Windows download and CI
    * [Pull request #178](https://github.com/gazebosim/gz-fuel-tools/pull/178)
    * [Pull request #164](https://github.com/gazebosim/gz-fuel-tools/pull/164)

1. Choose which owner to upload to
    * [Pull request #179](https://github.com/gazebosim/gz-fuel-tools/pull/179)

1. Included private-token to `config.yaml`
    * [Pull request #156](https://github.com/gazebosim/gz-fuel-tools/pull/156)

1. Add Windows Installation instructions
    * [Pull request #162](https://github.com/gazebosim/gz-fuel-tools/pull/162)

1. Use semantic version and prevent crash if version is missing
    * [Pull request #151](https://github.com/gazebosim/gz-fuel-tools/pull/151)

1. Document `GZ_FUEL_CACHE_PATH` on command line
    * [Pull request #149](https://github.com/gazebosim/gz-fuel-tools/pull/149)

1. Infrastructure
    * [Pull request #187](https://github.com/gazebosim/gz-fuel-tools/pull/187)
    * [Pull request #170](https://github.com/gazebosim/gz-fuel-tools/pull/170)
    * [Pull request #167](https://github.com/gazebosim/gz-fuel-tools/pull/167)
    * [Pull request #165](https://github.com/gazebosim/gz-fuel-tools/pull/165)

### Gazebo Fuel Tools 5.1.1 (2020-12-18)

1. Fix light map URI in materials
    * [Pull request 146](https://github.com/gazebosim/gz-fuel-tools/pull/146)

### Gazebo Fuel Tools 5.1.0 (2020-12-09)

1. Includes all changes up to version 4.3.0.

### Gazebo Fuel Tools 5.0.0 (2020-09-28)

1. Added support for annotation parsing during model creation.
    * [Pull request 95](https://github.com/gazebosim/gz-fuel-tools/pull/95)

1. Removed repeated installation instructions.
    * [Pull request 101](https://github.com/gazebosim/gz-fuel-tools/pull/101)

1. Update codeowners.
    * [Pull request 108](https://github.com/gazebosim/gz-fuel-tools/pull/108)

1. Hide Private headers and LocalCache.hh
    * [Pull request 109](https://github.com/gazebosim/gz-fuel-tools/pull/109)

1. Fixed test - Download world 2
    * [Pull request 110](https://github.com/gazebosim/gz-fuel-tools/pull/110)

## Gazebo Fuel Tools 4.x

### Gazebo Fuel Tools 4.8.3 (2023-03-29)

1. Support link referral download
    * [Pull request #333](https://github.com/gazebosim/gz-fuel-tools/pull/333)

### Gazebo Fuel Tools 4.9.1 (2024-01-05)

1. Create directories and more output on fail
    * [Pull request #392](https://github.com/gazebosim/gz-fuel-tools/pull/392)

1. Update GitHub action workflows
    * [Pull request #388](https://github.com/gazebosim/gz-fuel-tools/pull/388)

1. Zip: use non-deprecated methods
    * [Pull request #360](https://github.com/gazebosim/gz-fuel-tools/pull/360)

### Gazebo Fuel Tools 4.9.0 (2023-05-03)

1. Add bash completion
    * [Pull request #329](https://github.com/gazebosim/gz-fuel-tools/pull/329)

1. Reflect pagination of RESTful requests in iterator API
    * [Pull request #350](https://github.com/gazebosim/gz-fuel-tools/pull/350)

### Gazebo Fuel Tools 4.8.3 (2023-03-29)

1. Support link referral download
    * [Pull request #333](https://github.com/gazebosim/gz-fuel-tools/pull/333)

### Gazebo Fuel Tools 4.8.2 (2023-03-21)

1. Allow Curl redirect HTTP requests.
    * [Pull request #331](https://github.com/gazebosim/gz-fuel-tools/pull/331)

1. CI and license updates.
    * [Pull request #334](https://github.com/gazebosim/gz-fuel-tools/pull/334)
    * [Pull request #335](https://github.com/gazebosim/gz-fuel-tools/pull/335)

### Gazebo Fuel Tools 4.8.1 (2023-02-07)

1. Fix model downloads for ignitionrobotics.org URIs.
    * [Pull request #318](https://github.com/gazebosim/gz-fuel-tools/pull/318)

### Gazebo Fuel Tools 4.8.0 (2022-12-07)

1. Reduce asset download sizes.
    * [Pull request #316](https://github.com/gazebosim/gz-fuel-tools/pull/316)

1. Update tutorials to use Gazebo.
    * [Pull request #303](https://github.com/gazebosim/gz-fuel-tools/pull/303)

1. Change ignitionrobotics.org to gazebosim.org.
    * [Pull request #304](https://github.com/gazebosim/gz-fuel-tools/pull/304)

### Gazebo Fuel Tools 4.7.0 (2022-11-17)

1. ign -> gz Migrate Ignition Headers : gz-fuel-tools.
    * [Pull request #285](https://github.com/gazebosim/gz-fuel-tools/pull/285)

### Gazebo Fuel Tools 4.6.0 (2022-08-15)

. Ignition to Gazebo server rename
    * [Pull request #263](https://github.com/gazebosim/gz-fuel-tools/pull/263)

1. Remove redundant namespace references
    * [Pull request #284](https://github.com/gazebosim/gz-fuel-tools/pull/284)

1. Change `IGN_DESIGNATION` to `GZ_DESIGNATION`
    * [Pull request #278](https://github.com/gazebosim/gz-fuel-tools/pull/278)

1. Ignition -> Gazebo
    * [Pull request #276](https://github.com/gazebosim/gz-fuel-tools/pull/276)

1. Remove unused files (Bitbucket-pipelines, docs)
    * [Pull request #273](https://github.com/gazebosim/gz-fuel-tools/pull/273)

1. Disable `staging-fuel.gazebosim.org` test
    * [Pull request #257](https://github.com/gazebosim/gz-fuel-tools/pull/257)

### Gazebo Fuel Tools 4.5.0 (2022-06-01)

1. APIs for retrieving models in parallel
    * [Pull request #199](https://github.com/gazebosim/gz-fuel-tools/pull/199)

1. Add missing header guard for Interface.hh
    * [Pull request #249](https://github.com/gazebosim/gz-fuel-tools/pull/249)

1. Print error when unzipping fails to save a file
    * [Pull request #235](https://github.com/gazebosim/gz-fuel-tools/pull/235)

1. More checks when manipulating file system in tests
    * [Pull request #227](https://github.com/gazebosim/gz-fuel-tools/pull/227)

1. Move test cache to build folder
    * [Pull request #222](https://github.com/gazebosim/gz-fuel-tools/pull/222)

1. Improve and fix `ResultType` tests
    * [Pull request #225](https://github.com/gazebosim/gz-fuel-tools/pull/225)

1. FuelClient.cc: include <deque>
    * [Pull request #213](https://github.com/gazebosim/gz-fuel-tools/pull/213)

### Gazebo Fuel Tools 4.4.0 (2021-06-17)

1. Remove `tools/code_check` and update codecov
    * [Pull request #187](https://github.com/gazebosim/gz-fuel-tools/pull/187)

1. Fixed windows download
    * [Pull request #178](https://github.com/gazebosim/gz-fuel-tools/pull/178)

1. Master branch updates
    * [Pull request #170](https://github.com/gazebosim/gz-fuel-tools/pull/170)

1. Support private-token in `config.yaml`
    * [Pull request #156](https://github.com/gazebosim/gz-fuel-tools/pull/156)

1. Add Windows Installation
    * [Pull request #162](https://github.com/gazebosim/gz-fuel-tools/pull/162)

1. Use semantic version and prevent crash if version is missing
    * [Pull request #151](https://github.com/gazebosim/gz-fuel-tools/pull/151)

1. Document `GZ_FUEL_CACHE_PATH` on command line
    * [Pull request #149](https://github.com/gazebosim/gz-fuel-tools/pull/149)

1. Support editing/patching model files
    * [Pull request #140](https://github.com/gazebosim/gz-fuel-tools/pull/140)

1. Set keep alive on
    * [Pull request #141](https://github.com/gazebosim/gz-fuel-tools/pull/141)

### Gazebo Fuel Tools 4.3.0 (2020-12-01)

1. Improve fork experience.
    * [Pull request 126](https://github.com/gazebosim/gz-fuel-tools/pull/126)

1. Download dependencies.
    * [Pull request 123](https://github.com/gazebosim/gz-fuel-tools/pull/123)

1. Includes all changes up to version 3.5.0.

### Gazebo Fuel Tools 4.2.1 (2020-08-26)

1. Fix `gz fuel download`, which was missing the `-j` option.
    * [Pull request 116](https://github.com/gazebosim/gz-fuel-tools/pull/116)

### Gazebo Fuel Tools 4.2.0 (2020-08-26)

1. Set license information based on licenses available from a Fuel server
   and `legal` information in a `metadata.pbtxt` file.
    * [Pull request 69](https://github.com/gazebosim/gz-fuel-tools/pull/69)

1. Added `edit` subcommand to the `gz fuel`. The edit command currently
   supports editing a model's privacy.
    * [Pull request 67](https://github.com/gazebosim/gz-fuel-tools/pull/67)

1. Alphabetical listing of subcommands.
    * [Pull request 65](https://github.com/gazebosim/gz-fuel-tools/pull/65)

1. All changes up to and including Gazebo Fuel Tools 3.3.0 are included.

### Gazebo Fuel Tools 4.1.0 (2020-02-27)

1. Resource deletion CLI.
    * [BitBucket pull request 119](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/119)

1. Fetch files
    * [BitBucket pull request 123](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/123)

1. README and tutorial updates:
    * [BitBucket pull request 113](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/113)
    * [BitBucket pull request 114](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/114)
    * [BitBucket pull request 115](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/115)

### Gazebo Fuel Tools 4.0.0 (2019-12-10)

1. Model upload CLI, resource metadata CLI, depend on gz-msgs5.
    * [BitBucket pull request 108](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/108)

## Gazebo Fuel Tools 3.x

### Gazebo Fuel Tools 3.x.x (20xx-xx-xx)

### Gazebo Fuel Tools 3.5.0 (2020-11-30)

1. Fix windows build.
    * [Pull request 107](https://github.com/gazebosim/gz-fuel-tools/pull/107)

1. Resolve updated codecheck issues.
    * [Pull request 129](https://github.com/gazebosim/gz-fuel-tools/pull/129)

1. Update Backpack model to fix tests.
    * [Pull request 132](https://github.com/gazebosim/gz-fuel-tools/pull/132)

1. Use lowercase resource and owner names when storing assets on disk.
    * [Pull request 130](https://github.com/gazebosim/gz-fuel-tools/pull/130)

### Gazebo Fuel Tools 3.4.0 (2020-08-19)

1. Modernize GitHub actions by updating to the new style.
    * [Pull request 94](https://github.com/gazebosim/gz-fuel-tools/pull/94)

1. Fix test worlds.
    * [Pull request 100](https://github.com/gazebosim/gz-fuel-tools/pull/100)

1. Add capability for downloading collections.
    * [Pull request 98](https://github.com/gazebosim/gz-fuel-tools/pull/98)

1. Fix windows build.t
    * [Pull request 103](https://github.com/gazebosim/gz-fuel-tools/pull/013)

### Gazebo Fuel Tools 3.3.0 (2020-07-29)

1. openrobotics to OpenRobotics
    * [Pull request 75](https://github.com/gazebosim/gz-fuel-tools/pull/75)

1. Fix world tests
    * [Pull request 76](https://github.com/gazebosim/gz-fuel-tools/pull/76)

1. Add missing dependency in Actions CI
    * [Pull request 86](https://github.com/gazebosim/gz-fuel-tools/pull/86)

1. Backport file fetching
    * [Pull request 84](https://github.com/gazebosim/gz-fuel-tools/pull/84)

1. Convert model:// to Fuel URLs instead of absolute paths
    * [Pull request 85](https://github.com/gazebosim/gz-fuel-tools/pull/85)

### Gazebo Fuel Tools 3.2.2 (2020-05-18)

1. Fix URL encodings in RestClient.
    * [Pull request 70](https://github.com/gazebosim/gz-fuel-tools/pull/70)

1. Print message when downloading a resource.
    * [BitBucket pull request 102](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/102)

1. Use `${YAML_TARGET}` instead of `YAML::YAML` imported target.
    * [BitBucket pull request 103](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/103)

1. Convert emissive map file path.
    * [BitBucket pull request 105](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/105)

1. Fix windows build with `popen` and `pclose` macros.
    * [BitBucket pull request 109](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/109)

1. Remove std::experimental for filesystem (support for VS2019).
    * [BitBucket pull request 120](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/120)

### Gazebo Fuel Tools 3.2.1 (2019-08-12)

1. Support actors
    * [BitBucket pull request 101](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/101)

### Gazebo Fuel Tools 3.2.0 (2019-06-14)

1. FuelClient::DownloadModel accepts HTTP headers, and the `gz fuel
   download` command can accept a single HTTP header.
    * [BitBucket pull request 100](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/100)

### Gazebo Fuel Tools 3.1.0 (2019-05-xx)

1. Fix PBR material URI
    * [BitBucket pull request 95](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/95)
    * [BitBucket pull request 96](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/96)

### Gazebo Fuel Tools 3.0.0 (2018-02-28)

1. Integration with gz-sim. The following set of pull requests update
   fuel-tools to use gz-cmake2, sets api.ignitionfuel.org as the default
   server, fixes codecheck errors, removes old deprecations, and parses
   model.config files using tinyxml2.
    * [BitBucket pull request 82](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/82)
    * [BitBucket pull request 83](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/83)
    * [BitBucket pull request 84](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/84)
    * [BitBucket pull request 85](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/85)
    * [BitBucket pull request 87](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/87)
    * [BitBucket pull request 88](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/88)
    * [BitBucket pull request 89](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/89)

## Gazebo Fuel Tools 1.x

### Gazebo Fuel Tools 1.x.x (20xx-xx-xx)

1. Use `${YAML_TARGET}` instead of `YAML::YAML` imported target.
    * [BitBucket pull request 103](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/103)

### Gazebo Fuel Tools 1.2.0 (2018-05-30)

1. Get cached model resource file
    * [BitBucket pull request 63](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/63)

1. Add some AsString functions
    * [BitBucket pull request 54](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/54)

1. Change cache directory structure and use it
    * [BitBucket pull request 57](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/57)
    * [BitBucket pull request 71](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/71)
    * [BitBucket pull request 73](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/73)

1. Client return only relevant cached models
    * [BitBucket pull request 62](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/62)

1. Model version support
    * [BitBucket pull request 65](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/65)

1. Remove server API version from model unique name
    * [BitBucket pull request 47](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/47)

1. Client handle unique names and get server info from config
    * [BitBucket pull request 55](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/55)

1. Changed model list to use headers for paging instead of iterating until
   a 404 is hit. Also added a mechanism to set the user agent
    * [BitBucket pull request 46](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/46)
    * [BitBucket pull request 61](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/61)

1. FuelClient: Don't use ServerConfig if there's ModelId
    * [BitBucket pull request 56](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/56)

1. List models from command line
    * [BitBucket pull request 43](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/43)
    * [BitBucket pull request 48](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/48)
    * [BitBucket pull request 44](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/44)
    * [BitBucket pull request 45](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/45)

1. Get cached model resource file
    * [BitBucket pull request 63](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/63)

1. Added const constructors for ModelIter and Model
    * [BitBucket pull request 42](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/42)

1. Use common::URI for server URL
    * [BitBucket pull request 59](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/59)

1. Remove server local name
    * [BitBucket pull request 58](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/58)

1. Encode url path before downloading models
    * [BitBucket pull request 41](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/41)

1. Download model from command line
    * [BitBucket pull request 68](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/68)

1. Backport code style changes
    * [BitBucket pull request 69](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/69)
    * [BitBucket pull request 67](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-fuel-tools/pull-requests/67)

### Gazebo Fuel Tools 1.0.0 (2018-01-25)
