^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rqt_graph
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.3.1 (2024-05-15)
------------------
* Fixed fit_in_view icon button (backport `#95 <https://github.com/ros-visualization/rqt_graph/issues/95>`_) (`#97 <https://github.com/ros-visualization/rqt_graph/issues/97>`_)
* Contributors: mergify[bot]

1.3.0 (2022-09-13)
------------------

1.2.1 (2022-01-14)
------------------
* Fix quiet filtering for node_topic graphs (`#70 <https://github.com/ros-visualization/rqt_graph/issues/70>`_)
* Contributors: David V. Lu!!

1.2.0 (2021-03-30)
------------------
* Make topics that have qos incompatibilities red in the graph, add information to the node tooltip (`#61 <https://github.com/ros-visualization/rqt_graph/issues/61>`_)
* Add node name, topic name, and endpoint kind to the qos edge tooltip (`#60 <https://github.com/ros-visualization/rqt_graph/issues/60>`_)
* Contributors: Ivan Santiago Paunovic

1.1.1 (2021-03-18)
------------------
* Update maintainers for ROS2 branches (`#55 <https://github.com/ros-visualization/rqt_graph/issues/55>`_)
* Contributors: Michael Jeronimo

1.1.0 (2020-09-23)
------------------
* add edge tooltip with QoS of publishers and subscribers (`#53 <https://github.com/ros-visualization/rqt_graph/issues/53>`_)

1.0.5 (2020-09-18)
------------------
* install executable rqt_graph (`#49 <https://github.com/ros-visualization/rqt_graph/issues/49>`_)
* add setup.cfg with script install directories (`#42 <https://github.com/ros-visualization/rqt_graph/issues/42>`_)
* add pytest.ini so local tests don't display warning (`#48 <https://github.com/ros-visualization/rqt_graph/issues/48>`_)

1.0.4 (2020-01-06)
------------------
* skip removal of non-present tf connections (`#40 <https://github.com/ros-visualization/rqt_graph/issues/40>`_)

1.0.3 (2019-10-23)
------------------
* Add topic edges for the Nodes Only view mode. (`#35 <https://github.com/ros-visualization/rqt_graph/issues/35>`_)
* Fix node not found during refresh when namespace is not empty (`#34 <https://github.com/ros-visualization/rqt_graph/issues/34>`_)
* Contributors: Dirk Thomas, Michel Hidalgo, Xavier BROQUERE

1.0.2 (2019-10-03)
------------------
* fix typo that caused a crash when trying to load a dot file (`#31 <https://github.com/ros-visualization/rqt_graph/issues/31>`_)

1.0.0 (2019-02-04)
------------------
* copy base rosgraph.impl.graph file (`#23 <https://github.com/ros-visualization/rqt_graph/issues/23>`_)
* port to ROS 2 (`#22 <https://github.com/ros-visualization/rqt_graph/issues/22>`_)
* autopep8 and gitignore (`#19 <https://github.com/ros-visualization/rqt_graph/issues/19>`_)
* replace str() by unicode() (`#17 <https://github.com/ros-visualization/rqt_graph/issues/17>`_)
* edge class is unhashable so cannot put into a set, use a list instead (`#16 <https://github.com/ros-visualization/rqt_graph/issues/16>`_)

0.4.10 (2018-05-22)
-------------------
* add nested subnamespaces and more options to hide or group topics (`#13 <https://github.com/ros-visualization/rqt_graph/issues/13>`_)

0.4.9 (2017-07-27)
------------------
* fixes for Python 3 (`#10 <https://github.com/ros-visualization/rqt_graph/issues/10>`_)
* only show namespaces if all entities have a common non-empty base path (`#8 <https://github.com/ros-visualization/rqt_graph/issues/8>`_)

0.4.8 (2017-04-24)
------------------

0.4.7 (2017-03-02)
------------------
* fix statistics are enabled, regression of 0.4.4 (`#428 <https://github.com/ros-visualization/rqt_common_plugins/issues/428>`_)

0.4.6 (2017-02-27)
------------------

0.4.5 (2017-02-03)
------------------

0.4.4 (2017-01-24)
------------------
* use Python 3 compatible syntax (`#421 <https://github.com/ros-visualization/rqt_common_plugins/pull/421>`_)

0.4.3 (2016-11-02)
------------------

0.4.2 (2016-09-19)
------------------

0.4.1 (2016-05-16)
------------------
* fix mouse wheel delta in Qt 5 (`#376 <https://github.com/ros-visualization/rqt_common_plugins/issues/376>`_)

0.4.0 (2016-04-27)
------------------
* Support Qt 5 (in Kinetic and higher) as well as Qt 4 (in Jade and earlier) (`#359 <https://github.com/ros-visualization/rqt_common_plugins/pull/359>`_)

0.3.13 (2016-03-08)
-------------------
* Remove repeated prefices from buttons
* Prefix all node and topic names with `n\_` and `t\_` respectively, to allow dot to distinguish them
* Contributors: Eric Wieser

0.3.12 (2015-07-24)
-------------------

0.3.11 (2015-04-30)
-------------------
* fix duplicate rendering of statistics information (`#283 <https://github.com/ros-visualization/rqt_common_plugins/issues/283>`_)

0.3.10 (2014-10-01)
-------------------
* update plugin scripts to use full name to avoid future naming collisions

0.3.9 (2014-08-18)
------------------
* fix rendering of namespace boxes (`#266 <https://github.com/ros-visualization/rqt_common_plugins/issues/266>`_)

0.3.8 (2014-07-15)
------------------

0.3.7 (2014-07-11)
------------------
* fix compatibility with Groovy, use TopicStatistics only if available (`#252 <https://github.com/ros-visualization/rqt_common_plugins/issues/252>`_)
* export architecture_independent flag in package.xml (`#254 <https://github.com/ros-visualization/rqt_common_plugins/issues/254>`_)

0.3.6 (2014-06-02)
------------------

0.3.5 (2014-05-07)
------------------
* add displaying of topic/connection statistics along edges (`#214 <https://github.com/ros-visualization/rqt_common_plugins/pull/214>`_)
* using CATKIN_ENABLE_TESTING to optionally configure tests (`#220 <https://github.com/ros-visualization/rqt_common_plugins/pull/220>`_)

0.3.4 (2014-01-28)
------------------

0.3.3 (2014-01-08)
------------------
* add groups for rqt plugins, renamed some plugins (`#167 <https://github.com/ros-visualization/rqt_common_plugins/issues/167>`_)

0.3.2 (2013-10-14)
------------------

0.3.1 (2013-10-09)
------------------
* modified zooming method to work better on high-res trackpads like Macbook Pros (`#187 <https://github.com/ros-visualization/rqt_common_plugins/pull/187>`_)

0.3.0 (2013-08-28)
------------------
* fix rendering of icons on OS X (`ros-visualization/rqt#83 <https://github.com/ros-visualization/rqt/issues/83>`_)

0.2.17 (2013-07-04)
-------------------
* Improve checkbox labels and tooltips wording.

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
