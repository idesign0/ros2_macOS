.. _usage_file_load_plugin:

File Load Plugin
################

Source code on :source-master:`GitHub <cx_plugins/file_load_plugin>`.

.. admonition:: Plugin Class

  cx::FileLoadPlugin

This plugin provides the ability to load files into CLIPS using ``batch*`` and ``load*`` through configuration values.

Configuration
*************

:`pkg_share_dirs`:

  ============= =======
  Type          Default
  ------------- -------
  string vector []
  ============= =======

  Description
    When specifying relative paths, look at the share directories of the listed packages to resolve them (in the specified order).

:`load`:

  ============= =======
  Type          Default
  ------------- -------
  string vector []
  ============= =======

  Description
    Specify files to load using CLIPS load* command when the plugin is loaded.
    Supports absolute paths or relative paths using the share directories specified above.

:`batch`:

  ============= =======
  Type          Default
  ------------- -------
  string vector []
  ============= =======

  Description
    Specify files to load using CLIPS batch* command when the plugin is loaded. This happens after the files specified in ``load`` are loaded.
    Supports absolute paths or relative paths using the share directories specified above.

:`cleanup_batch`:

  ============= =======
  Type          Default
  ------------- -------
  string vector []
  ============= =======

  Description
    Specify files to load using CLIPS batch* command when the plugin is unloaded.
    This may be used to clean up all loaded content to gracefully undo whatever was loaded in before.
    Supports absolute paths or relative paths using the share directories specified above.

Features
********

This plugin injects the user-defined code into the CLIPS environments and has no other effects.

For the difference between ``load*`` and ``batch*`` please consult the CLIPS |BPG|.

.. Note::

  It is recommended to use load over batch whenever possible due to the superior error handling of load.
  E.g., when using batch, a missing closing parenthesis ``)``  may lead to a silent failure as CLIPS treats this as an uncompleted command and waits for another closing parenthesis to appear before actually processing the input.

Usage Example
*************

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/file_load.yaml

Note, that while `file-load.clp` is loaded before `file-load-batch.clp`, the rules of `file-load.clp` will only be executed, once the CLIPS engine runs. hence, the expected output is that `batch` is printed before `Hello World`.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/file_load.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_file_load"]
      cx_file_load:
        plugins: ["files"]
        watch: ["facts", "rules"]

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: ["clips/plugin_examples/file-load.clp"]
        batch: ["clips/plugin_examples/file-load-batch.clp"]
        cleanup_batch: ["clips/plugin_examples/file-load-cleanup-batch.clp"]

Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/file-load.clp`.

.. code-block:: lisp

  (defrule hello-world
     (not (hello))
     =>
     (printout green "Hello world" crlf)
     (assert (hello))
  )
  (defrule goodbye-world
     (executive-finalize)
     =>
     (printout blue "Goodbye world" crlf)
  )

File :source-master:`cx_bringup/clips/plugin_examples/file-load-batch.clp`.

.. code-block:: lisp

  (printout yellow "batch" crlf)

File :source-master:`cx_bringup/clips/plugin_examples/file-load-cleanup-batch.clp`.

.. code-block:: lisp

  ; file-load-cleanup-batch.clp

  (printout yellow "cleanup batch" crlf)
  (undefrule hello-world)
  (undefrule goodbye-world)
  (do-for-all-facts ((?h hello))
    (retract ?h)
  )
