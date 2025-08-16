Changelog for package rqt_gui
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.1.7 (2024-02-16)
------------------

1.1.6 (2024-01-24)
------------------

1.1.5 (2023-04-25)
------------------

1.1.4 (2022-05-10)
------------------
* Display basic help information when no plugins are loaded (`#268 <https://github.com/ros-visualization/rqt/issues/268>`_)
* Contributors: Michael Jeronimo

1.1.3 (2022-04-05)
------------------

1.1.2 (2021-08-31)
------------------

1.1.1 (2021-04-12)
------------------

1.1.0 (2021-04-07)
------------------

1.0.7 (2021-02-05)
------------------
* getiterator() renamed to iter() in Python 3.9 (`#239 <https://github.com/ros-visualization/rqt/issues/239>`_)
* Contributors: goekce

1.0.6 (2020-05-05)
------------------
* allow definition of settings file (`#216 <https://github.com/ros-visualization/rqt/issues/216>`_)
* Contributors: Alexander Gutenkunst

1.0.5 (2019-09-30)
------------------

1.0.2 (2019-02-04)
------------------

1.0.1 (2018-12-12)
------------------

1.0.0 (2018-12-11)
------------------
* change to ament_python build type (`#169 <https://github.com/ros-visualization/rqt/issues/169>`_) (`#170 <https://github.com/ros-visualization/rqt/issues/170>`_)
* fix style to pass lint tests (`#172 <https://github.com/ros-visualization/rqt/issues/172>`_)
* port rqt_gui_cpp to ROS 2 (`#163 <https://github.com/ros-visualization/rqt/issues/163>`_)
* add subclass of plugin_context to pass around the rqt node (`#161 <https://github.com/ros-visualization/rqt/issues/161>`_)
* remove pytests in favor of ament_lint_auto, moving rqt_gui resource to package specific destination (`#153 <https://github.com/ros-visualization/rqt/issues/153>`_)
* fix installation of rqt_gui executable (`#152 <https://github.com/ros-visualization/rqt/issues/152>`_)
* add tests to rqt_gui_py (`#150 <https://github.com/ros-visualization/rqt/issues/150>`_)
* port rqt_gui to ROS 2 (`#142 <https://github.com/ros-visualization/rqt/issues/142>`_)
* style changes (`#143 <https://github.com/ros-visualization/rqt/issues/143>`_)
* autopep8 (`#137 <https://github.com/ros-visualization/rqt/issues/137>`_)

0.5.0 (2017-04-24)
------------------
* version bump to match version of migrated package `rqt_py_common`

0.3.2 (2017-01-24)
------------------
* use Python 3 compatible syntax (`#113 <https://github.com/ros-visualization/rqt/pull/113>`_)

0.3.1 (2016-04-01)
------------------

0.3.0 (2016-04-01)
------------------
* switch to Qt5 (`#106 <https://github.com/ros-visualization/rqt/pull/106>`_)

0.2.14 (2014-03-04)
-------------------

0.2.13 (2014-01-08)
-------------------
* added hash of ros package path as prefix for plugin manager settings to allow for one cache per package path setting

0.2.12 (2013-10-09)
-------------------
* improve startup time (`#88 <https://github.com/ros-visualization/rqt/issues/88>`_)
* new icon for rqt

0.2.11 (2013-09-06)
-------------------

0.2.10 (2013-08-21)
-------------------
* add application icon (`#82 <https://github.com/ros-visualization/rqt/issues/82>`_)
* improve error message when plugin xml file does not exist

0.2.9 (2013-06-06)
------------------
* make plugin resources relative to plugin.xml (`ros-visualization/qt_gui_core#16 <https://github.com/ros-visualization/qt_gui_core/issues/16>`_)
* use standard rospy function to filter remapping arguments (`#76 <https://github.com/ros-visualization/rqt/issues/76>`_)
* fix help provider

0.2.8 (2013-01-11)
------------------
* properly ignore ROS remapping arguments when passed in via command line

0.2.7 (2012-12-31)
------------------
* first public release for Groovy
