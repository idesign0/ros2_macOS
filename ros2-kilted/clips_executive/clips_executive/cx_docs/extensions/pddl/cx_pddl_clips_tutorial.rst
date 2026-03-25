.. _structured_pddl_agent:

Tutorial: cx_pddl_clips Agent
=============================

**Goal:** Use CLIPS to plan and execute PDDL actions with monitoring and timing using the interfaces provided by ``cx_pddl_clips``.

**Tutorial level:** Advanced

**Time:** 45–60 minutes

.. contents:: Contents
   :depth: 2
   :local:


Overview
--------

This tutorial uses the interfaces provided by ``cx_pddl_clips`` to implement PDDL-based planning and action execution.
The presented agent not only requests a plan but also **executes and monitors actions** step by step, checking preconditions, applying effects and tracking execution times.

You will learn how to:

1. Extend fact templates provided by ``cx_pddl_clips``,
2. Plan using the PDDL Manager,
3. Execute actions while checking feasibility conditions,


Prerequisites
-------------

This tutorial assumes that |CX| is installed and that you are familiar with creating and configuring a custom package using it.

Package Layout
--------------

Below the relevant directory layout to configure and run the PDDL CLIPS agent is shown.
The code is part of the ``cx_pddl_bringup`` package.

.. code-block:: text

   cx_pddl_bringup
   ├── CMakeLists.txt
   ├── package.xml
   ├── config
   │   └── cx_pddl_clips_agent.yaml
   ├── clips
   │   └── cx_pddl_bringup
   │       ├── cx-pddl-clips-agent.clp
   │       └── deftemplate-overrides.clp
   └── pddl
       ├── domain.pddl
       └── problem.pddl

Directory Layout
----------------

The package contains several directories with different responsibilities:

* ``config/``
  Contains configuration files used to configure the CLIPS agent and plugins.

* ``clips/``
  Contains the CLIPS rule files implementing the agent logic.

* ``pddl/``
  Contains the PDDL domain and problem definitions.

Configuration
-------------

The file ``config/cx_pddl_clips_agent.yaml`` configures the CLIPS agent,
loads the required plugins and defines which CLIPS files should be loaded.

.. code-block:: yaml

  /**:
    ros__parameters:
      autostart_node: true
      environments: ["cx_pddl_clips_agent"]

      cx_pddl_clips_agent:
        plugins: ["executive", "ros_msgs",
                  "ament_index",
                  "plan_temporal_action",
                  "timed_plan_action_msg",
                  "pddl_files",
                  "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]
        redirect_stdout_to_debug: true

      ament_index:
        plugin: "cx::AmentIndexPlugin"

      executive:
        plugin: "cx::ExecutivePlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      pddl_files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_pddl_clips", "cx_pddl_bringup"]
        batch: [
          "clips/cx_pddl_clips/deftemplates.clp",
          "clips/cx_pddl_bringup/deftemplate-overrides.clp",
          "clips/cx_pddl_clips/pddl-no-deftemplates.clp"
        ]

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_pddl_bringup"]
        load: ["clips/cx_pddl_bringup/cx_pddl_clips_agent.clp"]

      plan_temporal_action:
        plugin: "cx::CXCxPddlInterfacesPlanTemporalPlugin"

      timed_plan_action_msg:
        plugin: "cx::CXCxPddlInterfacesTimedPlanActionPlugin"

The configuration creates a single CLIPS environment, which is augmented by several plugins:

* ``ament_index``
  Used to locate PDDL files from within CLIPS.

* ``executive``
  Provides the core execution loop for the CLIPS agent

* ``ros_msgs``
  Enables ROS interactions.

* ``pddl_files``
  Loads reusable CLIPS templates and rules related to PDDL integration.

* ``files``
  Loads the tutorial-specific CLIPS rule file implementing the agent logic.

* ``plan_temporal_action``
  Enables access to temporal planning via action servers (generic action clients are not covered by the generic ROS communication plugin in ROS Jazzy).

* ``timed_plan_action_msg``
  Needed for the nested definition inside of the temporal plan action.


Defining the PDDL Action Template
---------------------------------

This tutorial agent overrides the default ``pddl-action`` template to include
execution state, planned timing, and runtime tracking.
Further, it extends the ``pddl-plan`` template  by a ``plan-start`` slot to
mark the execution start of the plan.
The extended templates are defined in the file ``cx_pddl_bringup/deftemplate-overrides.clp``.

.. code-block:: lisp

  (deftemplate pddl-action
    (slot instance (type SYMBOL))
    (slot id (type SYMBOL)) ; this should be a globally unique ID
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot plan (type SYMBOL))
    (slot planned-start-time (type FLOAT))
    (slot planned-duration (type FLOAT))
    (slot actual-start-time (type FLOAT))
    (slot actual-duration (type FLOAT))
    (slot state (type SYMBOL) (allowed-values IDLE SELECTED EXECUTING DONE))
  )

  (deftemplate pddl-plan
    (slot instance (type SYMBOL))
    (slot id (type SYMBOL))
    (slot goal (type SYMBOL))
    (slot goal-ptr (type EXTERNAL-ADDRESS))
    (slot plan-type (type SYMBOL) (allowed-values CLASSICAL TEMPORAL) (default CLASSICAL))
    (slot goal-handle (type EXTERNAL-ADDRESS))
    (slot type (type SYMBOL) (allowed-values TEMPORAL CLASSICAL))
    (slot state (type SYMBOL) (allowed-values PENDING WAITING PLANNING REQUEST-CANCELING CANCELING CANCELED SUCCESS FAILURE) (default PENDING))
    (slot plan-start (type FLOAT) (default 0.0))
  )



The template override is loaded after the initial template definition
provided by the PDDL interface, but before any rules depending on the
template are defined. The override retains all original slots and only adds
additional ones required by this tutorial.

This approach ensures that the rule set provided by the interface can still be
loaded without modification, while allowing the tutorial agent to extend the
template with additional execution-related information.

Agent Code Logic
----------------

The file ``clips/cx_pddl_bringup/cx-pddl-clips-agent.clp`` contains the custom logic for the tutorial agent:

.. code-block:: lisp


  (defrule cx-pddl-clips-agent-pddl-init
  =>
    (assert (pddl-manager (node "/pddl_manager")))
  )

  (defrule cx-pddl-clips-agent-pddl-add-instance
  " Setup PDDL instance with an active goal to plan for "
    (pddl-manager (ros-comm-init TRUE))
  =>
    (bind ?share-dir (ament-index-get-package-share-directory "cx_pddl_bringup"))
    (assert
      (pddl-instance
        (name test)
        (domain "domain.pddl")
        (problem "problem.pddl")
        (directory (str-cat ?share-dir "/pddl"))
      )
      (pddl-get-fluents (instance test))
      (pddl-create-goal-instance (instance test) (goal active-goal))
      (pddl-goal-fluent (instance test) (goal active-goal) (name on) (params a b))
      (pddl-goal-fluent (instance test) (goal active-goal) (name on) (params b c))
      (pddl-set-goals (instance test) (goal active-goal))
      (pddl-plan (id test-plan) (instance test) (goal active-goal) (plan-type TEMPORAL))
    )
  )

  (defrule cx-pddl-clips-agent-select-action
  " Start executing the first action of the resulting plan "
    ?plan <- (pddl-plan (id ?plan-id) (plan-start ?p-start))
    (not (pddl-action (state EXECUTING|SELECTED)))
    ?pa <- (pddl-action (plan ?plan-id) (planned-start-time ?t) (state IDLE))
    (not (pddl-action (plan ?plan-id) (state IDLE) (planned-start-time ?ot&:(< ?ot ?t))))
  =>
    (if (= ?p-start 0.0) then (modify ?plan (plan-start (now))))
    (modify ?pa (state SELECTED))
  )

  (defrule cx-pddl-clips-agent-check-action
  " Before executing an action check the condition to make sure it is feasible "
    (pddl-action (id ?id) (state SELECTED) (name ?name) (params $?params))
    (not (pddl-action-condition (action ?id)))
  =>
    (assert (pddl-action-condition (instance test) (action ?id)))
  )

  (defrule cx-pddl-clips-agent-executable-action
  " Condition is satisfied, go ahead with execution "
    (pddl-plan (id ?plan-id) (plan-start ?t))
    (pddl-action-condition (action ?action-id) (state CONDITION-SAT))
    ?pa <- (pddl-action (id ?action-id) (plan ?plan-id) (name ?name) (params $?params) (state SELECTED))
  =>
    (modify ?pa (state EXECUTING) (actual-start-time (- (now) ?t)))
  )

  (defrule cx-pddl-clips-agent-execution-done
  " After the duration has elapsed, the action is done "
    (time ?now)
    (pddl-plan (id ?plan-id) (plan-start ?t))
    ?pa <- (pddl-action (id ?id) (plan ?plan-id) (state EXECUTING) (planned-duration ?d) (name ?name)
      (actual-start-time ?s&:(< (+ ?s ?d ?t) ?now)))
  =>
    (bind ?duration (- (now) (+ ?s ?t)))
    (printout info "Executed action " ?name " in " ?duration " seconds" crlf)
    (modify ?pa (state DONE) (actual-duration ?duration))
    (assert (pddl-action-get-effect (action ?id) (apply TRUE)))
  )

  (defrule cx-pddl-clips-agent-print-exec-times
  " Once everything is done, print out planned vs actual times "
    (pddl-action)
    (not (pddl-action (state ~DONE)))
    (not (printed))
  =>
    (printout blue "Execution done" crlf)
    (do-for-all-facts ((?pa pddl-action)) TRUE
       (printout green "action " ?pa:name " "
         ?pa:params " " ?pa:planned-start-time "|" ?pa:planned-duration
         " vs actual " ?pa:actual-start-time "|" ?pa:actual-duration crlf
       )
    )
    (assert (printed))
  )

Let us look at the different rules more closely.

Initializing the PDDL Manager
-----------------------------

The rule ``cx-pddl-clips-agent-pddl-init`` initializes the setup of the PDDL manager.

.. code-block:: lisp

  (defrule cx-pddl-clips-agent-pddl-init
  =>
    (assert (pddl-manager (node "/pddl_manager")))
  )



Uploading the PDDL Instance
---------------------------

Once initialized, the rule ``cx-pddl-clips-agent-pddl-add-instance`` sets up the
domain and problem to be planned for, defines the goal fluents and triggers plan generation.

.. code-block:: lisp

  (defrule cx-pddl-clips-agent-pddl-add-instance
  " Setup PDDL instance with an active goal to plan for "
    (pddl-manager (ros-comm-init TRUE))
  =>
    (bind ?share-dir (ament-index-get-package-share-directory "cx_pddl_bringup"))
    (assert
      (pddl-instance
        (name test)
        (domain "domain.pddl")
        (problem "problem.pddl")
        (directory (str-cat ?share-dir "/pddl"))
      )
      (pddl-get-fluents (instance test))
      (pddl-create-goal-instance (instance test) (goal active-goal))
      (pddl-goal-fluent (instance test) (goal active-goal) (name on) (params a b))
      (pddl-goal-fluent (instance test) (goal active-goal) (name on) (params b c))
      (pddl-set-goals (instance test) (goal active-goal))
      (pddl-plan (id test-plan) (instance test) (goal active-goal) (plan-type TEMPORAL))
    )
  )

The interface provided by ``cx_pddl_clips`` takes care of translating the asserted facts into appropriate service requests.
It also enforces a strict ordering whenever multiple operations are requested at the same time, as in this case.


Action Execution
----------------

Once the PDDL manager is done processing the requests, the result is a temporal plan including ``pddl-action`` facts representing the actions to execute.
Here, a mockup execution and monitoring loop is provided that takes care of sequentially executing the actions by:

1. selecting an action with lowest planned start time

  .. code-block:: lisp

    (defrule cx-pddl-clips-agent-select-action
    " Start executing the first action of the resulting plan "
      ?plan <- (pddl-plan (id ?plan-id) (plan-start ?p-start))
      (not (pddl-action (state EXECUTING|SELECTED)))
      ?pa <- (pddl-action (plan ?plan-id) (planned-start-time ?t) (state IDLE))
      (not (pddl-action (plan ?plan-id) (state IDLE) (planned-start-time ?ot&:(< ?ot ?t))))
    =>
      (if (= ?p-start 0.0) then (modify ?plan (plan-start (now))))
      (modify ?pa (state SELECTED))
    )

2. checking the condition of the selected action

  .. code-block:: lisp

    (defrule cx-pddl-clips-agent-check-action
    " Before executing an action check the condition to make sure it is feasible "
      (pddl-action (id ?id) (state SELECTED) (name ?name) (params $?params))
      (not (pddl-action-condition (action ?id)))
    =>
      (assert (pddl-action-condition (instance test) (action ?id)))
    )


3. given a satisfied condition, the action is executed by means of waiting the planned duration of the action.

  .. code-block:: lisp

    (defrule cx-pddl-clips-agent-executable-action
    " Condition is satisfied, go ahead with execution "
      (pddl-plan (id ?plan-id) (plan-start ?t))
      (pddl-action-condition (action ?action-id) (state CONDITION-SAT))
      ?pa <- (pddl-action (id ?action-id) (plan ?plan-id) (name ?name) (params $?params) (state SELECTED))
    =>
      (modify ?pa (state EXECUTING) (actual-start-time (- (now) ?t)))
    )

4. Once the planned duration elapsed, the action effect is applied

  .. code-block:: lisp

    (defrule cx-pddl-clips-agent-execution-done
    " After the duration has elapsed, the action is done "
      (time ?now)
      (pddl-plan (id ?plan-id) (plan-start ?t))
      ?pa <- (pddl-action (id ?id) (plan ?plan-id) (state EXECUTING) (planned-duration ?d) (name ?name)
        (actual-start-time ?s&:(< (+ ?s ?d ?t) ?now)))
    =>
      (bind ?duration (- (now) (+ ?s ?t)))
      (printout info "Executed action " ?name " in " ?duration " seconds" crlf)
      (modify ?pa (state DONE) (actual-duration ?duration))
      (assert (pddl-action-get-effect (action ?id) (apply TRUE)))
    )

Finalizing Execution
--------------------

Once all actions have completed, CLIPS prints a timing summary.

.. code-block:: lisp

  (defrule cx-pddl-clips-agent-print-exec-times
  " Once everything is done, print out planned vs actual times "
    (pddl-action)
    (not (pddl-action (state ~DONE)))
    (not (printed))
  =>
    (printout blue "Execution done" crlf)
    (do-for-all-facts ((?pa pddl-action)) TRUE
       (printout green "action " ?pa:name " "
         ?pa:params " " ?pa:planned-start-time "|" ?pa:planned-duration
         " vs actual " ?pa:actual-start-time "|" ?pa:actual-duration crlf
       )
    )
    (assert (printed))
  )

The planning model used in this tutorial is defined in two files located in the package:

.. code-block:: text

   cx_pddl_bringup
   └── pddl
       ├── domain.pddl
       └── problem.pddl

The **domain file** describes the available actions and predicates of the planning
environment, while the **problem file** defines a specific planning task including
objects, the initial state and the desired goal state.

The tutorial uses a simple **Blocks World** planning domain. In this domain, a robot
manipulates blocks on a table in order to achieve a desired stacking configuration.

The robot can perform the following actions:

* pick up a block from the table
* put down a block onto the table
* stack a block onto another block
* unstack a block from another block

Each action has **preconditions** that must be satisfied before execution and
**effects** that modify the world state. All actions are modeled as
**durative actions**, meaning they take a certain amount of time to execute.

PDDL Domain
^^^^^^^^^^^

The domain definition is located at ``pddl/domain.pddl``

It describes the **types**, **predicates**, and **actions** available to the planner.

.. code-block:: PDDL

  (define (domain blocksworld)

    (:types
        block - object
    )

    (:predicates
        (on ?x - block ?y - block)      ; block ?x is on block ?y
        (on-table ?x - block)           ; block ?x is on the table
        (clear ?x - block)              ; nothing is on top of ?x
        (arm-empty)                     ; the robot is not holding anything
        (holding ?x - block)            ; the robot is holding block ?x
    )

    (:durative-action pick-up
        :parameters (?ob - block)
        :duration (= ?duration 1)
        :condition
            (and
                (at start (clear ?ob))
                (at start (on-table ?ob))
                (at start (arm-empty)))
        :effect
            (and
                (at end (not (on-table ?ob)))
                (at end (not (clear ?ob)))
                (at end (not (arm-empty)))
                (at end (holding ?ob))))

    (:durative-action put-down
        :parameters (?ob - block)
        :duration (= ?duration 1)
        :condition
            (at start (holding ?ob))
        :effect
            (and
                (at end (not (holding ?ob)))
                (at end (clear ?ob))
                (at end (arm-empty))
                (at end (on-table ?ob)))
    )

    (:durative-action stack
        :parameters (?ob1 - block ?ob2 - block)
        :duration (= ?duration 2)
        :condition
            (and
                (at start (holding ?ob1))
                (at start (clear ?ob2)))
        :effect
            (and
                (at end (not (holding ?ob1)))
                (at end (not (clear ?ob2)))
                (at end (clear ?ob1))
                (at end (arm-empty))
                (at end (on ?ob1 ?ob2)))
    )

    (:durative-action unstack
        :parameters (?ob1 - block ?ob2 - block)
        :duration (= ?duration 1)
        :condition
            (and
                (at start (on ?ob1 ?ob2))
                (at start (clear ?ob1))
                (at start (arm-empty)))
        :effect
            (and
                (at end (holding ?ob1))
                (at end (clear ?ob2))
                (at end (not (clear ?ob1)))
                (at end (not (arm-empty)))
                (at end (not (on ?ob1 ?ob2))))
    )
  )

PDDL Problem
^^^^^^^^^^^^

The problem definition is located at ``pddl/problem.pddl``:

The problem file defines a **specific scenario** for the domain. It specifies
the available objects, the initial world state, and the desired goal configuration.

.. code-block:: PDDL

  (define (problem bw-1)
      (:domain blocksworld)

      (:objects A B C - block)

      (:init
          (arm-empty)
          (on-table A)
          (on B A)
          (on C B)
          (clear C)
      )

      (:goal
          (and
              (on A B)
              (on B C)
          )
      )
  )

In the initial configuration, block ``C`` is on ``B`` and ``B`` is on ``A``.
The planner must compute a sequence of actions that rearranges the blocks
so that ``A`` is on ``B`` and ``B`` is on ``C``.


Running the PDDL Agent
----------------------

The tutorial uses the generic launch file provided by ``cx_pddl_bringup``.
This launch file starts the PDDL manager and the CLIPS agent and loads the
configuration specified via the ``manager_config`` argument.

.. code-block:: bash

   ros2 launch cx_pddl_bringup cx_pddl_launch.py manager_config:=pddl_agents/cx_pddl_clips_agent.yaml

You should see the CLIPS output showing initialization, goal setup, planning,
and the full action execution process with timing data.


.. code-block:: bash:
   :emphasize-lines: 2,3

   [cx_node-2] [clips_manager] [INFO] Activated [clips_manager]...
   [cx_node-2] [ros_msgs] [WARN] service /pddl_manager/add_pddl_instance not available, abort request.
   [cx_node-2] [cx_pddl_clips_agent] [WARN] Sending of request failed, is the service /pddl_manager/add_pddl_instance running?
   [pddl_manager-1] [pddl_manager] [INFO] Start planning...
   [pddl_manager-1] [pddl_manager] [INFO] Successfully planned
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Executed action unstack in 1.1000874042511 seconds
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Executed action put-down in 1.09998250007629 seconds
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Executed action unstack in 1.09951519966125 seconds
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Executed action stack in 2.09970760345459 seconds
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Executed action pick-up in 1.09947609901428 seconds
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Executed action stack in 2.09946322441101 seconds
   [cx_node-2] [cx_pddl_clips_agent] [INFO] Execution done
   [cx_node-2] [cx_pddl_clips_agent] [INFO] action unstack ("c" "b") 0.0|1.0 vs actual 0.0989217758178711|1.1000874042511
   [cx_node-2] [cx_pddl_clips_agent] [INFO] action put-down ("c") 1.001|1.0 vs actual 1.49925684928894|1.09998250007629
   [cx_node-2] [cx_pddl_clips_agent] [INFO] action unstack ("b" "a") 2.002|1.0 vs actual 2.89918994903564|1.09951519966125
   [cx_node-2] [cx_pddl_clips_agent] [INFO] action stack ("b" "c") 3.003|2.0 vs actual 4.29887771606445|2.09970760345459
   [cx_node-2] [cx_pddl_clips_agent] [INFO] action pick-up ("a") 5.004|1.0 vs actual 6.69945955276489|1.09947609901428
   [cx_node-2] [cx_pddl_clips_agent] [INFO] action stack ("a" "b") 6.005|2.0 vs actual 8.09942483901978|2.09946322441101

Note that you might observe a warning message while running this example. This is expected, as the CX node may complete startup before the PDDL Manager does, requesting a service which is not available yet. The request is automatically retried again.

Summary
-------

You now have a **CLIPS PDDL agent** capable of:

* Setting up planning problems,
* Setting planning goals and requesting plans,
* Executing and monitoring actions,
* Comparing planned vs. actual execution durations.

This provides a powerful base for integrating reasoning and execution control
in robotic systems using the |CX| framework.
