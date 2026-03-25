^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_cmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.1.0 (2025-11-10)
------------------
* feat: add autoware_ament_auto_package() macro (`#37 <https://github.com/autowarefoundation/autoware_cmake/issues/37>`_)
* feat: jazzy-porting, add jazzy distro condition for jazzy related compiling (`#35 <https://github.com/autowarefoundation/autoware_cmake/issues/35>`_)
  cmake env::jazzy-porting::add jazzy distro condition for jazzy related compiling
* fix: when using CMake >= 3.24 use CMAKE_COMPILE_WARNING_AS_ERROR variable instead of setting -Werror directly (`#33 <https://github.com/autowarefoundation/autoware_cmake/issues/33>`_)
* Contributors: Silvio Traversaro, Yutaka Kondo, 心刚

1.0.2 (2025-04-08)
------------------
* fix(autoware_package.cmake): workaround to avoid `missing tinyxml2::tinyxml2` (`#24 <https://github.com/autowarefoundation/autoware_cmake/issues/24>`_)
  * add tinyxml2 workaround
  * move
  ---------
* Contributors: Yutaka Kondo

1.0.1 (2025-03-17)
------------------
* fix(autoware_cmake): fix links to issues in CHANGELOG.rst files (`#13 <https://github.com/autowarefoundation/autoware_cmake/issues/13>`_)
* Contributors: Esteve Fernandez

1.0.0 (2024-05-02)
------------------
* Merge pull request `#1 <https://github.com/autowarefoundation/autoware_cmake/issues/1>`_ from youtalk/import-from-autoware-common
  feat: import from autoware_common
* add maintainer
* move to autoware_cmake
* Contributors: Yutaka Kondo
