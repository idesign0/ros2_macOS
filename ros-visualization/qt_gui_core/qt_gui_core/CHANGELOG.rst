^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package qt_gui_core
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2.2.4 (2025-03-25)
------------------

2.2.3 (2024-01-24)
------------------

2.2.2 (2022-08-15)
------------------

2.2.1 (2022-05-10)
------------------

2.2.0 (2022-03-01)
------------------

2.1.1 (2022-01-14)
------------------

2.1.0 (2021-11-02)
------------------

2.0.1 (2021-04-29)
------------------

2.0.0 (2021-01-26)
------------------

1.1.2 (2020-09-18)
------------------

1.1.1 (2020-08-03)
------------------

1.1.0 (2020-05-27)
------------------

1.0.9 (2020-05-26)
------------------

1.0.8 (2020-05-05)
------------------

1.0.7 (2019-09-30)
------------------

1.0.6 (2019-06-10)
------------------

1.0.5 (2019-05-29)
------------------

1.0.4 (2019-02-08)
------------------
* remove obsolete maintainer (`#159 <https://github.com/ros-visualization/qt_gui_core/issues/159>`_)
* Contributors: Dirk Thomas

1.0.3 (2018-12-11 15:03)
------------------------

1.0.2 (2018-12-11 11:10)
------------------------

1.0.1 (2018-12-11 09:16)
------------------------

1.0.0 (2018-12-10)
------------------
* Ros2 port (`#135 <https://github.com/ros-visualization/qt_gui_core/issues/135>`_)
  Porting to ROS2
* Contributors: brawner

0.3.11 (2018-08-29)
-------------------

0.3.10 (2018-08-05)
-------------------

0.3.9 (2018-08-03)
------------------

0.3.8 (2017-11-03)
------------------

0.3.7 (2017-10-25)
------------------

0.3.6 (2017-08-03)
------------------
* Add dhood as maintainer (`#101 <https://github.com/ros-visualization/qt_gui_core/issues/101>`_)
* Contributors: dhood

0.3.5 (2017-07-27)
------------------

0.3.4 (2017-01-24)
------------------

0.3.3 (2016-09-19)
------------------

0.3.2 (2016-04-21)
------------------

0.3.1 (2016-04-18)
------------------

0.3.0 (2016-04-01)
------------------
* Merge pull request `#64 <https://github.com/ros-visualization/qt_gui_core/issues/64>`_ from ros-visualization/qt5
  switch to Qt5
* switch to Qt5
* Contributors: Dirk Thomas

0.2.30 (2016-03-30)
-------------------

0.2.29 (2015-09-19)
-------------------

0.2.28 (2015-06-08)
-------------------

0.2.27 (2015-04-29)
-------------------

0.2.26 (2014-08-18)
-------------------

0.2.25 (2014-07-10)
-------------------

0.2.24 (2014-05-21)
-------------------

0.2.23 (2014-05-07)
-------------------

0.2.22 (2014-03-04)
-------------------

0.2.21 (2014-02-12)
-------------------

0.2.20 (2014-01-19)
-------------------

0.2.19 (2014-01-08)
-------------------
* "0.2.19"
* Contributors: Dirk Thomas

0.2.18 (2013-10-09)
-------------------

0.2.17 (2013-08-21)
-------------------

0.2.16 (2013-06-06)
-------------------
* Removed unsuitable maintainer.
* Contributors: Isaac Saito

0.2.15 (2013-04-02)
-------------------
* Adding CMakeLists.txt to qt_gui_core metapackage
* Contributors: William Woodall

0.2.14 (2013-03-28 22:42)
-------------------------

0.2.13 (2013-03-28 18:08)
-------------------------
* all packages) A maintainer added, email address updated
* Contributors: Isaac Saito

0.2.12 (2013-01-17)
-------------------

0.2.11 (2013-01-13)
-------------------

0.2.10 (2013-01-11)
-------------------

0.2.9 (2012-12-21)
------------------

0.2.8 (2012-12-06)
------------------

0.2.7 (2012-11-30)
------------------

0.2.6 (2012-11-19 13:47)
------------------------

0.2.5 (2012-11-19 11:13)
------------------------

0.2.4 (2012-11-19 10:56)
------------------------

0.2.3 (2012-11-15)
------------------
* add metapackage
* Contributors: Dirk Thomas

0.2.2 (2012-11-14 19:10)
------------------------

0.2.1 (2012-11-14 00:32)
------------------------

0.2.0 (2012-11-13)
------------------
* catch exception instance with as instead of comma for Pzthon 3.x compatibility
* made colored terminal output depend on environment
* added verbose option, suppress qDebug() by default and colorize output
* fixed containers
* modified print/qDebug/qWarning outputs to be more consistent
* use different settings files for qt_gui and rqt_gui
* updated review status
* fixed about dialog when used from qt_gui_app
* Merge branch 'master' of https://kforge.ros.org/visualization/ros_gui
* added missing include, fixed spelling
* removed specific Qt version CMake < 2.8.5 can only not handle full versions (including patch) and the exact required version is not obvious
* modified help to use url from manifest
* prevent adding the same widget multiple times
* updated spelling
* enhanced API doc of PluginContext with ownership information
* code formatting according to pep8
* more updates to API doc
* code formatting according to pep8
* code formatting according to pep8
* updated API doc
* added more verbose comments for public API
* changed some labels
* fixed about handler
* colorizing stacks as a checkbox and implemented in plugin
* factory allowing to set edge style
* API cleanup
* removed need to notify framework about changed window titles, now automatically detected
* modified detection of main filename to work with package-relative imports in subprocesses
* robust against missing edge entry
* unescape newline in node and edge labels
* ignore dot nodes with style=invis (invisible)
* treating edges with same labels as siblings as a parameter
* more robust against missing node width and height
* renamed / disabled test
* better error msg
* renamed unit test
* relaxed dotcode checks in unit tests
* pydot factory robust against invalid names
* added .gitignore files
* explicitly name public/supported API
* garbage already added widgets when plugin fails to load
* raise exception when load fails
* using new shiboken check provided by python_qt_binding to test if it supports QGenericReturnArgument
* fixed compiler warning
* fixed compiler warning
* added missing const in cpp classes, reformated methods in cpp::PluginContext to camel case
* modified tag name in qtgui plugin manifest
* modified semantic of plugin manifest, renamed file names according to PEP 8, refactored relative imports according to PEP 328
* removed comment from description (which goes into wiki)
* removed electric support from code using pluginlib since the nodelet api does not work anyway
* major renaming and refactoring of all packages
* renamed packages and moved into separate stacks (refactoring not yet completed)
* Contributors: Aaron Blasdel, Dirk Thomas, Dorian Scholz, Thibault Kruse
