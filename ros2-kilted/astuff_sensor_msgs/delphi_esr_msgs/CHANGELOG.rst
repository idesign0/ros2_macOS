^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package delphi_esr_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4.0.0 (2023-04-24)
------------------

3.3.0 (2021-06-18)
------------------

3.2.0 (2021-03-12)
------------------
* Bag migration rules cleanup (`#76 <https://github.com/astuff/astuff_sensor_msgs/issues/76>`_)
  * delphi_esr_msgs: Consolidate bag migration rules
* Update delphi_esr_msgs (`#69 <https://github.com/astuff/astuff_sensor_msgs/issues/69>`_)
  * Update delphi_esr_msgs to closer match documentation and fix typos
  * Update delphi_esr_msgs CMakeLists.txt
  * Change from EsrTrackMotionPower to EsrTrackMotionPowerTrack
  * Add migration file for changes to delphi_esr_msgs
  * Fix up old migration rules, order was wrong, remove duplication
  * Factor out install directive for migration folder
  * Fix CATKIN_PACKAGE_SHARE_DESTINATION for ROS2
* Contributors: icolwell-as

3.1.0 (2020-06-17)
------------------

3.0.1 (2020-01-23)
------------------
* Adding ros_environment as required package to all packages.
* ROS1/2 Hybrid: delphi_esr_msgs (`#37 <https://github.com/astuff/astuff_sensor_msgs/issues/37>`_)
  * Adding hybrid CI.
  * Hybridizing delphi_esr_msgs.
  * Updating package.xml files for ROS2 rosdep.
  * Adding message migration rules.
* Contributors: Joshua Whitley

2.3.1 (2018-12-07)
------------------
* Merge pull request `#31 <https://github.com/astuff/astuff_sensor_msgs/issues/31>`_ from astuff/maint/add_urls
* Adding URLs to package.xml files.
* Contributors: Daniel-Stanek, Joshua Whitley

2.3.0 (2018-10-04)
------------------

2.2.2 (2018-08-30)
------------------

2.2.1 (2018-08-08)
------------------

2.1.0 (2018-05-14)
------------------
* Updating package.xml to format 2.
* Standardizing package.xml files.
* Removing properties that aren't part of raw message in EsrEthTx.
* Contributors: Joshua Whitley
