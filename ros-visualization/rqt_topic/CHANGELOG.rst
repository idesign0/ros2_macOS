^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rqt_topic
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.5.0 (2022-09-13)
------------------

1.3.0 (2022-05-10)
------------------
* Fix the display of array type elements. (`#41 <https://github.com/ros-visualization/rqt_topic/issues/41>`_)
* Fix removal of topics while they are being monitored. (`#39 <https://github.com/ros-visualization/rqt_topic/issues/39>`_)
* Contributors: Chris Lalancette

1.2.3 (2022-04-06)
------------------
* Update maintainers to Audrow Nash (`#37 <https://github.com/ros-visualization/rqt_topic/issues/37>`_)
* Contributors: Audrow Nash

1.2.2 (2021-08-31)
------------------
* Fix modern setuptools warning about dashes instead of underscores (`#34 <https://github.com/ros-visualization/rqt_topic/issues/34>`_)
* Remove obsolete warnings (`#31 <https://github.com/ros-visualization/rqt_topic/issues/31>`_)
* Contributors: Chris Lalancette, Michael Jeronimo

1.2.1 (2021-04-06)
------------------
* Add pytest.ini to silence warnings when running locally.
* Fix warnings pointed out by flake8.
* Contributors: Chris Lalancette

1.2.0 (2021-03-19)
------------------
* Created an entry-point for rqt_topic in setup.py (`#16 <https://github.com/ros-visualization/rqt_topic/issues/16>`_)
* Fix flake8 errors and add linter tests (`#28 <https://github.com/ros-visualization/rqt_topic/issues/28>`_)
* Update Open Robotics Maintainer (`#26 <https://github.com/ros-visualization/rqt_topic/issues/26>`_)
* Use raw / non-string value for ordering (`#23 <https://github.com/ros-visualization/rqt_topic/issues/23>`_)
* Support order fields as defined in message (`#22 <https://github.com/ros-visualization/rqt_topic/issues/22>`_)
* Fix the type cell value for sequence items (`#21 <https://github.com/ros-visualization/rqt_topic/issues/21>`_)
* Updated version package and license in setup.py (`#17 <https://github.com/ros-visualization/rqt_topic/issues/17>`_)
* Contributors: Alejandro Hernández Cordero, Dirk Thomas, Scott K Logan

1.1.0 (2019-11-13)
------------------
* set qos_profile (`#14 <https://github.com/ros-visualization/rqt_topic/issues/14>`_)

1.0.0 (2019-03-05)
------------------
* removing the spinner from the ros2 port (`#11 <https://github.com/ros-visualization/rqt_topic/issues/11>`_)
* Merge pull request `#9 <https://github.com/ros-visualization/rqt_topic/issues/9>`_ from ros-visualization/ros2_port
* Porting to ROS2
* autopep8 (`#6 <https://github.com/ros-visualization/rqt_topic/issues/6>`_)

0.4.10 (2017-11-16)
-------------------
* fix order by bandwidth column (`#4 <https://github.com/ros-visualization/rqt_topic/issues/4>`_)

0.4.9 (2017-11-06)
------------------
* fix section resize toggle with Qt5 (`#2 <https://github.com/ros-visualization/rqt_topic/issues/2>`_)

0.4.8 (2017-04-24)
------------------

0.4.7 (2017-03-02)
------------------

0.4.6 (2017-02-27)
------------------
* remove usage of undocumented cStringIO.StringIO len attribute (`#434 <https://github.com/ros-visualization/rqt_common_plugins/pull/434>`_)

0.4.5 (2017-02-03)
------------------

0.4.4 (2017-01-24)
------------------
* use Python 3 compatible syntax (`#421 <https://github.com/ros-visualization/rqt_common_plugins/pull/421>`_)
* catch unhandled exceptions when rosmaster disappears while widget is running (`#419 <https://github.com/ros-visualization/rqt_common_plugins/pull/419>`_)

0.4.3 (2016-11-02)
------------------

0.4.2 (2016-09-19)
------------------

0.4.1 (2016-05-16)
------------------

0.4.0 (2016-04-27)
------------------
* Support Qt 5 (in Kinetic and higher) as well as Qt 4 (in Jade and earlier) (`#359 <https://github.com/ros-visualization/rqt_common_plugins/pull/359>`_)

0.3.13 (2016-03-08)
-------------------
* check for divide by zero and data failures
* Contributors: Aaron Blasdel

0.3.12 (2015-07-24)
-------------------
* Save/Restore of headers added
* Contributors: Aaron Blasdel

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
* catch and show exceptions `#198 <https://github.com/ros-visualization/rqt_common_plugins/issues/198>`_

0.3.2 (2013-10-14)
------------------

0.3.1 (2013-10-09)
------------------
* improve rqt_topic initialization time (`#62 <https://github.com/ros-visualization/rqt_common_plugins/issues/62>`_)
* modified toggling topics to use checkbox instead of context menu (`#75 <https://github.com/ros-visualization/rqt_common_plugins/issues/75>`_)

0.3.0 (2013-08-28)
------------------
* fix cleaning old data in rqt_topic (fix `#74 <https://github.com/ros-visualization/rqt_common_plugins/issues/74>`_)

0.2.17 (2013-07-04)
-------------------

0.2.16 (2013-04-09 13:33)
-------------------------

0.2.15 (2013-04-09 00:02)
-------------------------
* Improve API (now either name or msg type are select-able in order to select which topics to monitor).
* API change to accept a list of the topics that this plugin watches.

0.2.14 (2013-03-14)
-------------------

0.2.13 (2013-03-11 22:14)
-------------------------

0.2.12 (2013-03-11 13:56)
-------------------------

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

0.2.4 (2012-12-21 01:13)
------------------------

0.2.3 (2012-12-21 00:24)
------------------------

0.2.2 (2012-12-20 18:29)
------------------------

0.2.1 (2012-12-20 17:47)
------------------------

0.2.0 (2012-12-20 17:39)
------------------------
* first release of this package into groovy
