.. _usage_ros_param_plugin:

ROS Param Plugin
################

Source code on :source-master:`GitHub <cx_plugins/ros_param_plugin>`.

.. admonition:: Plugin Class

  cx::RosParamPlugin

This plugin provides a function to retrieve ROS parameters from the node that manages the CLIPS environments.

In order to retrieve params from other nodes, the :ref:`RosMsgsPlugin <usage_ros_msgs_plugin>` may be used to call the respective services directly.


Configuration
*************

This plugin has no specific configuration options.

Features
********

Functions
~~~~~~~~~

This plugin adds a custom functions as listed below.

.. code-block:: lisp

  (?bind ?val (ros-param-get-value ?param-name ?default-value))
  ; example args: "environments" (create$ not-found)
  ; example ret:  ("cx_ros_param")

Usage Example
*************

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ros_param.yaml

It calls all of the provided function to print some parameters.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/ros_param.yaml`.

.. code-block:: yaml

  /**:
    ros__parameters:
      unused_param: "i am not used anywhere"
      environments: ["cx_ros_param"]
      cx_ros_param:
        plugins: ["ros_param", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      ros_param:
        plugin: "cx::RosParamPlugin"
      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        batch: [
          "clips/plugin_examples/ros-param.clp"]


Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/ros-param.clp`.

.. code-block:: lisp

  (printout info "(ros-param-get-value \"environments\" (create$ not-found))" crlf)
  (printout green "    " (ros-param-get-value "environments" (create$ not-found)) crlf)

  (printout info "(ros-param-get-value \"cx_ros_param.log_clips_to_file\" FALSE)" crlf)
  (printout green "    " (ros-param-get-value "cx_ros_param.log_clips_to_file" FALSE) crlf)

  (printout info "(ros-param-get-value \"bond_heartbeat_period\" 5.5)" crlf)
  (printout green "    " (ros-param-get-value "bond_heartbeat_period" 5.5) crlf)
