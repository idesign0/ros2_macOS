^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package google_benchmark_vendor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.1.2 (2023-07-18)
------------------
* Resolve Clang compilation issue with benchmark 1.5.3 (`#28 <https://github.com/ament/google_benchmark_vendor/issues/28>`_)
* Contributors: Adam Seiwert

0.1.1 (2022-03-29)
------------------
* Add git buildtool dependency.
* Contributors: Steven! Ragnarök

0.1.0 (2022-03-28)
------------------
* Use git hash for google_benchmark_vendor (`#20 <https://github.com/ament/google_benchmark_vendor/issues/20>`_)
* Update to google benchmark version 1.6.1 (`#19 <https://github.com/ament/google_benchmark_vendor/issues/19>`_)
* Contributors: Chris Lalancette, Shane Loretz

0.0.7 (2022-01-14)
------------------
* Update maintainers to Audrow Nash (`#18 <https://github.com/ament/google_benchmark_vendor/issues/18>`_)
* Update google_benchmark to v1.5.3 (`#16 <https://github.com/ament/google_benchmark_vendor/issues/16>`_)
  1. Change google_benchmark version from v1.5.2 to v1.5.3. Because v1.5.2
  can not build with GCC 11
  2. Removed shrink-tz-offset-size.patch because of this patch was merged in
  google-benchmark repo.
* Add changelog (`#15 <https://github.com/ament/google_benchmark_vendor/issues/15>`_)
* Contributors: Audrow Nash, Homalozoa X, Ivan Santiago Paunovic

0.0.6 (2021-04-12)
------------------
* Shrink the size of the tz_offset variable. (`#13 <https://github.com/ament/google_benchmark_vendor/issues/13>`_)
* Contributors: Chris Lalancette

0.0.5 (2021-03-18)
------------------
* Update the patching to work on Windows without admin. (`#11 <https://github.com/ament/google_benchmark_vendor/issues/11>`_)
* Always preserve source permissions in vendor packages. (`#12 <https://github.com/ament/google_benchmark_vendor/issues/12>`_)
* Contributors: Chris Lalancette, Scott K Logan

0.0.4 (2020-12-07)
------------------
* Update package maintainers. (`#10 <https://github.com/ament/google_benchmark_vendor/issues/10>`_)
* Upgrade google benchmark from v1.5.1 to v1.5.2 to include QNX patch. (`#9 <https://github.com/ament/google_benchmark_vendor/issues/9>`_)
* Contributors: Ahmed Sobhy, Michel Hidalgo

0.0.3 (2020-09-15)
------------------
* Set the SOVERSION on benchmark libraries. (`#8 <https://github.com/ament/google_benchmark_vendor/issues/8>`_)
* Contributors: Scott K Logan

0.0.2 (2020-07-20)
------------------
* Set minimum criteria for system package. (`#3 <https://github.com/ament/google_benchmark_vendor/issues/3>`_)
* Work around warnings building Google Benchmark w/Clang. (`#2 <https://github.com/ament/google_benchmark_vendor/issues/2>`_)
* Contributors: Scott K Logan

0.0.1 (2020-07-14)
------------------
* Initial google_benchmark_vendor package. (`#1 <https://github.com/ament/google_benchmark_vendor/issues/1>`_)
* Initial commit.
* Contributors: Scott K Logan
