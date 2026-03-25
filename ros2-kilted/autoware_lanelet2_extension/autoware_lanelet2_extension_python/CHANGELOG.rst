^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_lanelet2_extension_python
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.12.0 (2026-02-17)
-------------------

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
* feat(lanelet2_extension): deprecate fromBinMsg and toBinMsg (`#89 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/89>`_)
* feat: add tmp to python API (`#90 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/90>`_)
  * feat: add TMProjector in python API
  Also add code docs for easier usage in IDE.
  * Rename import for boost python projection module
* feat(lanelet2_extension): deprecate getLaneletLength2d and getLaneletLength3d (`#88 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/88>`_)
  Co-authored-by: Mamoru Sobue <hilo.soblin@gmail.com>
* feat(lanelet2_extension): deprecate ported functions in lanelet2_extension (`#81 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/81>`_)
* feat(lanelet2_extension_python): export `MGRSProjector::getProjectedMGRSGrid` for python (`#87 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/87>`_)
  feat(lanelet2_extension_python): export more MGRS functions for python
* feat(lanelet2_extension_python): add python binding for MGRSProjector (`#84 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/84>`_)
* chore(lanelet2_extension_python): add maintainer (`#85 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/85>`_)
* test(lanelet2_extension_python): add test & test for import (`#83 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/83>`_)
* Contributors: Mamoru Sobue, Sarun MUKDAPITAK, Yuxuan Liu

0.10.0 (2025-11-05)
-------------------
* feat(lanelet2_extension_python): add bindings for fromBinMsg (`#80 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/80>`_)
* Contributors: Maxime CLEMENT

0.9.0 (2025-08-04)
------------------

0.8.0 (2025-07-22)
------------------

0.7.2 (2025-05-14)
------------------

0.7.1 (2025-04-25)
------------------

0.7.0 (2025-04-07)
------------------

0.6.4 (2025-04-04)
------------------

0.6.3 (2025-03-17)
------------------
* chore: sync files (`#42 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/42>`_)
  Co-authored-by: M. Fatih Cırıt <mfc@autoware.org>
* fix(autoware_lanelet2_extension): fix links to issues in CHANGELOG.rst files (`#40 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/40>`_)
* Contributors: Esteve Fernandez, awf-autoware-bot[bot]

0.6.2 (2024-11-21)
------------------
* fix(Python bindings): add missing dependency to lanelet2 (`#32 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/32>`_)
* Contributors: Kenji Miyake

0.6.1 (2024-09-06)
------------------

0.6.0 (2024-08-28)
------------------

0.5.0 (2024-07-10)
------------------
* chore(ci): fix cpplint errors from pre-commit ci (`#15 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/15>`_)
* chore: apply pre-commit (`#14 <https://github.com/autowarefoundation/autoware_lanelet2_extension/issues/14>`_)
  apply pre-commit
* Contributors: Daisuke Nishimatsu, Ryohsuke Mitsudome

0.4.0 (2024-06-24)
------------------

0.3.0 (2024-05-31)
------------------

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
