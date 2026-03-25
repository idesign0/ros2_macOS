^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package angles
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.16.1 (2025-09-23)
-------------------
* Ensure that M_PI is defined in any downstream compilation unit in Windows (`#44 <https://github.com/ros/angles/issues/44>`_)
* Enable Testing (`#39 <https://github.com/ros/angles/issues/39>`_)
* New Maintainer (`#40 <https://github.com/ros/angles/issues/40>`_)
* Contributors: David V. Lu!!, Silvio Traversaro

1.16.0 (2022-07-12)
-------------------
* Correct versions for version bump
* ROS 2 Python Port (`#37 <https://github.com/ros/angles/issues/37>`_)
* Fix M_PI on Windows (`#34 <https://github.com/ros/angles/issues/34>`_)
* Contributors: Akash, David V. Lu!!, Geoffrey Biggs

1.13.0 (2022-01-18)
-------------------
* Export ament_cmake buildtool dependency (`#32 <https://github.com/ros/angles/issues/32>`_)
* Install includes to include/angles and add modern CMake target (`#28 <https://github.com/ros/angles/issues/28>`_)
* Contributors: Shane Loretz

1.12.4 (2021-03-18)
-------------------
* Upgrade to setuptools (`#23 <https://github.com/ros/angles/issues/23>`_)
* Contributors: Tully Foote

1.12.3 (2020-03-11)
-------------------
* Update the angle normalization function to a simpler implementation (`#19 <https://github.com/ros/angles/issues/19>`_) (`#21 <https://github.com/ros/angles/issues/21>`_)
  * Update the angle normalization function for a simpler alternative
  * Simplify 2*pi angle wrapping.
  * Simplify/fasten the C++ implementation of angle normalization (removes one fmod call)
* Contributors: Alexis Paques

1.12.2 (2020-01-08)
-------------------
* Added support for "large limits" (`#16 <https://github.com/ros/angles/issues/16>`_)
  * Added support for "large limits"
  * shortest_angle_with_large_limits in python
* Contributors: Franco Fusco

1.12.1 (2018-11-20)
-------------------
* Adding export lines to CMakeLists.txt (`#14 <https://github.com/ros/angles/issues/14>`_)
  * Adding export lines to CMakeLists.txt
  This is needed for dependent packages to pick up the include
  directory
* Contributors: Carl Delsey

1.12.0 (2018-06-28)
-------------------
* update for ament and ros2 (`#12 <https://github.com/ros/angles/issues/12>`_)
* Small documentation updates.
* enable for win10 ros2
* Contributors: Brian Fjeldstad, Jonathan Binney, Tully Foote

1.9.11 (2017-04-14)
-------------------
* Add a python implementation of angles
* Do not use catkin_add_gtest if CATKIN_ENABLE_TESTING
* Contributors: David V. Lu, David V. Lu!!, Ryohei Ueda

1.9.10 (2014-12-29)
-------------------
* Export architecture_independent flag in package.xml
* Simply and improve performance of shortest_angular_distance(). adding two unit test cases
* check for CATKIN_ENABLE_TESTING
* Contributors: Derek King, Lukas Bulwahn, Scott K Logan, Tully Foote

1.9.9 (2013-03-23)
------------------
* catkin as a buildtool dependency
* Contributors: Tully Foote

1.9.8 (2012-12-05)
------------------
* Removed 'copyright' tag from package.xml
* Contributors: William Woodall

1.9.7 (2012-10-02 21:23)
------------------------
* fix typo
* Contributors: Vincent Rabaud

1.9.6 (2012-10-02 15:39)
------------------------
* comply to the new catkin API
* Contributors: Vincent Rabaud

1.9.5 (2012-09-16 18:11)
------------------------
* fixes to build
* Contributors: Ioan Sucan

1.9.4 (2012-09-16 01:24)
------------------------
* rename the include folder to angles as it should be
* Contributors: Vincent Rabaud

1.9.3 (2012-09-03 00:55)
------------------------
* fix relative path
* Contributors: Ioan Sucan

1.9.2 (2012-09-03 00:32)
------------------------

1.9.1 (2012-07-24)
------------------
* add proper manifest
* Contributors: Ioan Sucan

1.9.0 (2012-07-23)
------------------
* fix the test for the new headers
* fix the guard
* package builds with catkin
* remove useless header
* copying from geometry/
* Contributors: Ioan Sucan, Vincent Rabaud
