Continuous Monitoring
#####################

**Goal:** Monitor the pose of a turtle from turtlesim using CLIPS

**Tutorial level:** Beginner

**Time:** 20 minutes

.. contents:: Contents
   :depth: 2
   :local:

Background
----------

CLIPS can be used to continuously monitor and control a robotic system.
This typically involves interleaving runs of the CLIPS inference engine with other components, e.g., to obtain feedback from ROS.

Prerequisites
-------------

This tutorial extends the ``cx_tutorial_agents`` package created in the :doc:`previous tutorial <hello_world>`.
Additionally, it requires the :rosdoc:`turtlesim` package, which provides a minimal simulation environment to interact with.

Continuous Monitoring
---------------------

The objective of this tutorial is to provide a basic monitor for the turtlesim simulator, which teleports the turtle to the center whenever it is approaching the canvas frame.

To achieve this, the CLIPS environment has to get information from a ROS topic publishing the pose.
Whenever the pose is outside of the bounding box, a request to the teleport service is sent to reset the turtle to the center of the frame.

1 Obtaining the Files
^^^^^^^^^^^^^^^^^^^^^

Navigate to the ``params`` directory of the ``cx_tutorial_agents`` package from the :doc:`Hello World tutorial <hello_world>` and download the configuration file using the following command:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/params
   wget -O turtlesim.yaml https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/params/turtlesim.yaml

This adds the file ``turtlesim.yaml``.

Next, two CLIPS files can be downloaded to the ``clips`` directory:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/clips
   wget -O turtlesim_monitor.clp https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/clips/turtlesim_monitor.clp
   wget -O turtlesim_teleport.clp https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/clips/turtlesim_teleport.clp

2 Configuring the |CX|
^^^^^^^^^^^^^^^^^^^^^^

The CLIPS environment needs to:

* Continuously run the inference engine, which can be achieved with the  :docsite:`ExecutivePlugin <clips_executive/plugins/executive_plugin>`.
* Establish ROS communication to other components, which is provided by the :docsite:`RosMsgsPlugin <clips_executive/plugins/ros_msgs_plugin>`.
* Load our CLIPS files in the environment, using the :docsite:`FileLoadPlugin <clips_executive/plugins/executive_plugin>`.

The following configuration is used to achieve this:

.. code-block:: yaml

  /**:
    ros__parameters:
      autostart_node: true
      environments: ["turtlesim_monitor"]

      turtlesim_monitor:
        plugins: ["executive","ros_msgs","files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]


      executive:
        plugin: "cx::ExecutivePlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_tutorial_agents"]
        load: ["clips/turtlesim_monitor.clp",
               "clips/turtlesim_teleport.clp"]


Again, the parameter file starts by configuring the node to automatically activate and then to setup a CLIPS environment, this time named ``turtlesim_monitor``.

.. code-block:: yaml

  /**:
    ros__parameters:
      autostart_node: true
      environments: ["turtlesim_monitor"]

This time, three plugins are needed, which are further specified afterwards.
The plugins are loaded in the same order as they are specified.
This is important as the loaded CLIPS files will contain functions from the other plugins, hence it must be loaded last.

.. code-block:: yaml

      turtlesim_monitor:
        plugins: ["executive","ros_msgs","files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]


The plugins for :docsite:`continuous execution <clips_executive/plugins/executive_plugin>` and :docsite:`ROS communication <clips_executive/plugins/ros_msgs_plugin>` are used as-is whithout further configuration.
The former calls the CLIPS inference engine with a fixed frequency, the latter provides CLIPS functions and templates for interfacing with ROS.

.. code-block:: yaml

      executive:
        plugin: "cx::ExecutivePlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

Lastly, the :docsite:`FileLoadPlugin <clips_executive/plugins/file_load_plugin>` is used to obtain the files.

.. code-block:: yaml

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_tutorial_agents"]
        load: ["clips/turtlesim_monitor.clp",
               "clips/turtlesim_teleport.clp"]


3 Monitoring the Turtle
^^^^^^^^^^^^^^^^^^^^^^^

The ``turtlesim_monitor.clp`` file provides a set of rules to listen to the ``Pose`` topic of ``turtle1`` and to detect if the pose leaves a defined safe area.

.. code:: lisp

  (defglobal
    ?*TURTLE-POSE-TOPIC* = "turtle1/pose"
    ?*TURTLE-POSE-TYPE* = "turtlesim/msg/Pose"
    ?*SAFE-AREA-LOWER-BOUND* = 1.0
    ?*SAFE-AREA-UPPER-BOUND* = 10.0
  )

  (defrule turtle-pose-topic-create-subscription
  " Create subscription to monitor the pose."
  =>
    (unwatch facts ros-msgs-message)
    (unwatch rules turtle-pose-receive)
    (ros-msgs-create-subscription ?*TURTLE-POSE-TOPIC* ?*TURTLE-POSE-TYPE*)
    (printout info "Listening to " ?*TURTLE-POSE-TOPIC* crlf)
  )

  (defrule turtle-pose-receive
  " React to incoming messages andcheck for critical pose. "
    (ros-msgs-subscription (topic ?t&:(eq ?t ?*TURTLE-POSE-TOPIC*)))
    ?msg-f <- (ros-msgs-message (topic ?t) (msg-ptr ?inc-msg))
    =>
    (bind ?x (ros-msgs-get-field ?inc-msg "x"))
    (bind ?y (ros-msgs-get-field ?inc-msg "y"))
    (if (or
      (< ?x ?*SAFE-AREA-LOWER-BOUND*)
      (< ?y ?*SAFE-AREA-LOWER-BOUND*)
      (> ?x ?*SAFE-AREA-UPPER-BOUND*)
      (> ?y ?*SAFE-AREA-UPPER-BOUND*)
    )
    then
      (printout yellow "turtle out of bounds" crlf)
      (assert (turtle-out-of-bounds))
    )
    (ros-msgs-destroy-message ?inc-msg)
    (retract ?msg-f)
  )

  (defrule turtle-pose-destroy-subscription
  " Delete the subscription on executive finalize. "
    (executive-finalize)
    (ros-msgs-subscription (topic ?t&:(eq ?t ?*TURTLE-POSE-TOPIC*)))
  =>
    (printout info "Destroying subscription for " ?*TURTLE-POSE-TOPIC* crlf)
    (ros-msgs-destroy-subscription ?*TURTLE-POSE-TOPIC*)
  )


First, some global variables are defined for convenience:

.. code:: lisp

  (defglobal
    ?*TURTLE-POSE-TOPIC* = "turtle1/pose"
    ?*TURTLE-POSE-TYPE* = "turtlesim/msg/Pose"
    ?*SAFE-AREA-LOWER-BOUND* = 1.0
    ?*SAFE-AREA-UPPER-BOUND* = 10.0
  )

The rest of the file is comprised of individual CLIPS rules, which rely on the features provided by the ``RosMsgsPlugin``. Specifically, here you need to utilize:

* ``ros-msgs-subscription``: A fact template for storing currently open subscriptions. A fact of that template is created via the function ``ros-msgs-create-subscription`` and destroyed via ``ros-msgs-destroy-subscription``.
* ``ros-msgs-message``: A fact template holding messages. Facts of this template are asserted whenever an open subscription receives a message.
* Utility functions ``ros-msgs-get-field`` and ``ros-msgs-destroy-message`` to handle the incoming messages.

Refer to the :docsite:`RosMsgsPlugin documentation <clips_executive/plugins/ros_msgs_plugin>` for more information and a complete list of provided features.


Then, a rule called ``turtle-pose-topic-create-subscription`` is defined, which creates the subsciption to the ``turtle1/pose`` pose using the ROS communication plugin. It has no condition, hence can be executed right away.
Before the subscription is established, it unwatches the ``ros-msgs-message`` template and ``turtle-pose-receive`` rule, which processes the message facts. This greatly reduces the amount of log output as data is published with high frequency on this topic.

.. code:: lisp

  (defrule turtle-pose-topic-create-subscription
  " Create subscription to monitor the pose."
  =>
    (unwatch facts ros-msgs-message)
    (unwatch rules turtle-pose-receive)
    (ros-msgs-create-subscription ?*TURTLE-POSE-TOPIC* ?*TURTLE-POSE-TYPE*)
    (printout info "Listening to " ?*TURTLE-POSE-TOPIC* crlf)
  )


The next rule ``turtle-pose-receive`` processes each incoming message from the pose topic and if the received (x,y) coordinate is outside of the safe area, it asserts a fact ``(turtle-out-of-bounds)``. This will be the trigger for another set of rules to teleport the turtle back to the center of the safe area.

.. code:: lisp

  (defrule turtle-pose-receive
  " React to incoming messages andcheck for critical pose. "
    (ros-msgs-subscription (topic ?t&:(eq ?t ?*TURTLE-POSE-TOPIC*)))
    ?msg-f <- (ros-msgs-message (topic ?t) (msg-ptr ?inc-msg))
    =>
    (bind ?x (ros-msgs-get-field ?inc-msg "x"))
    (bind ?y (ros-msgs-get-field ?inc-msg "y"))
    (if (or
      (< ?x ?*SAFE-AREA-LOWER-BOUND*)
      (< ?y ?*SAFE-AREA-LOWER-BOUND*)
      (> ?x ?*SAFE-AREA-UPPER-BOUND*)
      (> ?y ?*SAFE-AREA-UPPER-BOUND*)
    )
    then
      (printout yellow "turtle out of bounds" crlf)
      (assert (turtle-out-of-bounds))
    )
    (ros-msgs-destroy-message ?inc-msg)
    (retract ?msg-f)
  )

Lastly, the rule ``turtle-pose-destroy-subscription`` closes the subscription when the |CX| stops the environment.

.. code:: lisp

  (defrule turtle-pose-destroy-subscription
  " Delete the subscription on executive finalize. "
    (executive-finalize)
    (ros-msgs-subscription (topic ?t&:(eq ?t ?*TURTLE-POSE-TOPIC*)))
  =>
    (printout info "Destroying subscription for " ?*TURTLE-POSE-TOPIC* crlf)
    (ros-msgs-destroy-subscription ?*TURTLE-POSE-TOPIC*)
  )

4 Teleporting the Turtle
^^^^^^^^^^^^^^^^^^^^^^^^

The file ``turtlesim_teleport.clp`` contains a set of rule that will teleport the turtle back to the center of the canvas, whenever it leaves the safe area, as indicated by the ``(turtle-out-of-bounds)`` fact.

.. code:: lisp

    (defglobal
      ?*TURTLE-SERVICE* = "turtle1/teleport_absolute"
      ?*TURTLE-TELEPORT-TYPE* = "turtlesim/srv/TeleportAbsolute"
    )

    (defrule turtle-teleport-client-init
    " Create publisher for ros_cx_out."
      (not (ros-msgs-client (service ?s&:(eq ?s ?*TURTLE-SERVICE*))))
      (not (executive-finalize))
    =>
      ; create the client
      (ros-msgs-create-client ?*TURTLE-SERVICE* ?*TURTLE-TELEPORT-TYPE*)
      (printout green "Opening client for " ?*TURTLE-SERVICE* crlf)
    )

    (defrule turtle-teleport-request-teleport-mid
    " Attempt to request the service. "
      (ros-msgs-client (service ?service&:(eq ?service ?*TURTLE-SERVICE*)))
      (not (request ?any-id))
      (turtle-out-of-bounds)
      (time ?any-time) ; used to continuously attempt to request the service until success
      =>
      (bind ?new-req (ros-msgs-create-request ?*TURTLE-TELEPORT-TYPE*))
      (ros-msgs-set-field ?new-req "x" 5.5)
      (ros-msgs-set-field ?new-req "y" 5.5)
      (bind ?id (ros-msgs-async-send-request ?new-req ?service))
      (if ?id then
        (printout yellow "Request sent with id " ?id crlf)
        (assert (request ?id))
       else
        (printout red "Sending of request failed, is the service running?" crlf)
        (printout red "Start it using \"ros2 run turtlesim turtlesim_node\"" crlf)
      )
      (ros-msgs-destroy-message ?new-req)
    )

    (defrule turtle-teleport-done
    " Got response, delete it without reading, it is empty."
      ?msg-fact <- (ros-msgs-response (service ?service) (msg-ptr ?ptr) (request-id ?id))
      ?request-fact <- (request ?id)
      ?out-of-bounds-fact <- (turtle-out-of-bounds)
    =>
      (printout yellow "Turtle teleport done (request id " ?id")" crlf)
      (ros-msgs-destroy-message ?ptr)
      (retract ?msg-fact ?request-fact ?out-of-bounds-fact)
    )

    (defrule turtle-teleport-client-finalize
    " Delete the client on executive finalize. "
      (executive-finalize)
      (ros-msgs-client (service ?service))
    =>
      (printout info "Destroying client" crlf)
      (ros-msgs-destroy-client ?service)
    )

Again, the file begins with some global constants for convenience.

.. code:: lisp

    (defglobal
      ?*TURTLE-SERVICE* = "turtle1/teleport_absolute"
      ?*TURTLE-TELEPORT-TYPE* = "turtlesim/srv/TeleportAbsolute"
    )

Similar to handling subscriptions and messages, the ``RosMsgsPlugin`` also supports the creation of ROS service clients to send service requests.
The ``turtle-teleport-client-init`` rule simply ensures a client to the ``turtle1/teleport_absolute`` service is created.

.. code:: lisp

    (defrule turtle-teleport-client-init
    " Create publisher for ros_cx_out."
    =>
      ; create the client
      (ros-msgs-create-client ?*TURTLE-SERVICE* ?*TURTLE-TELEPORT-TYPE*)
      (printout green "Opening client for " ?*TURTLE-SERVICE* crlf)
    )

The ``turtle-teleport-request-teleport-mid`` rule can be activated once the client is established and the turtle is out of bounds.
Additionally, it is conditioned on the current time, a fact updated by the ``ExecutivePlugin`` in between each inference engine run.
This allows the rule to fire at each iteration if needed (e.g., if the service is not reachable).
The rule effect takes care of creating and sending the request.
If the request is successfully sent, it asserts a request fact (including the request ID assigned to it), indicating that the request is now processed asynchronously and preventing the rule to fire again.

.. code:: lisp

    (defrule turtle-teleport-request-teleport-mid
    " Attempt to request the service. "
      (ros-msgs-client (service ?service&:(eq ?service ?*TURTLE-SERVICE*)))
      (turtle-out-of-bounds)
      (not (request ?any-id))
      (time ?any-time) ; used to continuously attempt to request the service until success
      =>
      (bind ?new-req (ros-msgs-create-request ?*TURTLE-TELEPORT-TYPE*))
      (ros-msgs-set-field ?new-req "x" 5.5)
      (ros-msgs-set-field ?new-req "y" 5.5)
      (bind ?id (ros-msgs-async-send-request ?new-req ?service))
      (if ?id then
        (printout yellow "Request sent with id " ?id crlf)
        (assert (request ?id))
       else
        (printout red "Sending of request failed, is the service running?" crlf)
        (printout red "Start it using \"ros2 run turtlesim turtlesim_node\"" crlf)
      )
      (ros-msgs-destroy-message ?new-req)
    )


The rule ``turtle-teleport-done`` processes the response of the service request. The ``TeleportAbsolute`` service does not provide an explicit response, hence the rule simply cleans it up, along with the facts indicating the turtle being out of bounds ``(turtle-out-of-bounds)`` and that the request awaited an answer ``(request ?id)``.

.. code:: lisp

    (defrule turtle-teleport-done
    " Got response, delete it without reading, it is empty."
      ?msg-fact <- (ros-msgs-response (service ?service) (msg-ptr ?ptr) (request-id ?id))
      ?request-fact <- (request ?id)
      ?out-of-bounds-fact <- (turtle-out-of-bounds)
    =>
      (printout yellow "Turtle teleport done (request id " ?id")" crlf)
      (ros-msgs-destroy-message ?ptr)
      (retract ?msg-fact ?request-fact ?out-of-bounds-fact)
    )

Lastly, the rule ``turtle-teleport-client-finalize`` takes care of removing the client on shutdown of the environment.

.. code:: lisp

    (defrule turtle-teleport-client-finalize
    " Delete the client on executive finalize. "
      (executive-finalize)
      (ros-msgs-client (service ?service))
    =>
      (printout info "Destroying client" crlf)
      (ros-msgs-destroy-client ?service)
    )

5 Build and Run
^^^^^^^^^^^^^^^

First, build the package ans source it:


.. code-block:: bash

    cd ~/ros2/cx_tutorial_ws/
    colcon build --symlink-install
    source install/setup.bash

Using the launch file of the ``cx_bringup`` package, you can start the CLIPS environment with the following command:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=turtlesim.yaml package:=cx_tutorial_agents

Run the turtlesim simulation in a different terminal:

.. code-block:: bash

    ros2 run turtlesim turtlesim_node

In a third terminal you can run the turtle_teleop_key node to control the turtle with your keyboard.

.. code-block:: bash

    ros2 run turtlesim turtle_teleop_key


When attempting to hit the canvas border you will see the turtle spawning back to the center. The output from CLIPS will look like this:

.. code-block:: bash

  [<timestamp>] [turtlesim_monitor] [info] FIRE    1 turtle-pose-topic-create-subscription: *
  [<timestamp>] [turtlesim_monitor] [info] ==> f-1     (ros-msgs-subscription (topic "turtle1/pose") (type "turtlesim/msg/Pose"))
  [<timestamp>] [turtlesim_monitor] [info] Listening to turtle1/pose
  [<timestamp>] [turtlesim_monitor] [info] FIRE    2 turtle-teleport-client-init: *
  [<timestamp>] [turtlesim_monitor] [info] ==> f-2     (ros-msgs-client (service "turtle1/teleport_absolute") (type "turtlesim/srv/TeleportAbsolute"))
  [<timestamp>] [turtlesim_monitor] [info] Opening client for turtle1/teleport_absolute
  [<timestamp>] [turtlesim_monitor] [info] turtle out of bounds
  [<timestamp>] [turtlesim_monitor] [info] ==> f-2980  (turtle-out-of-bounds)
  [<timestamp>] [turtlesim_monitor] [info] FIRE    2 turtle-teleport-request-teleport-mid: f-2,*,f-2980,f-2979
  [<timestamp>] [turtlesim_monitor] [info] Request sent with id 1
  [<timestamp>] [turtlesim_monitor] [info] ==> f-2981  (request 1)
  [<timestamp>] [turtlesim_monitor] [info] turtle out of bounds
  [<timestamp>] [turtlesim_monitor] [info] turtle out of bounds
  [<timestamp>] [turtlesim_monitor] [info] turtle out of bounds
  [<timestamp>] [turtlesim_monitor] [info] turtle out of bounds
  [<timestamp>] [turtlesim_monitor] [info] turtle out of bounds
  [<timestamp>] [turtlesim_monitor] [info] ==> f-2983  (ros-msgs-response (service "turtle1/teleport_absolute") (request-id 1) (msg-ptr <Pointer-C-0x7f639c000bb0>))
  [<timestamp>] [turtlesim_monitor] [info] FIRE    6 turtle-teleport-done: f-2983,f-2981,f-2980
  [<timestamp>] [turtlesim_monitor] [info] Turtle teleport done (request id 1)
  [<timestamp>] [turtlesim_monitor] [info] <== f-2983  (ros-msgs-response (service "turtle1/teleport_absolute") (request-id 1) (msg-ptr <Pointer-C-0x7f639c000bb0>))
  [<timestamp>] [turtlesim_monitor] [info] <== f-2981  (request 1)
  [<timestamp>] [turtlesim_monitor] [info] <== f-2980  (turtle-out-of-bounds)


Summary
-------

You leveraged the ``ExecutivePlugin`` to subsequently run the CLIPS inference engine indefinitely. Together with the ``RosMsgsPlugin`` this enabled you to receive the latest updates from ROS topics in CLIPS.
You utilized this to monitor the turtlesim simulator in order to prevent the turtle from hitting the canvas border.

Next Steps
----------

You can check out the other plugins offered by the |CX| :docsite:`here <clips_executive/plugins>`.
Also, you can learn how to :doc:`write your own CLIPS plugin <writing_a_plugin>`.

Related Content
---------------

Providing a service or utilizing ROS actions requires additional work, as general ROS introspection support is currently limited to messages and service clients in the latest ROS LTS version.

Therefore, the |CX| offers the :docsite:`ROS Interface Plugin Generator <clips_executive/plugin_generator>` to bridge this gap.
