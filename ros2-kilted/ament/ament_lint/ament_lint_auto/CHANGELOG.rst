^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package ament_lint_auto
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.19.2 (2025-04-17)
-------------------
* Add docu for AMENT_LINT_AUTO_EXCLUDE (`#524 <https://github.com/ament/ament_lint/issues/524>`_)
* Contributors: Alexander Reimann

0.19.1 (2025-02-02)
-------------------

0.19.0 (2024-11-20)
-------------------

0.18.1 (2024-06-17)
-------------------

0.18.0 (2024-04-26)
-------------------

0.17.0 (2024-04-16)
-------------------

0.16.4 (2024-03-28)
-------------------
* Update maintainer list in package.xml files (`#474 <https://github.com/ament/ament_lint/issues/474>`_)
* Contributors: Michael Jeronimo

0.16.3 (2024-02-07)
-------------------

0.16.2 (2023-12-26)
-------------------

0.16.1 (2023-11-06)
-------------------

0.16.0 (2023-10-04)
-------------------

0.15.2 (2023-07-11)
-------------------

0.15.1 (2023-06-07)
-------------------

0.15.0 (2023-04-27)
-------------------

0.14.1 (2023-04-11)
-------------------

0.14.0 (2023-02-14)
-------------------
* Add flake8 linter ignore support (`#424 <https://github.com/ament/ament_lint/issues/424>`_)
* [rolling] Update maintainers - 2022-11-07 (`#421 <https://github.com/ament/ament_lint/issues/421>`_)
* Contributors: Audrow Nash, RFRIEDM-Trimble, Abrar Rahman Protyasha

0.13.2 (2022-09-13)
-------------------
* [ament_lint_auto] General file exclusion with AMENT_LINT_AUTO_FILE_EXCLUDE (`#386 <https://github.com/ament/ament_lint/issues/386>`_)
* Update maintainers (`#379 <https://github.com/ament/ament_lint/issues/379>`_)
* Contributors: Abrar Rahman Protyasha, methylDragon

0.13.1 (2022-05-09)
-------------------

0.13.0 (2022-04-29)
-------------------

0.12.3 (2022-04-08)
-------------------

0.12.2 (2022-03-28)
-------------------

0.12.1 (2022-03-01)
-------------------

0.12.0 (2022-02-18)
-------------------

0.11.4 (2022-01-14)
-------------------
* Update forthcoming version in changelogs
* Contributors: Audrow Nash

0.11.3 (2022-01-14)
-------------------
* Update maintainers to Michael Jeronimo and Michel Hidalgo (`#340 <https://github.com/ament/ament_lint/issues/340>`_)
* Contributors: Audrow Nash

0.11.2 (2021-06-18)
-------------------

0.11.1 (2021-06-18)
-------------------

0.11.0 (2021-06-18)
-------------------

0.10.6 (2021-05-06)
-------------------

0.10.5 (2021-04-14)
-------------------
* Remove Claire as a maintainer. (`#312 <https://github.com/ament/ament_lint/issues/312>`_)
  * Remove Claire as a maintainer.
  * Remove dead email addresses.
  * Remove more dead email addresses.
  * Switch setup.py maintainer to Audrow.
* Contributors: Chris Lalancette

0.10.4 (2021-03-18)
-------------------

0.10.3 (2021-03-03)
-------------------
* 0.10.3
* Contributors: Audrow Nash

0.10.2 (2021-02-12)
-------------------
* Add Audrow as a maintainer (`#294 <https://github.com/ament/ament_lint/issues/294>`_)
* Drop trailing tab from package manifests (`#291 <https://github.com/ament/ament_lint/issues/291>`_)
  Follow-up to 8bf194aa1ac282db5483dd0d3fefff8f325b0db8
* Contributors: Audrow Nash, Scott K Logan

0.10.1 (2021-01-25)
-------------------
* Use correct lint package dependencies (`#278 <https://github.com/ament/ament_lint/issues/278>`_)
* Update maintainer (`#274 <https://github.com/ament/ament_lint/issues/274>`_)
  * update maintainer
  * add authors
* Contributors: Claire Wang, Esteve Fernandez

0.10.0 (2020-09-18)
-------------------

0.9.4 (2020-05-26)
------------------

0.9.3 (2020-05-18)
------------------

0.9.2 (2020-05-08)
------------------

0.9.1 (2020-04-10)
------------------
* 0.9.0
* Rename pep8 packages to pycodestyle (part 2 of 2)
  The python package 'pep8' has been renamed to 'pycodestyle'. This change
  follows suit by renaming ament_pep8 and ament_cmake_pep8 to
  ament_pycodestyle and ament_cmake_pycodestyle respectively.
* Contributors: Chris Lalancette, Scott K Logan

0.8.1 (2019-10-23)
------------------

0.8.0 (2019-09-17)
------------------

0.7.4 (2019-07-31)
------------------

0.7.3 (2019-05-09 14:08)
------------------------

0.7.2 (2019-05-09 09:30)
------------------------

0.7.1 (2019-05-07)
------------------
* Fix unused-arg check in auto find test deps: (`#136 <https://github.com/ament/ament_lint/issues/136>`_)
  - Arguments to a macro are not variables, so it's not
  possible to do 'if(ARGN)' to check for arguments;
  however, copying ARGN to a variable works
* add version dependency for new feature (`#133 <https://github.com/ament/ament_lint/issues/133>`_)
* Use exclude option of ament_execute_extensions in ament_lint_auto (`#133 <https://github.com/ament/ament_lint/issues/133>`_)
* Contributors: Dirk Thomas, jpsamper2009

0.7.0 (2019-04-11)
------------------

0.6.3 (2019-01-14)
------------------

0.6.2 (2018-12-06)
------------------

0.6.1 (2018-11-15)
------------------

0.6.0 (2018-11-14)
------------------

0.5.2 (2018-06-27)
------------------

0.5.1 (2018-06-18 13:47)
------------------------

0.5.0 (2018-06-18 10:09)
------------------------

0.4.0 (2017-12-08)
------------------
* 0.0.3
* 0.0.2
* update schema url
* add schema to manifest files
* Merge pull request `#56 <https://github.com/ament/ament_lint/issues/56>`_ from ament/cmake35
  require CMake 3.5
* require CMake 3.5
* Merge pull request `#50 <https://github.com/ament/ament_lint/issues/50>`_ from ament/ctest_build_testing
  use CTest BUILD_TESTING
* use CTest BUILD_TESTING
* Merge pull request `#26 <https://github.com/ament/ament_lint/issues/26>`_ from ament/error_when_used_after_ament_package
  ensure that ament_package() was called before finding ament_lint_auto
* ensure that ament_package() was called before finding ament_lint_auto
* add explicit build type
* disable debug output
* add missing copyright / license information
* Merge pull request `#9 <https://github.com/ament/ament_lint/issues/9>`_ from ament/docs
  add docs for linters
* add docs for linters
* Merge pull request `#2 <https://github.com/ament/ament_lint/issues/2>`_ from ament/ament_lint_auto
  allow linting based on test dependencies only
* add ament_lint_auto and ament_lint_common, update all linter packages to implement extension point of ament_lint_auto
* Contributors: Dirk Thomas
