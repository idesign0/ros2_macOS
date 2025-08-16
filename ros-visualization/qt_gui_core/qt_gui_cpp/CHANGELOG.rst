^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package qt_gui_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
* Install headers to include${PROJECT_NAME} (`#259 <https://github.com/ros-visualization/qt_gui_core/issues/259>`_)
* Contributors: Shane Loretz

2.1.1 (2022-01-14)
------------------
* Export targets instead of old-style CMake variables (`#257 <https://github.com/ros-visualization/qt_gui_core/issues/257>`_)
* Contributors: Shane Loretz

2.1.0 (2021-11-02)
------------------
* FindPython3 explicitly instead of FindPythonInterp implicitly (`#254 <https://github.com/ros-visualization/qt_gui_core/issues/254>`_)
* Contributors: Shane Loretz

2.0.1 (2021-04-29)
------------------

2.0.0 (2021-01-26)
------------------
* Fix duplicated QMap to QMultiMap (`#244 <https://github.com/ros-visualization/qt_gui_core/issues/244>`_)
* Switch to using the filesystem implementation in rcpputils. (`#239 <https://github.com/ros-visualization/qt_gui_core/issues/239>`_)
* Contributors: Chris Lalancette, Homalozoa X

1.1.2 (2020-09-18)
------------------

1.1.1 (2020-08-03)
------------------
* avoid a warning about C++ plugins on Windows (`#232 <https://github.com/ros-visualization/qt_gui_core/issues/232>`_)
* qt_gui_cpp_sip: declare private assignment operator for SIP (`#226 <https://github.com/ros-visualization/qt_gui_core/issues/226>`_)

1.1.0 (2020-05-27)
------------------
* Use ament_target_dependencies for qt_gui_cpp. (`#221 <https://github.com/ros-visualization/qt_gui_core/issues/221>`_)
* Contributors: Chris Lalancette

1.0.9 (2020-05-26)
------------------

1.0.8 (2020-05-05)
------------------
* quiet upstream Qt5 warnings (`#210 <https://github.com/ros-visualization/qt_gui_core/issues/210>`_)
* fix project name in log message (`#208 <https://github.com/ros-visualization/qt_gui_core/issues/208>`_)
* fixed namespace in typesystem.xml (`#201 <https://github.com/ros-visualization/qt_gui_core/issues/201>`_)

1.0.7 (2019-09-30)
------------------

1.0.6 (2019-06-10)
------------------

1.0.4 (2019-02-08)
------------------
* support TinyXML2 version 2.2.0 on Xenial (`#169 <https://github.com/ros-visualization/qt_gui_core/issues/169>`_)
* set default C++ standard to 14
* remove obsolete maintainer (`#159 <https://github.com/ros-visualization/qt_gui_core/issues/159>`_)

1.0.3 (2018-12-11)
------------------
* skip qt_gui_cpp on Windows (`#158 <https://github.com/ros-visualization/qt_gui_core/issues/158>`_)
* fix finding Qt5 widgets on Windows (`#157 <https://github.com/ros-visualization/qt_gui_core/issues/157>`_)

1.0.2 (2018-12-11)
------------------
* move build location of qt_gui_cpp_sip library out-of-source (`#156 <https://github.com/ros-visualization/qt_gui_core/issues/156>`_)

1.0.1 (2018-12-11)
------------------

1.0.0 (2018-12-10)
------------------
* port to Windows (`#146 <https://github.com/ros-visualization/qt_gui_core/issues/146>`_)
* check check of return value of loadFile (`#152 <https://github.com/ros-visualization/qt_gui_core/issues/152>`_)
* add required libraries (`#148 <https://github.com/ros-visualization/qt_gui_core/issues/148>`_)
* migrate from tinyxml to tinyxml2 (`#147 <https://github.com/ros-visualization/qt_gui_core/issues/147>`_)
* add test for importing sip generated library (`#150 <https://github.com/ros-visualization/qt_gui_core/issues/150>`_)
* register plugins at the correct location (`#144 <https://github.com/ros-visualization/qt_gui_core/issues/144>`_)
* fix include dir path (`#140 <https://github.com/ros-visualization/qt_gui_core/issues/140>`_)
* use ament_lint_auto (`#136 <https://github.com/ros-visualization/qt_gui_core/issues/136>`_)
* modify qt_gui_cpp to fix build errors in rqt_gui_core (`#137 <https://github.com/ros-visualization/qt_gui_core/issues/137>`_)
* update tests (`#133 <https://github.com/ros-visualization/qt_gui_core/issues/133>`_)
* port to ROS 2 (`#135 <https://github.com/ros-visualization/qt_gui_core/issues/135>`_)
* remove boost shared_ptr references
* remove boost filesystem depends
* autopep8 (`#123 <https://github.com/ros-visualization/qt_gui_core/issues/123>`_)

0.3.11 (2018-08-29)
-------------------

0.3.10 (2018-08-05)
-------------------

0.3.9 (2018-08-03)
------------------
* change included pluginlib header to avoid deprecation warning (`#114 <https://github.com/ros-visualization/qt_gui_core/issues/114>`_)

0.3.8 (2017-11-03)
------------------

0.3.7 (2017-10-25)
------------------

0.3.6 (2017-08-03)
------------------
* add missing run_depend on TinyXML (`#100 <https://github.com/ros-visualization/qt_gui_core/issues/100>`_)
* add TinyXML to target_link_libraries (`#99 <https://github.com/ros-visualization/qt_gui_core/issues/99>`_)

0.3.5 (2017-07-27)
------------------
* find and depend on tinyxml, add missing tinyxml include (`#96 <https://github.com/ros-visualization/qt_gui_core/issues/96>`_, `#97 <https://github.com/ros-visualization/qt_gui_core/issues/97>`_)
* fix relative import for Python3 (`#89 <https://github.com/ros-visualization/qt_gui_core/pull/89>`_)

0.3.4 (2017-01-24)
------------------

0.3.3 (2016-09-19)
------------------

0.3.2 (2016-04-21)
------------------

0.3.1 (2016-04-18)
------------------
* more fixes for shiboken 2
* add missing Qt include directories

0.3.0 (2016-04-01)
------------------
* switch to Qt5 (`#64 <https://github.com/ros-visualization/qt_gui_core/pull/64>`_)

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
* fix finding specific version of PythonLibs with CMake 3 (`#45 <https://github.com/ros-visualization/qt_gui_core/issues/45>`_)

0.2.24 (2014-05-21)
-------------------

0.2.23 (2014-05-07)
-------------------

0.2.22 (2014-03-04)
-------------------
* add shutdown notification for plugin providers (`#39 <https://github.com/ros-visualization/qt_gui_core/issues/39>`_)

0.2.21 (2014-02-12)
-------------------

0.2.20 (2014-01-19)
-------------------
* disable shiboken when version is detected which would segfault (`#35 <https://github.com/ros-visualization/qt_gui_core/issues/35>`_)

0.2.19 (2014-01-08)
-------------------
* use specific python version catkin has decided on
* fix sip bindings when paths contain spaces (`#33 <https://github.com/ros-visualization/qt_gui_core/issues/33>`_)

0.2.18 (2013-10-09)
-------------------
* improve startup time (`#28 <https://github.com/ros-visualization/qt_gui_core/issues/28>`_)
* disabled check for existance of library for cpp plugins
* fix build on OS X with new version of SIP (`#26 <https://github.com/ros-visualization/qt_gui_core/issues/26>`_)

0.2.17 (2013-08-21)
-------------------
* add PluginLoadError for know errors to avoid printing stacktraces (`ros-visualization/rqt#85 <https://github.com/ros-visualization/rqt/issues/85>`_)

0.2.16 (2013-06-06)
-------------------
* make plugin resources relative to plugin.xml (instead of package.xml) (`#16 <https://github.com/ros-visualization/qt_gui_core/issues/16>`_)
* fix help provider

0.2.15 (2013-04-02)
-------------------
* revert changes to help_provider from 0.2.13

0.2.14 (2013-03-28 22:42)
-------------------------

0.2.13 (2013-03-28 18:08)
-------------------------
* work around for broken QGenericReturnArgument constuctor with shiboken, make it build on Ubuntu precise (`ros-visualization/rqt#7 <https://github.com/ros-visualization/rqt/issues/7>`_)
* modify help_provider

0.2.12 (2013-01-17)
-------------------

0.2.11 (2013-01-13)
-------------------

0.2.10 (2013-01-11)
-------------------

0.2.9 (2012-12-21)
------------------
* first public release for Groovy
