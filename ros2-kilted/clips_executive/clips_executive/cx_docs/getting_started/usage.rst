Usage
#####
This document assumes you have the |CX| installed and sourced in your environment.

ROS Node
++++++++

The |CX| mainly provides a ROS `lifecycle`_ node (:docsite:`Package <cx_clips_env_manager>`, :source-master:`Github <cx_clips_env_manager>`) that manages a CLIPS instance and has the following responsibilities:

 * Dynamically create and destroy CLIPS environments.
 * Log the CLIPS output to ROS and to files.
 * Extend CLIPS environments by plugins (using `pluginlib`_) that allow to externally customize CLIPS environments further via the CLIPS C++ API  (e.g., to inject C++ functions to CLIPS environments).

It provides a clear execution flow guided through it's lifecycle states:

 * On being configured, it parses all parameters to determine the environments and plugins to load. It initializes the environments and plugins.
 * On being activated, it first loads all requested plugins, before calling **(reset)**  in the environments, refreshing all agendas and calling **(run)** to start rule execution.
 * On being deactivated, it first asserts a ``(executive-finalize)`` fact before refreshing all agendas and calling **(run)** one more time to allow cleanup routines to execute before shutdown.

Additionally, the node also supports the usage of :rosdoc:`bond` for advanced system health monitoring (e.g., via the :rosdoc:`nav2_lifecycle_manager`).


CLIPS Environments
++++++++++++++++++

Each CLIPS environment created and managed by the |CX| is a **standard, isolated CLIPS environment**.
The environments are only extended in a minimal and well-defined way to support integration
with ROS and system-level control:

* **Logging router**
  A custom CLIPS output router is registered to forward CLIPS output to ROS logging facilities
  and, optionally, to log file (see :doc:`logging` for more details).

* **Initial watch configuration**
  The default CLIPS watch settings are disabled on creation and selectively re-enabled based
  on ROS parameters (see :doc:`configuration` for more details).

* **System shutdown hook**
  A CLIPS function ``(cx-shutdown)`` is injected into each environment.
  When called, it requests a clean ROS shutdown, enabling CLIPS-based executive logic to
  explicitly terminate the system in a controlled manner.

Aside from these extensions, each environment behaves exactly like a normal CLIPS environment.
No additional functions, facts, or rules are introduced implicitly, and all higher-level behavior
is provided through explicitly loaded plugins.

Starting the |CX|
+++++++++++++++++

To just launch a standalone node, just run the following command:

.. code-block:: bash

  ros2 run cx_clips_env_manager cx_node

A more practical way to to start the node is to utilize the launch file of the `cx_bringup`, which supports convenient configuration, namespacing and composition:

.. code-block:: bash

  ros2 launch cx_bringup cx_launch.py

.. hint::

  Use the `--show-args` option to learn about all parameters that the launch file provides.

Example
+++++++

The following example showcases simple interactions between CLIPS and ROS via a topic:


1. Run the `ros_msgs` demo:

.. code-block:: bash

   ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ros_msgs.yaml

2. Publish a String on the `/ros_cx_in` topic:

.. code-block:: bash

   ros2 topic pub /ros_cx_in std_msgs/msg/String "{data: 'Hello'}"

3. Listen on the `/ros_cx_out` topic for responses:

.. code-block:: bash

   ros2 topic echo /ros_cx_out

You should receive a `"Hello World!"` message for each message received in the CLIPS environment.

Check out the :ref:`Tutorials` page to learn how to write your own CLIPS applications.
