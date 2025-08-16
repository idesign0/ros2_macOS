^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rqt_console
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2.0.3 (2024-02-16)
------------------
* added new maintainer
* Contributors: Arne Hitzmann

2.0.2 (2021-10-27)
------------------
* Use a custom QoS profile based on the default rosout QoS profile (`#37 <https://github.com/ros-visualization/rqt_console/issues/37>`_)
* Contributors: Arne Hitzmann, Cole Tucker

2.0.1 (2021-04-27)
------------------
* Use underscores in setup.cfg instead of dashes (`#31 <https://github.com/ros-visualization/rqt_console/issues/31>`_)
* Contributors: Michel Hidalgo

2.0.0 (2021-03-18)
------------------
* Fix regression introduced in `#21 <https://github.com/ros-visualization/rqt_console/issues/21>`_ (`#28 <https://github.com/ros-visualization/rqt_console/issues/28>`_)
* Changed the build type to ament_python and added setup.cfg (`#21 <https://github.com/ros-visualization/rqt_console/issues/21>`_)
* Contributors: Alejandro Hernández Cordero

1.1.1 (2020-01-14)
------------------
* fix division in Python 3 (`#18 <https://github.com/ros-visualization/rqt_console/issues/18>`_)
* fix highlight filter by message (`#17 <https://github.com/ros-visualization/rqt_console/issues/17>`_)
* fix exclude messages (`#11 <https://github.com/ros-visualization/rqt_console/issues/11>`_)
* add context menu for hiding and showing columns (`#13 <https://github.com/ros-visualization/rqt_console/issues/13>`_)
* remove topics concept which has been removed in ROS 2 (`#16 <https://github.com/ros-visualization/rqt_console/issues/16>`_)
* fix handle_pause_clicked doesn't need args (`#15 <https://github.com/ros-visualization/rqt_console/issues/15>`_)

1.1.0 (2019-10-03)
------------------
* update for Dashing (`#10 <https://github.com/ros-visualization/rqt_console/issues/10>`_)

1.0.1 (2018-12-12)
------------------
* remove unnecesary find_package call (`#9 <https://github.com/ros-visualization/rqt_console/issues/9>`_)

1.0.0 (2018-12-11)
------------------
* porting rqt_console to ros2 (`#8 <https://github.com/ros-visualization/rqt_console/issues/8>`_)
* flake8 (`#7 <https://github.com/ros-visualization/rqt_console/issues/7>`_)
* autopep8 (`#6 <https://github.com/ros-visualization/rqt_console/issues/6>`_)
* Contributors: Mike Lautman

0.4.8 (2017-04-24)
------------------

0.4.7 (2017-03-02)
------------------

0.4.6 (2017-02-27)
------------------

0.4.5 (2017-02-03)
------------------

0.4.4 (2017-01-24)
------------------
* fix regression with Qt 4 (`#414 <https://github.com/ros-visualization/rqt_common_plugins/issues/414>`_)
* fix missing dependency on rqt_py_common (`#408 <https://github.com/ros-visualization/rqt_common_plugins/pull/408>`_)

0.4.3 (2016-11-02)
------------------

0.4.2 (2016-09-19)
------------------
* make message column stretch (`#398 <https://github.com/ros-visualization/rqt_common_plugins/issues/398>`_)

0.4.1 (2016-05-16)
------------------

0.4.0 (2016-04-27)
------------------
* Support Qt 5 (in Kinetic and higher) as well as Qt 4 (in Jade and earlier) (`#359 <https://github.com/ros-visualization/rqt_common_plugins/pull/359>`_)

0.3.13 (2016-03-08)
-------------------

0.3.12 (2015-07-24)
-------------------

0.3.11 (2015-04-30)
-------------------

0.3.10 (2014-10-01)
-------------------
* update plugin scripts to use full name to avoid future naming collisions

0.3.9 (2014-08-18)
------------------

0.3.8 (2014-07-15)
------------------

0.3.7 (2014-07-11)
------------------
* export architecture_independent flag in package.xml (`#254 <https://github.com/ros-visualization/rqt_common_plugins/issues/254>`_)

0.3.6 (2014-06-02)
------------------

0.3.5 (2014-05-07)
------------------

0.3.4 (2014-01-28)
------------------

0.3.3 (2014-01-08)
------------------
* add groups for rqt plugins, renamed some plugins (`#167 <https://github.com/ros-visualization/rqt_common_plugins/issues/167>`_)
* use icons instead of text when available, refactor pause/resume button

0.3.2 (2013-10-14)
------------------

0.3.1 (2013-10-09)
------------------
* rewrite of rqt_console to drastically improve performance (`#186 <https://github.com/ros-visualization/rqt_common_plugins/pull/186>`_)

0.3.0 (2013-08-28)
------------------
* pause button no more saves state (`#125 <https://github.com/ros-visualization/rqt_common_plugins/issues/125>`_)
* persist message limit (`#138 <https://github.com/ros-visualization/rqt_common_plugins/issues/138>`_)
* add ability to set logger level (`#117 <https://github.com/ros-visualization/rqt_common_plugins/issues/117>`_)
* add tooltips to table cells (`#143 <https://github.com/ros-visualization/rqt_common_plugins/issues/143>`_)
* improve labels for filters (`#146 <https://github.com/ros-visualization/rqt_common_plugins/issues/146>`_)
* fix time column when loading data from file (`#160 <https://github.com/ros-visualization/rqt_common_plugins/issues/160>`_)
* fix applying message limit on change (`#133 <https://github.com/ros-visualization/rqt_common_plugins/issues/133>`_)
* fix clear button to remove all messages (`#141 <https://github.com/ros-visualization/rqt_common_plugins/issues/141>`_)
* fix sorting to use row index to decide order between equal values (except for time column) (`#124 <https://github.com/ros-visualization/rqt_common_plugins/issues/124>`_)
* fix locking of message queue
* fix rendering of icons on OS X (`ros-visualization/rqt#83 <https://github.com/ros-visualization/rqt/issues/83>`_)

0.2.17 (2013-07-04)
-------------------
* added missing word in status tip

0.2.16 (2013-04-09 13:33)
-------------------------

0.2.15 (2013-04-09 00:02)
-------------------------

0.2.14 (2013-03-14)
-------------------

0.2.13 (2013-03-11 22:14)
-------------------------

0.2.12 (2013-03-11 13:56)
-------------------------
* Fix; can't add filters when using pyside (`#36 <https://github.com/ros-visualization/rqt_common_plugins/issues/36>`_)

0.2.11 (2013-03-08)
-------------------

0.2.10 (2013-01-22)
-------------------

0.2.9 (2013-01-17)
------------------

0.2.8 (2013-01-11)
------------------

0.2.7 (2012-12-24)
------------------

0.2.6 (2012-12-23)
------------------

0.2.5 (2012-12-21 19:11)
------------------------
* first release of this package into groovy
