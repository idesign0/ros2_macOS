^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rqt
^^^^^^^^^^^^^^^^^^^^^^^^^

1.1.7 (2024-02-16)
------------------

1.1.6 (2024-01-24)
------------------

1.1.5 (2023-04-25)
------------------
* fix build of `rqt` with `setuptools>=v61.0.0` (`#271 <https://github.com/ros-visualization/rqt/issues/271>`_) (`#286 <https://github.com/ros-visualization/rqt/issues/286>`_)
* Contributors: mergify[bot]

1.1.4 (2022-05-10)
------------------

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
* 1.0.7 (`#243 <https://github.com/ros-visualization/rqt/issues/243>`_)
* Remove Dirk from maintainers in package.xml files per request. (`#236 <https://github.com/ros-visualization/rqt/issues/236>`_)
* Update maintainers for the crystal-devel branch (`#234 <https://github.com/ros-visualization/rqt/issues/234>`_)
* Contributors: Michael Jeronimo, Scott K Logan

1.0.6 (2020-05-05)
------------------

1.0.5 (2019-09-30)
------------------

1.0.4 (2019-05-29)
------------------

1.0.3 (2019-05-09)
------------------

1.0.2 (2019-02-04)
------------------

1.0.1 (2018-12-12)
------------------

1.0.0 (2018-12-11)
------------------
* Moving rqt to pure python (`#169 <https://github.com/ros-visualization/rqt/issues/169>`_) (`#170 <https://github.com/ros-visualization/rqt/issues/170>`_)
* Changing meta package to ament package (`#146 <https://github.com/ros-visualization/rqt/issues/146>`_)
* Contributors: Mike Lautman, brawner

0.5.0 (2017-04-24)
------------------
* unify version number before next release
* fixed matplot when using arrays, merged some code from matplot and plot
* modified print/qDebug/qWarning outputs to be more consistent
* moved update of publisher plugin comboboxes into worker thread
* code formatting according to pep8
* modified semantic of plugin manifest, renamed file names according to PEP 8, refactored relative imports according to PEP 328
* major renaming and refactoring of all packages
* renamed packages and moved into separate stacks (refactoring not yet completed)
* Contributors: Dirk Thomas, Dorian Scholz

0.3.2 (2017-01-24)
------------------

0.3.1 (2016-04-01 15:51)
------------------------

0.3.0 (2016-04-01 14:19)
------------------------
* Merge pull request `#106 <https://github.com/ros-visualization/rqt/issues/106>`_ from ros-visualization/qt5
  switch to Qt5
* switch to Qt5
* remove obsolete email address
* Contributors: Dirk Thomas

0.2.14 (2014-03-04)
-------------------

0.2.13 (2014-01-08)
-------------------
* "0.2.13"
* Contributors: Dirk Thomas

0.2.12 (2013-10-09)
-------------------

0.2.11 (2013-09-06)
-------------------

0.2.10 (2013-08-21)
-------------------

0.2.9 (2013-06-06)
------------------
* update maintainer
* Updated metapackage description.
* Conform to REP-0127
* Correct spelling/grammer error
* rqt metapackage maintainer added
* rqt) email addr changed
* rqt) package.xml elaborated so that wiki page ros.org/wiki/rqt can show pointer to relevant pkgs.
* Added an original author that was missing.
* get maintainer status for rqt infrastructure
* Contributors: Dirk Thomas, Isaac Saito

0.2.8 (2013-01-11)
------------------

0.2.7 (2012-12-31)
------------------

0.2.6 (2012-12-23)
------------------
* missing author tag
* remove other plugins after duplicating repo
* Contributors: Aaron Blasdel, Dirk Thomas

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

0.1.7 (2012-12-13 16:17)
------------------------

0.1.6 (2012-12-13 14:43)
------------------------

0.1.5 (2012-12-08)
------------------

0.1.4 (2012-11-20)
------------------

0.1.3 (2012-11-19)
------------------
* removed qt_gui_core, moved rqt to root
* renamed rqt_tf_graph plugin to rqt_tf_tree
* rqt_console: not in formatting and save adding character bug
* rqt_console: file extension defaulting
* rqt_console: fixes for ini file loading when changing between perspectives, code formatting
* rqt_console: added todos
* rqt_console: code style
* updated plugin descriptions
* rqt_console: function reshuffle, documentation and commenting
* catch exception instance with as instead of comma for Pzthon 3.x compatibility
* rqt_logger_level: code style
* Added TF plugin like rosrun tf view_frames
* revert part of commit fecb8d81d
  rosmake -t replaces make test
* Fixed saving against filterproxymodel, various maintainability edits, ui prettying up, error in location filtering fixed, reorganized source files, regular expression escaping fixed
* rqt_console: Re-implemented filter save/load with in the new filter system
* Fixed file load, it now properly emits a RowsInserted signal after it is finished
* Added Custom Filter for filtering multiple types of data at once
* Reenabled and redid code for rightclick menus in new filtering scheme
* Fixed Sorting/Highlighting conflict. They function together now
* Major overhaul of the message filtering system.
* rqt_console: Modified the configuration panel to allow setting of messagelimits and removed the logger level interface/code
* rqt_logger_level: Initial commit. port of Rxloggerlevel
* rqt_console: Added automatic stream pause on file load
* Support for pretty print message browser, accessable via double click or right click menu
* Implemented Message Limiting, added some exception handling/raising
* Rightclick Menu reformat
* UI renaming and rightclick menu functionality, added tr functions for easy translation, added raise statements, general class and file Reorganization and readability changes, changed how the exclude/include items work for non message filters, column filter data now displayed in tooltip instead of header, row deletion by block, file header changed for rqt msg files, changed some methods to static, Message object now accepts ros Log messages directly.
* time function added, some commenting and reformatting
* added a mutex and buffered inserting to fix responsiveness problem, fixed time formatting and representation issues
* Reorganization of MainWindow widget code into main_window_widget module
* fixed rosservice not contactable error, logger level refresh button now clears items properly, module Reorganization, File load issue fixed
* Header files properly display filters again, initial message # display changed
* fixed matplot when using arrays, merged some code from matplot and plot
* filter editors now populate with current filter, time filters now handle Msecs, folded filter code into Proxy_model.
* Merge branch 'master' of https://kforge.ros.org/visualization/ros_gui
* reenable matplot, added check for matplotlib version, fixed window title of matplot
* implemented QSortFilterProxyModel sorting
* Reformat of time display to include Msecs, config dialog now modal, # of messages now displayed on a label instead of statustip,  fixes large speed issue by breaking "sort on insert", Next push will contain a fix for this break without the speed hit, replaced QDebugs with QWarnings
* modified print/qDebug/qWarning outputs to be more consistent
* load/save/pause visual overhaul
* use different settings files for qt_gui and rqt_gui
* removed debug code
* Fixed filesave format issue, boolean logic paren matching issue, combodialog is now multiselect
* updated review status
* matplot plugin: disabled as the current matplotlib packages doe not support API version 2 which is used in ROS GUI
* matplot plugin: added exception when using non-pyqt bindings
* matplot plugin: work around dateutil bug
* fixed slot name
* fixed use of None object in console plugin
* moved update of publisher plugin comboboxes into worker thread
* fixed bug in matplot plugin
* Merge branch 'master' of https://kforge.ros.org/visualization/ros_gui
* Message save/load functionality added, custom comboboxinputdialog, misc small fixes
* removed quit action from rviz menu bar and made menu bar non-native (`#5484 <https://github.com/ros-visualization/rqt/issues/5484>`_)
* created rightclick menu and functionality for easily excluding/including currently selected items, increased speed of boolean filtering code
* fix unit test failing mock.list not iterable
* enhancement ticket Ticket `#5469 <https://github.com/ros-visualization/rqt/issues/5469>`_: enable 'make test' target for rqt stack and packages
* rqt_console: Addressed some speed issues in message received callback. Removed various print statements. Re-enabled sorting on columns
* Added boolean not (^) to boolean filters, AND and OR changed to & and | for ease of viewing
* rt_console added saving filters on close, status tooltip of filtered/total messages, reenabled sorting, redo of timedialog for easier use, reworked the boolean text filtering to make it easier to change the characters used.
* code style only of rqt_console
* Fixed ui file naming issue
* Added initial version of rqt_console
* removed specific Qt version CMake < 2.8.5 can only not handle full versions (including patch) and the exact required version is not obvious
* code formatting according to pep8
* more updates to API doc
* fixed overriding Python bindings in rqt app
* refactored rest of rqt plugins to inherit from rqt_gui_py Plugin instead of QWidget
* modified some rqt plugins to inherit from rqt_gui_py Plugin
* fixed rqt_gui_py Plugin
* code formatting according to pep8
* code formatting according to pep8
* added explicit rqt plugin class (for API doc only)
* updated API doc
* modified added publishers to not be enabled by default
* changed some labels
* options moved to second ui row
* colorizing stacks as a checkbox and implemented in plugin
* show packages even if their stack could not be determined (dry vs wet)
* modified detection of main filename to work with package-relative imports in subprocesses
* more documentation and better error msg
* bugfix carry arg over in recursion
* treating edges with same labels as siblings as a parameter
* fixed unit tests after code moved
* added .gitignore files
* explicitly name public/supported API
* make rviz plugin more robust
* Fix subwindow title
* fixed matplot imports
* replaced argument names for save/restore settings of a plugin
* added missing const in cpp classes, reformated methods in cpp::PluginContext to camel case
* modified tag name in qtgui plugin manifest
* modified semantic of plugin manifest, renamed file names according to PEP 8, refactored relative imports according to PEP 328
* removed comment from description (which goes into wiki)
* major renaming and refactoring of all packages
* renamed packages and moved into separate stacks (refactoring not yet completed)
* Contributors: Aaron Blasdel, Dirk Thomas, Dorian Scholz, Thibault Kruse
