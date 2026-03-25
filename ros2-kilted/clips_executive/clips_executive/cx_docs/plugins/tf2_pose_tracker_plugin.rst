.. _usage_tf2_pose_tracker_plugin:

TF2 Pose Tracker Plugin
#######################

Source code on :source-master:`GitHub <cx_plugins/tf2_pose_tracker-plugin>`.

.. admonition:: Plugin Class

  cx::Tf2PoseTrackerPlugin

This plugin provides the ability to perform periodic lookups on the transform tree of a ROS system, updating the fact-base accordingly.

Configuration
*************

:`spin_thread`:

  ============= =======
  Type          Default
  ------------- -------
  bool          true
  ============= =======

  Description
    When set to true, this creates a new node that handles the transform listener, otherwise the main CLIPS manager node handles the subscription in a new callback group.


Features
********

Facts
~~~~~

.. code-block:: lisp

    ; Asserted and modified by the periodic lookup started with tf2-start-periodic-lookup.
    (deftemplate tf2-tracked-pose
      (slot parent (type STRING))
      (slot child (type STRING))
      (slot stamp (type FLOAT))
      (multislot translation (type FLOAT) (cardinality 3 3))
      (multislot rotation (type FLOAT) (cardinality 4 4))
      (slot timer (type EXTERNAL-ADDRESS))
    )

Functions
~~~~~~~~~

.. code-block:: lisp

    ; Start a ROS timer to periodically lookup the pose of ?child-frame relative to ?parent-frame.
    ; Returns TRUE if timer was created successfully, FALSE otherwise.
    (tf2-start-periodic-lookup ?parent-frame ?child-frame ?frequency-hz)   ; example-args: "map" "base_link" 2.0
    ; Stop a started ROS timer using the timer of the associated tf2-tracked-pose fact.
    (tf2-stop-periodic-lookup ?timer-ptr) ; example args: <Pointer-C-0x7f1550001d20>

Usage Example
*************

This plugin is discussed in more depth in the :docsite:`Writing a Plugin for TF Monitoring Tutorial <plugins/writing_a_plugin.rst>`

A minimal working example is provided by the :docsite:`cx_tutorial_agents` package.
It requires the :rosdoc:`turtlesim` package, which provides a minimal simulation environment to interact with and the :rosdoc:`turtle_tf2_py` package to obtain transforms from the simulation.

Open a terminal and start the tf2 turtlesim demo:

.. code-block:: bash

    ros2 launch turtle_tf2_py turtle_tf2_demo.launch.py

In a second terminal run the example setup for the plugin:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=tf2_tracked_pose.yaml package:=cx_tutorial_agents

It will track the pose of turtle1 with a frequency of 0.2 hz and stop the tracking after 5 updates.

Optionally, open a third terminal to control the turtle:

.. code-block:: bash

    ros2 run turtlesim turtle_teleop_key

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_tutorial_agents/params/tf2_tracked_pose.yaml`.

.. code-block:: yaml

    /**:
      ros__parameters:
        autostart_node: true
        environments: ["tf2_pose_tracker_turtlebot"]

        tf2_pose_tracker_turtlebot:
          plugins: ["executive", "tf2_pose_tracker", "files"]
          log_clips_to_file: true
          watch: ["facts", "rules"]

        executive:
          plugin: "cx::ExecutivePlugin"

        tf2_pose_tracker:
          plugin: "cx::Tf2PoseTrackerPlugin"

        files:
          plugin: "cx::FileLoadPlugin"
          pkg_share_dirs: ["cx_tutorial_agents"]
          load: ["clips/tf2_tracked_pose.clp"]


Code
~~~~

File :source-master:`cx_tutorial_agents/clips/tf2_tracked_pose.clp`.

.. code-block:: lisp

    (deftemplate counter (slot iteration (type INTEGER)))


    (defrule tf2-tracked-pose-open
      =>
      (tf2-start-periodic-lookup "world" "turtle1" 0.2)
      (assert (counter (iteration 0)))
    )

    (defrule tf2-tracked-pose-print
      (tf2-tracked-pose (translation $?trans) (rotation $?rot))
      =>
      (printout green "pos at " ?trans " with rot " ?rot crlf)
      (do-for-fact ((?c counter)) TRUE (modify ?c (iteration (+ 1 ?c:iteration))))
    )

    (defrule tf2-tracked-pose-stop
      ?pose <- (tf2-tracked-pose (timer ?t))
      ?count <- (counter (iteration 5))
      =>
      (printout blue "Stopping timer " ?t crlf)
      (tf2-stop-periodic-lookup ?t)
      (retract ?pose ?count)
    )
