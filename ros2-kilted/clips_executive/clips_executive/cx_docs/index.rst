.. _main:

ROS 2 Clips-Executive
=====================

The |CX| offers an integration for CLIPS with the ROS ecosystem for developing knowledge-based applications.

CLIPS
+++++

`CLIPS`_ (C Language Integrated Production System) is a rule-based programming language designed for building expert systems. It was developed in the 1980s by NASA to provide a portable, efficient, and easy-to-use environment for developing knowledge-based applications.
CLIPS uses a production rule system, where knowledge is represented as if-then rules. These rules operate on facts, which represent data or conditions in the system. The inference engine (based on the `Rete algorithm`_) matches rules against facts and executes the applicable rules based on a conflict resolution strategy.
Once no rules can be activated anymore, the execution is halted.

The documentation assumes a basic understanding of CLIPS. If you have never worked with CLIPS before, check out the documentation at `clipsrules.net <https://clipsrules.net/>`_ to get familiar with its functionalities.

The :docsite:`clips_vendor` package provides a packaged version of CLIPS suitable for the ROS ecosystem (and is also used by the CLIPS-Executive).

Overview
++++++++

The heart of the |CX| is the CLIPS Environment Manager node of the :docsite:`cx_clips_env_manager` package that allows creating and destroying CLIPS environments and extending these environments through dynamically loadable plugins (e.g., to inject C++ functions).

Several plugins are provided out-of-the-box that enable interfacing with ROS as well as other useful features. See the :ref:`plugins` page for more information.

Package Index
-------------

The **cx_bringup** package provides example configurations of the CLIPS-Executive that serve as a good starting point for familiarization with the system.

To further customize CLIPS, refer to the **cx_clips_env_manager** documentation for more details on plugin handling. You may also review **cx_example_plugin** as a template for writing custom plugins.



Related Projects
----------------

This project is inspired by the original CLIPS-Executive from the `Fawkes robotics framework <https://ojs.aaai.org/index.php/ICAPS/article/view/3544/3412>`_ (`GitHub <https://github.com/fawkesrobotics/fawkes>`_), which is no longer actively being developed and will be archived soon (once a full transition to ROS is made).

However, while the original CLIPS-Executive presents itself as a goal reasoning framework, this project focuses on generalized CLIPS usage foremost.

It is mostly rewritten from scratch, including a different design philosophy, direct CLIPS 6.4 support via the clips_vendor provider (instead of relying on `clipsmm`_), as well as a more robust CLIPS extension management via plugins.

All new code is released under Apache 2.0 license, while some optional modules that were derived from fawkes are licensed under the GPLv2+ license.

.. toctree::
   :hidden:

   getting_started/index.rst
   plugins/index.rst
   plugin_generator/index.rst
   tutorials/index.rst
   extensions/index.rst

   standards.rst
   __readme_include.rst
