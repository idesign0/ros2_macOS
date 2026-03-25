^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package ament_black
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.2.6 (2024-09-05)
------------------
* Add packaging as a required dependency (`#14 <https://github.com/botsandus/ament_black/issues/14>`_)
  Follow up to:
  - https://github.com/botsandus/ament_black/pull/13
* Contributors: Ruffin

0.2.5 (2024-07-29)
------------------
* Fix ament_black for new get_sources API (`#13 <https://github.com/botsandus/ament_black/issues/13>`_)
  * Fix ament_black for new get_sources API
  * Fix import order
  * Add backward compatibility with older black versions
  * style
  * Move global variables inside method to fix flake8
  * Import only for newer versions
  * Last tweak
  ---------
  Co-authored-by: Ignacio Vizzo <ignacio@dexory.com>
* Contributors: Błażej Sowa

0.2.4 (2024-01-19)
------------------
* Nacho/fix rolling build (`#9 <https://github.com/botsandus/ament_black/issues/9>`_)
* Nacho/fix xunit report (`#8 <https://github.com/botsandus/ament_black/issues/8>`_)
* Contributors: Ignacio Vizzo

0.2.3 (2023-10-13)
------------------
* Add python3-uvloop as dependency https://github.com/ros/rosdistro/pull/38754
* Contributors: Ignacio Vizzo

0.2.2 (2023-10-11)
------------------
* Nacho/fix ci tests (`#5 <https://github.com/botsandus/ament_black/issues/5>`_)
  * Add missing test dependency
  * pre-commit
  * Add pre-commit workflow
  * Add CLI test to make sure the package has been properly installed
  * Remove ament hooks
  * Add missing apt-python dependency
  Not sure if it's necessary ...
  * Fix CI pre-commit
  The CI error was fixed in: https://github.com/psf/black/issues/2964
  Thus, we need black >22.3.0
* Contributors: Ignacio Vizzo

0.2.1 (2023-10-11)
------------------
* Nacho/fix black (`#4 <https://github.com/botsandus/ament_black/issues/4>`_)
  * Don't guess filenames. Black knows better
  * Pass only the files to be formatted
  * Fix ament-black multithread execution
  * Add uvloop as dependency to allow concurrency
* Contributors: Ignacio Vizzo

0.2.0 (2023-10-09)
------------------------
* Create a new release for a renewed and mantained package
* Contributors: Ignacio Vizzo [Dexory]

0.1.0 (2022-05-31)
------------------
* Initial release.
