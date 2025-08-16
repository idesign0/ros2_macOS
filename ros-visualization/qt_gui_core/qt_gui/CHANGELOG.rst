^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package qt_gui
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2.2.4 (2025-03-25)
------------------

2.2.3 (2024-01-24)
------------------
* (qt_gui) extended theme logic to get icons (`#280 <https://github.com/ros-visualization/qt_gui_core/issues/280>`_)
* Contributors: Matthijs van der Burgh

2.2.2 (2022-08-15)
------------------

2.2.1 (2022-05-10)
------------------
* Fix flake8 errors introduced by the previous commit. (`#262 <https://github.com/ros-visualization/qt_gui_core/issues/262>`_)
* Enable basic help information if no plugins are running (`#261 <https://github.com/ros-visualization/qt_gui_core/issues/261>`_)
* Contributors: Chris Lalancette, Michael Jeronimo

2.2.0 (2022-03-01)
------------------

2.1.1 (2022-01-14)
------------------

2.1.0 (2021-11-02)
------------------

2.0.1 (2021-04-29)
------------------
* Always prefer 'Tango' icon theme (`#250 <https://github.com/ros-visualization/qt_gui_core/issues/250>`_)
* Contributors: Scott K Logan

2.0.0 (2021-01-26)
------------------
* Fix 'dict_keys' object not subscriptable (`#243 <https://github.com/ros-visualization/qt_gui_core/issues/243>`_)
* Contributors: Michael Jeronimo

1.1.2 (2020-09-18)
------------------
* allow hide title in standalone (`#235 <https://github.com/ros-visualization/qt_gui_core/issues/235>`_)

1.1.1 (2020-08-03)
------------------
* add logic to load qt_gui_icons on windows and macOS (`#222 <https://github.com/ros-visualization/qt_gui_core/issues/222>`_)
* fix exporting perspective for Python 3.6 (`#228 <https://github.com/ros-visualization/qt_gui_core/issues/228>`_)
* remove tango-icon-theme dependency (`#224 <https://github.com/ros-visualization/qt_gui_core/issues/224>`_)

1.1.0 (2020-05-27)
------------------

1.0.9 (2020-05-26)
------------------
* fix export perspective on Python 3 (`#217 <https://github.com/ros-visualization/qt_gui_core/issues/217>`_)
* fix runtime error on shutdown (`#213 <https://github.com/ros-visualization/qt_gui_core/issues/213>`_)

1.0.8 (2020-05-05)
------------------
* qt_gui: use native default icon theme if suitable (`#202 <https://github.com/ros-visualization/qt_gui_core/issues/202>`_)
* fix title bar buttons rendering on macOS (`#205 <https://github.com/ros-visualization/qt_gui_core/issues/205>`_)
* fix platform check for macOS in themeSearchPaths patching (`#204 <https://github.com/ros-visualization/qt_gui_core/issues/204>`_)
* auto adjust main window if standalone (`#199 <https://github.com/ros-visualization/qt_gui_core/issues/199>`_)

1.0.7 (2019-09-30)
------------------
* fix C++ Settings methods allKeys, childGroups, childKeys (`#182 <https://github.com/ros-visualization/qt_gui_core/issues/182>`_)

1.0.6 (2019-06-10)
------------------
* add missing QtCore import (`#178 <https://github.com/ros-visualization/qt_gui_core/issues/178>`_)

1.0.4 (2019-02-08)
------------------
* fix dictionary changes size during iteration (`#166 <https://github.com/ros-visualization/qt_gui_core/issues/166>`_)
* fix typo (`#164 <https://github.com/ros-visualization/qt_gui_core/issues/164>`_)
* remove obsolete maintainer (`#159 <https://github.com/ros-visualization/qt_gui_core/issues/159>`_)

1.0.3 (2018-12-11)
------------------

1.0.2 (2018-12-11)
------------------

1.0.1 (2018-12-11)
------------------

1.0.0 (2018-12-10)
------------------
* fix warning message (`#143 <https://github.com/ros-visualization/qt_gui_core/issues/143>`_)
* fix debug entry point of doc widget title bar (`#141 <https://github.com/ros-visualization/qt_gui_core/issues/141>`_)
* use ament_lint_auto (`#136 <https://github.com/ros-visualization/qt_gui_core/issues/136>`_)
* update tests (`#133 <https://github.com/ros-visualization/qt_gui_core/issues/133>`_)
* port to ROS 2 (`#135 <https://github.com/ros-visualization/qt_gui_core/issues/135>`_)
  Porting to ROS2
* autopep8 (`#123 <https://github.com/ros-visualization/qt_gui_core/issues/123>`_)

0.3.11 (2018-08-29)
-------------------
* fix path being set for exporting perspective (`#121 <https://github.com/ros-visualization/qt_gui_core/issues/121>`_, regression from 0.3.9)

0.3.10 (2018-08-05)
-------------------
* keep multi_process variable in standalone mode (`#120 <https://github.com/ros-visualization/qt_gui_core/issues/120>`_)

0.3.9 (2018-08-03)
------------------
* load and save perspective file path and name adjustments (`#118 <https://github.com/ros-visualization/qt_gui_core/issues/118>`_)
* add hint to run with --force-discover, when no plugin found (`#119 <https://github.com/ros-visualization/qt_gui_core/issues/119>`_)
* remove --multi-process command line argument (`#116 <https://github.com/ros-visualization/qt_gui_core/issues/116>`_)
* avoid crash when a plugin in .perspective is not available (`#110 <https://github.com/ros-visualization/qt_gui_core/issues/110>`_)

0.3.8 (2017-11-03)
------------------

0.3.7 (2017-10-25)
------------------
* Hide the menu bar when using lock perspective (`#103 <https://github.com/ros-visualization/qt_gui_core/issues/103>`_)
* Fix for Python 3 compatibility (`#104 <https://github.com/ros-visualization/qt_gui_core/issues/104>`_)

0.3.6 (2017-08-03)
------------------

0.3.5 (2017-07-27)
------------------
* remove trailing spaces from exported perspective files (`#92 <https://github.com/ros-visualization/qt_gui_core/issues/92>`_)
* fix perspective export with Python3 (`#91 <https://github.com/ros-visualization/qt_gui_core/pull/91>`_)
* hide the remaining title bar elements not disabled by -l and -f (`#90 <https://github.com/ros-visualization/qt_gui_core/issues/90>`_)
* use file name for rqt window title when loading a .perspective (`#84 <https://github.com/ros-visualization/qt_gui_core/pull/84>`_)

0.3.4 (2017-01-24)
------------------
* use Python 3 compatible syntax (`#81 <https://github.com/ros-visualization/qt_gui_core/pull/81>`_)
* fix leftover dock widgets when using --command-switch-perspective (`#80 <https://github.com/ros-visualization/qt_gui_core/pull/80>`_)
* make finding new parent logic more robust (`#76 <https://github.com/ros-visualization/qt_gui_core/pull/76>`_)

0.3.3 (2016-09-19)
------------------
* remove attribute AA_X11InitThreads which is obsolete in Qt 5

0.3.2 (2016-04-21)
------------------
* only restore state on toolbars which have an object name (`#65 <https://github.com/ros-visualization/qt_gui_core/pull/65>`_)

0.3.1 (2016-04-18)
------------------
* workaround bug (QTBUG-52582) in QMenu with Qt 5 (`ros-visualization/python_qt_binding#33 <https://github.com/ros-visualization/python_qt_binding/issues/33>`_)

0.3.0 (2016-04-01)
------------------
* switch to Qt5 (`#64 <https://github.com/ros-visualization/qt_gui_core/pull/64>`_)

0.2.30 (2016-03-30)
-------------------
* add X11 threading for ssh display (`#62 <https://github.com/ros-visualization/qt_gui_core/pull/62>`_)
* allow renaming dock widgets (`#63 <https://github.com/ros-visualization/qt_gui_core/pull/63>`_)

0.2.29 (2015-09-19)
-------------------
* use icon of standalone plugin for app (`#54 <https://github.com/ros-visualization/qt_gui_core/pull/54>`_)

0.2.28 (2015-06-08)
-------------------

0.2.27 (2015-04-29)
-------------------

0.2.26 (2014-08-18)
-------------------
* prevent floating of plugins via double-click when -f flag is set (`#48 <https://github.com/ros-visualization/qt_gui_core/issues/48>`_)

0.2.25 (2014-07-10)
-------------------

0.2.24 (2014-05-21)
-------------------

0.2.23 (2014-05-07)
-------------------
* fix ToolBarArea type with PySide

0.2.22 (2014-03-04)
-------------------
* add shutdown notification for plugin providers (`#39 <https://github.com/ros-visualization/qt_gui_core/issues/39>`_)

0.2.21 (2014-02-12)
-------------------
* add keyboard shortcuts for static menu entries

0.2.20 (2014-01-19)
-------------------

0.2.19 (2014-01-08)
-------------------
* added prefix for the plugin managers settings to allow for multiple caches
* support minimize for containers (`#30 <https://github.com/ros-visualization/qt_gui_core/issues/30>`_)
* fix stacktrace when closing container via 'x' in title bar (`#32 <https://github.com/ros-visualization/qt_gui_core/issues/32>`_)
* fix toolbar area type conversion for pyside
* update icon for container

0.2.18 (2013-10-09)
-------------------
* improve startup time (`#28 <https://github.com/ros-visualization/qt_gui_core/issues/28>`_)
* rename rqt window title
* exit application when standalone plugin fails to load

0.2.17 (2013-08-21)
-------------------
* add PluginLoadError for know errors to avoid printing stacktraces (`ros-visualization/rqt#85 <https://github.com/ros-visualization/rqt/issues/85>`_)
* inherit icons from plugin menu for dock widgets
* fix several OS X related rendering issues, mostly icons and bring the window to front on startup (`ros-visualization/rqt#83 <https://github.com/ros-visualization/rqt/issues/83>`_)
* fix about dialog to not show application icon

0.2.16 (2013-06-06)
-------------------
* make plugin resources relative to plugin.xml (instead of package.xml) (`#16 <https://github.com/ros-visualization/qt_gui_core/issues/16>`_)
* add feature to minimize dock widgets (`#13 <https://github.com/ros-visualization/qt_gui_core/issues/13>`_)
* add feature that each each dock widget can show its own window icon (`#19 <https://github.com/ros-visualization/qt_gui_core/issues/19>`_)
* add option '--perspective-file' to load exported perspective from a file via cli (`#18 <https://github.com/ros-visualization/qt_gui_core/issues/18>`_)
* add option '-f' to freeze layout of dock widgets (`#21 <https://github.com/ros-visualization/qt_gui_core/issues/21>`_)
* restrict reparenting to specific main windows, prevent reparenting into arbitrary main windows (`#14 <https://github.com/ros-visualization/qt_gui_core/issues/14>`_)
* fix help provider
* fix container being closable even when perspective is locked (`#20 <https://github.com/ros-visualization/qt_gui_core/issues/20>`_)
* fix search path of theme icons for OS X (`#17 <https://github.com/ros-visualization/qt_gui_core/issues/17>`_)

0.2.15 (2013-04-02)
-------------------
* revert changes to help_provider from 0.2.13

0.2.14 (2013-03-28 22:42)
-------------------------

0.2.13 (2013-03-28 18:08)
-------------------------
* modify help_provider
* fix menu bar visibility on OS X

0.2.12 (2013-01-17)
-------------------
* fix when dbus is available but no session bus (`#9 <https://github.com/ros-visualization/qt_gui_core/issues/9>`_)

0.2.11 (2013-01-13)
-------------------

0.2.10 (2013-01-11)
-------------------
* add option -t option to keep windows always on top
* enable plugins to provide their arguments for the command line

0.2.9 (2012-12-21)
------------------
* first public release for Groovy
