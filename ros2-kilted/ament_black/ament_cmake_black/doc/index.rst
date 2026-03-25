ament_cmake_black
========================

Checks the code style of Python source files using `black
<http://https://github.com/psf/black>`_.
Files with the following extension is being considered:
``.py``.


How to run the check from the command line?
-------------------------------------------

The command line tool is provided by the package `ament_black
<https://github.com/botsandus/ament_black>`_.


How to run the check from within a CMake ament package as part of the tests?
----------------------------------------------------------------------------

``package.xml``:

.. code:: xml

    <buildtool_depend>ament_cmake</buildtool_depend>
    <test_depend>ament_cmake_black</test_depend>

``CMakeLists.txt``:

.. code:: cmake

    find_package(ament_cmake REQUIRED)
    if(BUILD_TESTING)
      find_package(ament_cmake_black REQUIRED)
      ament_black()
    endif()

When running multiple linters as part of the CMake tests the documentation of
the package `ament_lint_auto <https://github.com/ament/ament_lint>`_ might
contain some useful information.

The documentation of the package `ament_cmake_test
<https://github.com/ament/ament_cmake>`_ provides more information on testing
in CMake ament packages.
