ament_clang_format
==================

Checks the code style of Python source files using `black
<http://https://github.com/psf/black>`_.
Files with the following extension is being considered:
``.py``.


How to run the check from the command line?
-------------------------------------------

.. code:: sh

    ament_black [<path> ...]

When using the option ``--reformat`` the proposed changes are applied in place.


How to run the check from within a CMake ament package as part of the tests?
----------------------------------------------------------------------------

The CMake integration is provided by the package `ament_cmake_black
<https://github.com/botsandus/ament_black/ament_cmake_black>`_.
