ament_pep257
============

Performs a static analysis to check compliance with Python docstring conventions
of Python source files using `pydocstyle <http://pydocstyle.readthedocs.org/>`_ (formerly pep257).
Files with the following extensions are being considered: ``.py``.


How to run the check from the command line?
-------------------------------------------

.. code:: sh

    ament_pep257 [<path> ...]


How to run the check from within a CMake ament package as part of the tests?
----------------------------------------------------------------------------

The CMake integration is provided by the package `ament_cmake_pep257
<https://github.com/ament/ament_lint>`_.
