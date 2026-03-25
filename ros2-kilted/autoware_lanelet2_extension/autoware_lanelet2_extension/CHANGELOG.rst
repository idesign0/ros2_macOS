^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_lanelet2_extension
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.12.0 (2026-02-17)
-------------------
* feat(visualizer): change a tag name; map_area_filter to obstacle_removal_area (`#100 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/100>`_)
* feat(visualzer): add new maker type (`#96 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/96>`_)
  * add new maker
  * add docs
  * fix spell
  ---------
* Contributors: Yuki TAKAGI

0.11.0 (2026-02-14)
-------------------
* feat(lanelet2_extension): deprecate some functions in query (2) (`#94 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/94>`_)
  deprecate ported functions
  - getLaneChangeableNeighbors
  - getSuceedingLaneletSequences
  - getPrecedingLaneletSequences
* feat(lanelet2_extension): deprecate some functions in query (`#91 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/91>`_)
  * deprecate following functions
  - getAllNeighbors
  - getAllNeighborsLeft
  - getAllNeighborsRight
  - getLaneletsWithinRange
  * deprecate overload of getAllNeighbors
  ---------
* fix(autoware_lanelet2_extension): remove unused variable (`#93 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/93>`_)
  Remove unused  variable that was declared but never used.
  The lanelet::Origin is created directly in the UtmProjector constructor.
  Detected by Facebook Infer static analyzer (DEAD_STORE).
* fix(autoware_lanelet2_extension): remove unused variables (`#92 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/92>`_)
  fix(autoware_lanelet2_extension): remove unused variables in visualization.cpp
  Remove unused 'point' and 'color' variables that were declared but never used.
  The code directly assigns to marker->pose.position and marker->color
  without using these local variables.
  Detected by Facebook Infer static analyzer (DEAD_STORE).
* feat(lanelet2_extension): deprecate fromBinMsg and toBinMsg (`#89 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/89>`_)
* feat(lanelet2_extension): deprecate getLaneletLength2d and getLaneletLength3d (`#88 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/88>`_)
  Co-authored-by: Mamoru Sobue <hilo.soblin@gmail.com>
* feat(lanelet2_extension): deprecate ported functions in lanelet2_extension (`#81 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/81>`_)
* Contributors: Ryuta Kambe, Sarun MUKDAPITAK

0.10.0 (2025-11-05)
-------------------
* chore: add build option -Wno-error=maybe-uninitialized (`#79 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/79>`_)
  build::jazzy-porting::add build option -Wno-error=maybe-uninitialized to surpress compile error caused by boost-geometry library, v0.0
* fix: combineLaneletsShape duplicated point (`#77 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/77>`_)
  * fix: combineLaneletsShape duplicated point
  * check distance
  style(pre-commit): autofix
  style(pre-commit): autofix
  ---------
* Contributors: Kosuke Takeuchi, 心刚

0.9.0 (2025-08-04)
------------------
* feat: add roundabout regulatory element (`#75 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/75>`_)
  * feat: add roundabout regulatory element implementation
  * feat: add entry and exit lanelet checks for roundabouts
  * feat: enhance roundabout functionality with lanelet ID caching and checks
  * feat: update roundabout lanelet checks to use lanelet IDs
  * style(pre-commit): autofix
  * fix: include unordered_set for improved functionality
  * style(pre-commit): autofix
  * fix: remove unnecessary include
  Co-authored-by: Yukinari Hisaki <42021302+yhisaki@users.noreply.github.com>
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
  Co-authored-by: Yukinari Hisaki <42021302+yhisaki@users.noreply.github.com>
* Contributors: Sho Iwasawa

0.8.0 (2025-07-22)
------------------
* feat: add linestring with type getter (`#73 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/73>`_)
* fix: skip duplicate marker (`#69 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/69>`_)
  * fix: skip duplicate marker
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: Ryohsuke Mitsudome, Zulfaqar Azmi

0.7.2 (2025-05-14)
------------------
* fix(autoware_lanelet2_extension): point on the edge of a triangle was  not treated as inside the triangle (`#66 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/66>`_)
  fix(autoware_lanelet2_extension): point on the edge of a triangle was not treated as inside the triangle
  Co-authored-by: Marek Piechula <mpiechula@autonomous-systems.pl>
* Contributors: Marek Piechula

0.7.1 (2025-04-25)
------------------
* ci(pre-commit): autoupdate (`#55 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/55>`_)
  * ci(pre-commit): autoupdate
  * style(pre-commit): autofix
  ---------
  Co-authored-by: github-actions <github-actions@github.com>
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* fix(autoware_lanelet2_extension): not use obsolete header (`#63 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/63>`_)
  * Update query.cpp
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: Yutaka Kondo, awf-autoware-bot[bot]

0.7.0 (2025-04-07)
------------------
* feat(lanelet2_extension)!: remove dependency on autoware_utils (`#47 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/47>`_)
  * chore(lanelet2_extension)!: remove dependency on autoware_utils
  * add test
  * WIP
  * WIP2
  * remove tinyxml2
  * remove from build_depends
  ---------
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* feat: add query for all waypoints (`#56 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/56>`_)
* Contributors: Mamoru Sobue, Mehmet Dogru

0.6.4 (2025-04-04)
------------------
* feat(projection): add_scale_factor (`#54 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/54>`_)
  * add_scale_factor
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* feat: add getArcCoordinatesConsideringWaypoints to enable custom waypoint usage (`#49 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/49>`_)
  * feat: add getArcCoordinatesUsingWaypoints to enable custom waypoint usage
  * update function name and explanation
  ---------
* fix: update link to autoware_map_loader (`#53 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/53>`_)
* Contributors: Mehmet Dogru, Ryohsuke Mitsudome, Yamato Ando

0.6.3 (2025-03-17)
------------------
* docs(localization_landmarks): add localization reflector subtype (`#44 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/44>`_)
  doc: add localization reflector subtype
* feat: add the lanelet length test (`#39 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/39>`_)
  * lanelet length test added.
  * style(pre-commit): autofix
  * compare the precise values
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* docs: update projection type from "local" to "Local" (`#34 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/34>`_)
* chore: sync files (`#42 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/42>`_)
  Co-authored-by: M. Fatih Cırıt <mfc@autoware.org>
* fix(autoware_lanelet2_extension): fix links to issues in CHANGELOG.rst files (`#40 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/40>`_)
* Contributors: Esteve Fernandez, Hans Oersted, Motz, Ryohsuke Mitsudome, awf-autoware-bot[bot]

0.6.2 (2024-11-21)
------------------
* fix: update the github link of map_loader (`#31 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/31>`_)
  add autoware prefix to map_loader
* Contributors: Masaki Baba

0.6.1 (2024-09-06)
------------------
* feat(lanelet2_extension): add util/visualization of BusStopArea (`#28 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/28>`_)
* feat(lanelet2_extension): add query for bicycle_lane (`#29 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/29>`_)
  add bicycle_lane query
* Contributors: Mamoru Sobue

0.6.0 (2024-08-28)
------------------
* feat(lanelet2_extension)!: release format_v2 (`#26 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/26>`_)
* docs(autoware_lanelet2_extension): refactor document (`#25 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/25>`_)
  * refactor doc
  * Update autoware_lanelet2_extension/docs/lanelet2_format_extension.md
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
  ---------
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
* feat(lanelet2_extension): format v2 bicycle lane doc (`#24 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/24>`_)
* feat(lanelet2_extension): update documentation for bus_stop_area to be introduced in format_version2 (`#21 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/21>`_)
* feat(lanelet2_extension): add bus_stop_area implementation (`#22 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/22>`_)
* Contributors: Mamoru Sobue

0.5.0 (2024-07-10)
------------------
* feat(lanelet2_extension)!: introduce API versioning along with format_version (`#18 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/18>`_)
* build: remove redundant move for build on noble (`#12 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/12>`_)
  remove redundant move
* refactor: remove redundant cmake definition (`#13 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/13>`_)
  * remove redundant cmake definition
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* chore: apply pre-commit (`#14 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/14>`_)
  apply pre-commit
* Contributors: Daisuke Nishimatsu, Mamoru Sobue

0.4.0 (2024-06-24)
------------------
* Merge pull request `#11 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/11>`_ from youtalk/import-update
  feat: import updates from `autoware_common`
* fix link
* feat(lanelet2_extension): overwriteLaneletsCenterline supports "waypoints" (`#252 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/252>`_)
  * feat(lanelet2_extension): centerline is converted to waypoints
  * fix lanelet2_extension_python
  * update README
  * fix
  * fix
  * early return
  * fix clang-tidy
  * Update tmp/lanelet2_extension/lib/utilities.cpp
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
  * style(pre-commit): autofix
  * fix
  ---------
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* fix: boost optional build error on rolling environment (`#241 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/241>`_)
* perf(lanelet2_extension): use std::unordered_set<>::find instead of std::find (`#244 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/244>`_)
  perf(exists): use std::unordered_set<>::find instead of std::find
* Contributors: Maxime CLEMENT, Takayuki Murooka, Yutaka Kondo, ぐるぐる

0.3.0 (2024-05-31)
------------------
* Merge remote-tracking branch 'upstream/main' into rolling
* Merge pull request `#6 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/6>`_ from youtalk/autoware-msgs-migration
  feat: `autoware_msgs` migration
* feat(autoware_common): update and replace autoware_auto_msg
* feat(lanelet2_extension): replace autoware_auto_mapping_msg with autoware_map_msg (`#216 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/216>`_)
  * feat(lanelet2_extension): replace autoware-auto-mapping-msg to autoware-map-msg
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: Yutaka Kondo, cyn-liu, liu cui

0.2.0 (2024-05-07)
------------------
* refactor: add `autoware\_` prefix to package names and namespaces (`#3 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/3>`_)
  * fix readme
  * fix link
  * fix link
  * revert readme
  * wip
  * update package names
  * update package names
  * fix include
  * rename module
  * rename to autoware\_
  * fix depend
  * rename to autoware\_
  * Revert "wip"
  This reverts commit 8079660c318feace9d841aacd39a8945aa144cf7.
  ---------
* Contributors: Yutaka Kondo
