^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package automotive_navigation_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3.0.4 (2020-10-21)
------------------
* Change VelocityAccel comment from lateral to longitudinal, also remove trailing whitespace from all messages (`#23 <https://github.com/astuff/automotive_autonomy_msgs/issues/23>`_)
* Fix package.xml website URL (`#19 <https://github.com/astuff/automotive_autonomy_msgs/issues/19>`_)
* Contributors: Jacob Perron, icolwell-as

3.0.3 (2019-12-31)
------------------
* Add find_package for ros_environment.
* Contributors: Joshua Whitley

3.0.2 (2019-12-19)
------------------
* Adding missing dependency on ros_environment.
* Contributors: Joshua Whitley

3.0.1 (2019-12-12)
------------------
* Merge pull request `#15 <https://github.com/astuff/automotive_autonomy_msgs/issues/15>`_ from astuff/maint/ros1_ros2_hybrid
  ROS1/ROS2 Hybrid Packages
* Adding migration rules.
* Fixing XML linting errors.
* Making all messages ROS2-compliant.
* Hybridizing all packages.
* Contributors: Joshua Whitley

2.0.3 (2018-12-07)
------------------
* Merge pull request `#13 <https://github.com/astuff/automotive_autonomy_msgs/issues/13>`_ from astuff/maint/add_urls
* Adding URLs to package.xml files.
* Contributors: Joshua Whitley, Rinda Gunjala

2.0.2 (2018-08-08)
------------------
* Reorganized messages into renamed packages.
* Changed package name from module_comm_msgs to automotive_navigation_msgs.
* add RouteArray message to handle roads with more than one lane
* Fixing license in package.xml.
* Add ACC max distance in meters so that it can be displayed on shuttle GUI instead of just the 0 to 100% value on the existing highway autopilot GUI
* Add more lane styles to lane boundary message to support double lines
* Add lane boundary messages
* Add new message for velocity and acceleration that includes a calculated covariance
* Add Direction message from marti_localization_msgs
* Add ROS service to request map images
* Updating package.xml to format 2.
* Standardizing package.xml files.
* Version bump and minor formatting clean-up.
* Adding headers to some files that were missing them.
* Moving VelocityAccel to module_comm_msgs.
* Major package reorganization. See README for new package definitions.
* Contributors: Daniel Stanek, Joshua Whitley
