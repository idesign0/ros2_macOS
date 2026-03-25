.. _usage_config_plugin:

YAML Configuration Plugin
#########################

Source code on :source-master:`GitHub <cx_plugins/config_plugin>`.

.. admonition:: Plugin Class

  cx::ConfigPlugin

This plugin provides the ability to parse YAML files into CLIPS facts.

Configuration
*************

This plugin has no specific configuration.

Features
********

This plugin adds deftemplates and custom functions as listed below.

Facts
~~~~~

.. code-block:: lisp

  ; Asserted by the config-load function for each configuration value in the parsed yaml file.
  (deftemplate confval
    (slot path (type STRING))
    (slot type (type SYMBOL) (allowed-values FLOAT UINT INT BOOL STRING))
    (slot value)
    (slot is-list (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
    (multislot list-value)
  )

Functions
~~~~~~~~~

.. code-block:: lisp

  ; Load all config values from a file (absolute path) given a prefix and store them into (confval) facts.
  ; The prefix can be used to restrict the loaded values to a prefix
  (config-load ?file ?prefix)

Usage Example
*************

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/config.yaml

It loads the used environment manager configuration file into CLIPS and prints the corresponding facts.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/config.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_config"]
      cx_config:
        plugins: ["ament_index", "config", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      ament_index:
        plugin: "cx::AmentIndexPlugin"
      config:
        plugin: "cx::ConfigPlugin"
      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/config.clp"]

Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/config.clp`.

.. code-block:: lisp

  (defrule load-bringup-config
    (not (confval))
    =>
    (bind ?share-dir (ament-index-get-package-share-directory "cx_bringup"))
    (bind ?file (str-cat ?share-dir "/params/plugin_examples/config.yaml"))
    (printout green "Loading yaml file: " ?file crlf)
    (config-load ?file "/")
    (delayed-do-for-all-facts ((?cv confval))
      (ppfact ?cv blue)
    )
  )
