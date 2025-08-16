^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rqt_reconfigure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.1.2 (2023-11-13)
------------------
* Fixed executor conflict (backport `#126 <https://github.com/ros-visualization/rqt_reconfigure/issues/126>`_) (`#136 <https://github.com/ros-visualization/rqt_reconfigure/issues/136>`_)
* Contributors: mergify[bot]

1.1.1 (2022-09-13)
------------------
* Fix float slider step size (`#117 <https://github.com/ros-visualization/rqt_reconfigure/issues/117>`_) (`#119 <https://github.com/ros-visualization/rqt_reconfigure/issues/119>`_)
* Contributors: mergify[bot]

1.0.8 (2021-05-10)
------------------
* Cleanups to the install scripts. (`#103 <https://github.com/ros-visualization/rqt_reconfigure/issues/103>`_)
* Contributors: Chris Lalancette

1.0.7 (2021-02-19)
------------------
* Fix a flake8 warning. (`#99 <https://github.com/ros-visualization/rqt_reconfigure/issues/99>`_)
* Use timeouts in service calls to avoid hangs (`#98 <https://github.com/ros-visualization/rqt_reconfigure/issues/98>`_)
* Add maintainer to package.xml (`#95 <https://github.com/ros-visualization/rqt_reconfigure/issues/95>`_)
* Contributors: Chris Lalancette, Michael Jeronimo

1.0.6 (2020-08-03)
------------------
* Save instance state in rqt settings (`#90 <https://github.com/ros-visualization/rqt_reconfigure/issues/90>`_)
* Use safe YAML loader (`#89 <https://github.com/ros-visualization/rqt_reconfigure/issues/89>`_)
* Don't process scroll events unless specifically focused (`#88 <https://github.com/ros-visualization/rqt_reconfigure/issues/88>`_)
* Fix node selection from command line (`#87 <https://github.com/ros-visualization/rqt_reconfigure/issues/87>`_)
* Add pytest.ini so local tests don't display warning (`#91 <https://github.com/ros-visualization/rqt_reconfigure/issues/91>`_)
* Support PEP 338 invocation of rqt_reconfigure (`#85 <https://github.com/ros-visualization/rqt_reconfigure/issues/85>`_)
* Fixed package to run with ros2 run (`#81 <https://github.com/ros-visualization/rqt_reconfigure/issues/81>`__)
* Contributors: Alejandro Hernández Cordero, Chris Lalancette, Scott K Logan

1.0.5 (2020-06-02)
------------------
* Fix new flake8 warning (`#65 <https://github.com/ros-visualization/rqt_reconfigure/issues/65>`_)
* Contributors: Dirk Thomas

1.0.4 (2019-10-10)
------------------
* install package marker (#55) (`#55 <https://github.com/ros-visualization/rqt_reconfigure/pull/55>`_)

1.0.3 (2019-08-23)
------------------
* Drop ROS 1 info from package description (`#54 <https://github.com/ros-visualization/rqt_reconfigure/issues/54>`_)
* Contributors: Scott K Logan

1.0.2 (2019-08-23)
------------------
* Use a consistent formatting method when logging (`#49 <https://github.com/ros-visualization/rqt_reconfigure/issues/49>`_)
  This will improve logging function compatibility between ROS 1 and
  ROS 2.
* Re-format license headers to conform to ROS templates (`#39 <https://github.com/ros-visualization/rqt_reconfigure/issues/39>`_)
  This change adds a line to the license headers specifying the name of
  the license that follows. It also re-formats the recently added LICENSE
  and CONTRIBUTING.md files to match the templates.
* Rename DynreconfClientWidget => ParamClientWidget (`#36 <https://github.com/ros-visualization/rqt_reconfigure/issues/36>`_)
* Add ament tests
* Add LICENSE and CONTRIBUTING.md (`#37 <https://github.com/ros-visualization/rqt_reconfigure/issues/37>`_)
* Format per linter suggestions and run tests (`#35 <https://github.com/ros-visualization/rqt_reconfigure/issues/35>`_)
* Fix some linter errors and get tests running (`#33 <https://github.com/ros-visualization/rqt_reconfigure/issues/33>`_)
* Pull logging methods into a separate file (`#34 <https://github.com/ros-visualization/rqt_reconfigure/issues/34>`_)
* Update to package.xml format 2 (`#32 <https://github.com/ros-visualization/rqt_reconfigure/issues/32>`_)
* Migration to ROS2
* Contributors: Gonzalo de Pedro, Gonzo, Michel Hidalgo, Scott K Logan, Timon Engelke

0.4.10 (2018-04-19)
-------------------
* Lazy load dynamic_reconfigure client for each node
  Fixes `#20 <https://github.com/ros-visualization/rqt_reconfigure/issues/20>`_
* Use English locale in QDoubleValidator
  Fixes `#21 <https://github.com/ros-visualization/rqt_reconfigure/issues/21>`_
* Contributors: Arkady Shapkin, Yuki Furuta

0.4.9 (2018-01-30)
------------------
* Added error handling for dynamic_reconfigure exceptions (`#10 <https://github.com/ros-visualization/rqt_reconfigure/pull/10>`_)
* Fix left pane tree view resizing (`#11 <https://github.com/ros-visualization/rqt_reconfigure/pull/11>`_)

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
* replace setShown with setVisible (`#418 <https://github.com/ros-visualization/rqt_common_plugins/issues/418>`_)
* use Python 3 compatible syntax (`#421 <https://github.com/ros-visualization/rqt_common_plugins/pull/421>`_)
* add buttons to 'save' to and 'load' from file (`#406 <https://github.com/ros-visualization/rqt_common_plugins/pull/406>`_)

0.4.3 (2016-11-02)
------------------

0.4.2 (2016-09-19)
------------------

0.4.1 (2016-05-16)
------------------
* fix accessing attribute superseded in Qt5 (`#370 <https://github.com/ros-visualization/rqt_common_plugins/issues/370>`_)

0.4.0 (2016-04-27)
------------------
* Support Qt 5 (in Kinetic and higher) as well as Qt 4 (in Jade and earlier) (`#359 <https://github.com/ros-visualization/rqt_common_plugins/pull/359>`_)

0.3.13 (2016-03-08)
-------------------

0.3.12 (2015-07-24)
-------------------
* Added refresh button to re-scan reconfigure server list
* Now retains functioning nodes when refreshing
* Contributors: Kei Okada, Scott K Logan

0.3.11 (2015-04-30)
-------------------
* restore support for parameter groups (`#162 <https://github.com/ros-visualization/rqt_common_plugins/issues/162>`_)
* fix background colors for dark themes (`#293 <https://github.com/ros-visualization/rqt_common_plugins/issues/293>`_)

0.3.10 (2014-10-01)
-------------------
* update plugin scripts to use full name to avoid future naming collisions

0.3.9 (2014-08-18)
------------------

0.3.8 (2014-07-15)
------------------

0.3.7 (2014-07-11)
------------------
* fix slider bar, add context menus for common operations (`#251 <https://github.com/ros-visualization/rqt_common_plugins/issues/251>`_)
* fix bug in float range calculations (`#241 <https://github.com/ros-visualization/rqt_common_plugins/issues/241>`_)
* remove experimental suffix from rqt_reconfigure (`#256 <https://github.com/ros-visualization/rqt_common_plugins/issues/256>`_)
* export architecture_independent flag in package.xml (`#254 <https://github.com/ros-visualization/rqt_common_plugins/issues/254>`_)

0.3.6 (2014-06-02)
------------------
* remove unnecessary margins to improve usability on small screens (`#228 <https://github.com/ros-visualization/rqt_common_plugins/issues/228>`_)

0.3.5 (2014-05-07)
------------------
* numerous improvements and bug fixes (`#209 <https://github.com/ros-visualization/rqt_common_plugins/pull/209>`_, `#210 <https://github.com/ros-visualization/rqt_common_plugins/pull/210>`_)
* add option to open list of names from command line (`#214 <https://github.com/ros-visualization/rqt_common_plugins/pull/214>`_)

0.3.4 (2014-01-28)
------------------

0.3.3 (2014-01-08)
------------------
* add groups for rqt plugins, renamed some plugins (`#167 <https://github.com/ros-visualization/rqt_common_plugins/issues/167>`_)
* mark rqt_launch and rqt_reconfigure as experimental (`#167 <https://github.com/ros-visualization/rqt_common_plugins/issues/167>`_)

0.3.2 (2013-10-14)
------------------

0.3.1 (2013-10-09)
------------------

0.3.0 (2013-08-28)
------------------
* fix updating range limits (`#108 <https://github.com/ros-visualization/rqt_common_plugins/issues/108>`_)
* fix layout quirks (`#150 <https://github.com/ros-visualization/rqt_common_plugins/issues/150>`_)
* fix icon for closing a node (`#48 <https://github.com/ros-visualization/rqt_common_plugins/issues/48>`_)
* fix handling of enum parameters with strings

0.2.17 (2013-07-04)
-------------------
* Improvement; "GUI hangs for awhile or completely, when any one of nodes doesn't return any value" (`#81 <https://github.com/ros-visualization/rqt_common_plugins/issues/81>`__)

0.2.16 (2013-04-09 13:33)
-------------------------

0.2.15 (2013-04-09 00:02)
-------------------------
* Fix; Segmentation fault using integer slider (`#63 <https://github.com/ros-visualization/rqt_common_plugins/issues/63>`_)

0.2.14 (2013-03-14)
-------------------

0.2.13 (2013-03-11 22:14)
-------------------------

0.2.12 (2013-03-11 13:56)
-------------------------
* Improve performance significantly upon launch (`#45 <https://github.com/ros-visualization/rqt_common_plugins/issues/45>`_)

0.2.11 (2013-03-08)
-------------------

0.2.10 (2013-01-22)
-------------------

0.2.9 (2013-01-17)
------------------
* Add feature to delete of shown nodes feature

0.2.8 (2013-01-11)
------------------
* Fix; No Interaction with Boolean values (`#2 <https://github.com/ros-visualization/rqt_common_plugins/issues/2>`_)

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
* renamed rqt_param to rqt_reconfigure (added missing file)
* first release of this package into groovy
