Reinforcement Learning CLIPS Interfaces
#######################################

To facilitate the development of CLIPS-based RL agents, this extension provides CLIPS logic to abstract away the ROS interactions and to provide a symbolic representation of the RL workflow as provided by the environments derived from the ``CXRLGym`` class.
In the following, the CLIPS interface is described as provided by the ``cx_rl_clips`` package.

.. important::

   The ``cx_rl_clips`` package requires the usage of ROS Kilted or above as it heavily relies on service introspection.

Using the |CX| with cx_rl_clips
*******************************

In order to integrate the CXRLGym with the |CX|, the following plugins are needed:

- :ref:`cx::ExecutivePlugin <usage_executive_plugin>`: Manages the overall reasoning and control flow, interleaving ROS feedback with CLIPS reasoning.
- :ref:`cx::RosMsgsPlugin <usage_ros_msgs_plugin>`: Provides access to ROS interfaces.
- :ref:`cx::RosParamPlugin <usage_ros_param_plugin>`: Used to fetch RL parameters that are also required within CLIPS.
- :ref:`cx::AmentIndexPlugin <usage_ament_index_plugin>`: Resolves package paths via ``ament_index``. Required for setting up the CLIPS interfaces.

Also, as the current configuration is compatible with ROS 2 kilted or above, action server introspection is not supported, hence the following plugins (generated through :ref:`ros_comm_gen <usage_cx_ros_comm_gen>` by the ``cx_rl_clips`` package) are needed:

- ``cx::CXCxRlInterfacesGetFreeRobotPlugin``
- ``cx::CXCxRlInterfacesExecActionSelectionPlugin``
- ``cx::CXCxRlInterfacesResetEnvPlugin``

With all required plugins loaded, the CLIPS interface can be initialized in one of two ways:

- By batch-loading the ``cx-rl.clp`` file from the ``cx_rl_clips`` package as provided.
- By loading ``deftemplates.clp`` and ``cx_rl_no_deftemplates.clp`` separately.

The latter approach allows extending or modifying the provided ``deftemplate`` definitions between the two loading steps.

A minimal configuration file is depicted below.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_rl_bringup"]
      cx_rl_bringup:
        plugins: ["executive",
                  "ament_index",
                  "ros_msgs",
                  "ros_param",
                  "action_selection",
                  "get_free_robot",
                  "reset_env",
                  "rl_files"]

      ament_index:
        plugin: "cx::AmentIndexPlugin"

      executive:
        plugin: "cx::ExecutivePlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      ros_param:
        plugin: "cx::RosParamPlugin"

      reset_env:
        plugin: "cx::CXCxRlInterfacesResetEnvPlugin"
      action_selection:
        plugin: "cx::CXCxRlInterfacesActionSelectionPlugin"

      get_free_robot:
        plugin: "cx::CXCxRlInterfacesGetFreeRobotPlugin"

      rl_files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_rl_clips"]
        batch: [
          "clips/cx_rl_clips/cx-rl.clp",
        ]


.. _clips_workflows:

Implementing RL Workflows in CLIPS
**********************************

Once the CLIPS interfaces are loaded, several deftemplates, rules and functions are loaded to the specified CLIPS environment to handle ROS communication and workflow logic to interact with RL nodes based on :ref:`CXRLGym environments <cx_rl_gym>`.

The following table summarizes the mapping between ROS interfaces and their
corresponding CLIPS deftemplates (all deftemplate definitions can be found).

.. list-table::
   :header-rows: 1
   :widths: 50 50

   * - ROS interface
     - Corresponding Deftemplate(s)

   * - :abbr:`/get_predefined_observables (Fetch predefined symbolic observables)`
     - :ref:`rl-predefined-observable`

   * - :abbr:`/get_observable_predicates (Fetch parameterized observable predicates)`
     - :ref:`rl-observable-predicate`

   * - :abbr:`/get_observable_objects (Fetch objects grouped by type)`
     - :ref:`rl-observable-type`

   * - :abbr:`/get_predefined_actions (Fetch predefined symbolic actions)`
     - :ref:`rl-predefined-action`

   * - :abbr:`/get_observable_actions (Fetch parameterized symbolic actions)`
     - :ref:`rl-observable-action`

   * - :abbr:`/get_status (Query RL node and environment status)`
     - :ref:`cx-rl-node`

   * - :abbr:`/reset_env (Reset the RL environment)`
     - :ref:`rl-reset-env`, :ref:`cx-rl-node`

   * - :abbr:`/get_current_observations (Fetch current environment observations)`
     - :ref:`rl-observation`, :ref:`rl-robot`

   * - :abbr:`/get_free_robot (Query a free robot for action execution)`
     - :ref:`rl-robot`, :ref:`rl-ros-action-meta-get-free-robot`

   * - :abbr:`/get_action_list_executable_for_robot (Query executable actions)`
     - :ref:`rl-current-action-space`, :ref:`rl-action`, :ref:`rl-robot`

   * - :abbr:`/action_selection (Execute a selected symbolic action)`
     - :ref:`rl-action`, :ref:`rl-ros-action-meta-action-selection`,
       :ref:`rl-action-request-meta`

   * - :abbr:`/get_episode_end (Check whether the current episode ended)`
     - :ref:`rl-episode-end`

   * - :abbr:`/exec_action_selection (Request policy-based action recommendation)`
     - :ref:`rl-current-action-space`, :ref:`rl-action`

By using the provided deftemplates, the individual steps for training and executing RL models can be naturally integrated in CLIPS agents.

Step 0: Configuration via Global Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Before starting the RL workflow, optional global configuration values can be defined
to customize logging behavior, reward shaping, and node identification within CLIPS.

These settings are provided as CLIPS global variables and influence the behavior
of the predefined rules and interfaces.

The following global variables may be overridden **after** loading the ``cx_rl.clp``
file by asserting a new ``defglobal`` definition:

.. code-block:: lisp

  (defglobal
    ?*CX-RL-LOG-LEVEL* = debug

    ?*CX-RL-REWARD-EPISODE-SUCCESS* = 0
    ?*CX-RL-REWARD-EPISODE-FAILURE* = 0
  )

To customize the ROS node name of the RL node, the following global variable must be
defined **before** loading the ``cx_rl.clp`` file. If not specified, it defaults to
``"/cx_rl_node"``.

.. code-block:: lisp

  (defglobal
    ?*CX-RL-NODE-NAME* = "/cx_rl_node"
  )


Step 1: Defining the Environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to define the RL observation space,
individual observations can be added using :ref:`rl-predefined-observable` facts.
Parameterized observations can be added using r:ref:`rl-observable-predicate` facts, defining parameters and their types, along with :ref:`rl-observable-type` facts describing the possible objects of a given type.
Similarly, the action space is defined using :ref:`rl-predefined-action` and `rl-observable-action` facts, along with the observable types.

The below snippet would describe an observation space containing `on(block1#block2), clear(block1), clear(block2), clear(block3), clear(block4)` and an action space containing `pickup(robot1#block1)`:

.. code-block:: lisp

  (assert
    (rl-predefined-observable (name on) (params block1 block2))
    (rl-observable-predicate (name clear) (param-names a) (param-types block))
    (rl-observable-type (type block) (objects block1 block2 block3 block4))
    (rl-predefined-action (name pickup) (params robot1 block1))
   )

.. admonition::

   The resulting internal string representations of the actions and opservation is constructed from the name and the parameters by encapsulating parameters with braces `()`. and separating individual parameters by `#`.

Aside from the action and observation space, the initial state needs to be defined.
This is handled through facts of type :ref:`rl-observation`.

The below example registers `clear(block1)` and `on-table(block1)` as current observation.

.. code-block:: lisp

   (assert
    (rl-observation (name clear) (params block1))
    (rl-observation (name on-table) (params block1))
   )

Once the observation space, action space, and initial observations are available, the RL environment is ready to be initialized.

Training or execution is started by asserting a :ref:`cx-rl-node` fact. This assertion triggers a backup of the current fact base that can be used for resetting the environment (as detailed in :ref:`Step 2 <cx_rl_clips_step2>`).
The fact must only be asserted if it does not already exist, as it will persist and be updated across environment resets.

.. code-block:: lisp

  (if (not (any-factp ((?node cx-rl-node)) (eq ?node:name ?*CX-RL-NODE-NAME*))) then
   (assert (cx-rl-node (name ?*CX-RL-NODE-NAME*) (mode UNSET)))
  )


.. _cx_rl_clips_step2:

Step 2: Defining the Reset Procedure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

During training, environment resets are triggered automatically by asserting
an :ref:`rl-reset-env` fact. The reset process is executed as a staged procedure,
where progress is controlled via the ``state`` slot of the fact.

The following reset states are processed in order:

* **ABORT-RUNNING-ACTIONS**
  Automatic step that gracefully terminates any currently executing actions
  before the episode reset begins.

* **USER-CLEANUP**
  User-defined hook executed before the default reset logic.
  From this state, users may either:

  - transition to ``LOAD-FACTS`` to continue with the default reset behavior, or
  - transition directly to ``DONE`` to fully replace the default reset procedure.

* **LOAD-FACTS**
  Automatic step that restores the CLIPS fact base to the snapshot taken after
  the initial assertion of the ``cx-rl-node`` fact.

* **USER-INIT**
  User-defined hook executed after the default reset has completed and before
  the next episode starts. Transitioning to ``DONE`` resumes training.

* **DONE**
  Finalizes the reset procedure and hands control back to the training loop.

When no customization of the reset procedure is required, it is sufficient to
define rules that advance the reset state through the user-defined stages
without performing additional actions:

.. code-block:: lisp

  (defrule reset-to-load-facts
    ?reset <- (rl-reset-env (state USER-CLEANUP))
    =>
    (modify ?reset (state LOAD-FACTS))
  )

  (defrule reset-to-done
    ?reset <- (rl-reset-env (state USER-INIT))
    =>
    (modify ?reset (state DONE))
  )

Step 3: Action Execution
~~~~~~~~~~~~~~~~~~~~~~~~
Action generation and selection are driven by the assertion of a
:ref:`rl-current-action-space` fact. The workflow differs slightly depending on
whether the system is operating in **training** or **execution** mode.

Training Mode
^^^^^^^^^^^^^


During training, the action-selection cycle is initiated automatically:

1. An :ref:`rl-current-action-space` fact with state ``PENDING`` is asserted by the
   system once a robot becomes available and new observations are present.

2. User-defined rules generate candidate actions by asserting :ref:`rl-action`
   facts based on the current observations and robot state.

3. After all candidate actions have been generated, the user transitions
   :ref:`rl-current-action-space` to state ``DONE``.

4. The system automatically selects one of the candidate actions by:

   - marking the corresponding :ref:`rl-action` fact with ``is-selected TRUE``, and
   - marking the associated :ref:`rl-robot` as busy by setting the ``waiting`` slot to ``FALSE``.

   If no candidate action is provided, the episode terminates pre-emptively.
   In this case, a ``no-op`` action is registered and rewarded using the value
   defined by the global variable ``?*CX-RL-REWARD-EPISODE-SUCCESS*``.

5. The user executes the selected action and:

   - updates environment observations via :ref:`rl-observation` facts,
   - marks the action as completed by setting the ``is-finished`` slot to ``TRUE``,
   - assigns a reward using the ``reward`` slot of the :ref:`rl-action` fact.

6. Collected rewards are consumed by the RL node to advance the learning
   procedure.

7. The user may also explicitly terminate the episode by asserting an
   :ref:`rl-episode-end` fact and setting the ``success`` slot accordingly.
   Depending on its value, either ``?*CX-RL-REWARD-EPISODE-SUCCESS*`` or
   ``?*CX-RL-REWARD-EPISODE-FAILURE*`` is registered.

8. Once training is complete, the fact :ref:`rl-end-training` is asserted to inform the user.

Execution Mode
^^^^^^^^^^^^^^

In execution mode, action selection is explicitly controlled by the user:

1. The user asserts an :ref:`rl-current-action-space` fact in ``EXECUTION`` mode.

2. Based on the current observations and robot availability, the user asserts
   executable :ref:`rl-action` facts and assigns them to a robot.

3. After all candidate actions have been defined, the user transitions
   :ref:`rl-current-action-space` to state ``DONE``.

4. The system automatically marks one of the candidate actions as selected by
   setting ``is-selected TRUE`` and marking the associated :ref:`rl-robot` as busy by setting slot ``waiting`` to ``FALSE``.

5. The user executes the selected action and updates the environment state by
   asserting new  :ref:`rl-observation` facts.

6. This process may be repeated until no further actions are available or the
   execution goals are satisfied.


If no actions are specified before setting the :ref:`rl-current-action-space` fact to ``DONE``, then the prediction asserts a :ref:`rl-action` fact with name ``no-op``.


.. _rl_deftemplates:

Provided Deftemplates
*********************

In the remainder of this document, all provided deftemplates of the ``cx_rl_clips``
integration are described. These templates form the symbolic interface between
CLIPS-based reasoning and a CXRLGym reinforcement learning environment.

They are used to define symbolic observation and action spaces, represent the
current environment state, generate and execute actions, and control training
and execution workflows.

When working with templates, make sure you understand which facts and slots are automatically populated by cx_rl_clips and which must be defined by the user.
Providing or overriding automatically managed fields may lead to unintended behavior.

.. _rl-reset-env:

rl-reset-env
~~~~~~~~~~~~

Represents a request to reset the RL environment.
This fact is asserted by the environment and processed to coordinate cleanup, initialization, and reset transitions.
Users are responsible to transition the ``state`` slot from `USER-CLEANUP` to `LOAD-FACTS` and from `USER-INIT` to `DONE`.

.. code-block:: lisp

  (deftemplate rl-reset-env
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot state (type SYMBOL)
      (allowed-values ABORT-RUNNING-ACTIONS USER-CLEANUP LOAD-FACTS USER-INIT DONE))
    (slot uuid (type STRING))
  )


.. _cx-rl-node:

cx-rl-node
~~~~~~~~~~

Represents the RL node instance and its current lifecycle state.
It also serves as the main entry point for tracking training and execution
progress.

This fact is asserted once by the user during initialization and updated automatically
via status queries.
Users should assert this fact and specify the slot ``name`` according to the global variable `*?CX-RL-NODE-NAME*` used for loading ``cx_rl_clips``.


.. code-block:: lisp

  (deftemplate cx-rl-node
    (slot name (type STRING) (default "/cx_rl_node"))
    (slot ros-comm-init (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
    (slot fact-reset-file (type STRING) (default ""))
    (slot mode (type SYMBOL) (allowed-values UNSET TRAINING EXECUTION))
    (slot episode (type INTEGER))
    (slot step (type INTEGER))
    (slot total-steps (type INTEGER))
    (slot model-loaded (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
  )


.. _rl-get-status:

rl-get-status
~~~~~~~~~~~~~

Request the current status of the RL environment.
Asserting this fact causes the corresponding :ref:`cx-rl-node` fact to be updated.
The system initially requests updates in order to monitor the startup process.
Users may assert this fact (and set the slot ``node``) request updates to observe training progression.

.. code-block:: lisp

  (deftemplate rl-get-status
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot request-id (type INTEGER)) ; internal
  )


.. _rl-end-training:

rl-end-training
~~~~~~~~~~~~~~~

Signals that training has completed. This fact is asserted by the environmnent
and notifies users about the end of training.

.. code-block:: lisp

  (deftemplate rl-end-training
    (slot node (type STRING) (default "/cx_rl_node"))
  )


.. _rl-episode-end:

rl-episode-end
~~~~~~~~~~~~~~

Marks the end of an episode during training.
Users can assert this to indicate indicate success or failure of the current episode and trigger environment resets.


.. code-block:: lisp

  (deftemplate rl-episode-end
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot success (type SYMBOL)
      (allowed-values TRUE FALSE)
      (default TRUE))
  )


.. _rl-observable-type:

rl-observable-type
~~~~~~~~~~~~~~~~~~

Defines a symbolic type and its corresponding objects.
These definitions are provided by the user and used to construct grounded observation and action spaces.

.. code-block:: lisp

  (deftemplate rl-observable-type
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot type (type SYMBOL))
    (multislot objects (type SYMBOL) (default (create$)))
  )


.. _rl-observable-predicate:

rl-observable-predicate
~~~~~~~~~~~~~~~~~~~~~~~

Defines a predicate with typed parameters.
All valid groundings are generated using the objects defined via
:ref:`rl-observable-type` for spanning the observation space.
Users are responsible for asserting facts of this type.

.. code-block:: lisp

  (deftemplate rl-observable-predicate
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot name (type SYMBOL))
    (multislot param-names (type SYMBOL))
    (multislot param-types (type SYMBOL))
  )


.. _rl-predefined-observable:

rl-predefined-observable
~~~~~~~~~~~~~~~~~~~~~~~~

Defines a grounded observable directly.
Used to add individual observations into the observation space.
Users are responsible for asserting facts of this type.

.. code-block:: lisp

  (deftemplate rl-predefined-observable
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL))
  )


.. _rl-predefined-action:

rl-predefined-action
~~~~~~~~~~~~~~~~~~~~

Defines a grounded action directly.
Used to add individual actions into the action space.
Users are responsible for asserting facts of this type.

.. code-block:: lisp

  (deftemplate rl-predefined-action
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL))
  )


.. _rl-observable-action:

rl-observable-action
~~~~~~~~~~~~~~~~~~~~

Defines a parameterized symbolic action.
All valid groundings are generated based on the defined parameter types and objects.
Users are responsible for asserting facts of this type.

.. code-block:: lisp

  (deftemplate rl-observable-action
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot name (type SYMBOL))
    (multislot param-names (type SYMBOL))
    (multislot param-types (type SYMBOL))
  )


.. _rl-observation:

rl-observation
~~~~~~~~~~~~~~

Represents a currently active observation.
The set of all asserted ``rl-observation`` facts defines the current environment state.
Users are responsible for asserting facts of this type.

.. code-block:: lisp

  (deftemplate rl-observation
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL))
  )


.. _rl-robot:

rl-robot
~~~~~~~~
Represents a robot that can execute actions.
A corresponding fact must be asserted for each robot whose actions are to be controlled by RL.
The ``waiting`` slot is managed automatically by the RL interface, indicating
that a robot is idling and ready for getting a new action assigned.

.. code-block:: lisp

  (deftemplate rl-robot
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot name (type SYMBOL))
    (slot waiting (type SYMBOL) (allowed-values TRUE FALSE) (default TRUE))
  )


.. _rl-current-action-space:

rl-current-action-space
~~~~~~~~~~~~~~~~~~~~~~~
Asserted automatically to indicate the generation phase of the current action space.
Facts of this type are created with the slot ``state`` set to `PENDING`, signaling that the user must assert :ref:`rl-action` facts before advancing the ``state`` to `DONE`.
Once the state is set to ``DONE``, action selection proceeds automatically.

.. code-block:: lisp

  (deftemplate rl-current-action-space
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot state (type SYMBOL) (allowed-values PENDING DONE) (default PENDING))
  )


.. _rl-action:

rl-action
~~~~~~~~~

Users must assert facts of this type whenever a corresponding
:ref:`rl-current-action-space` fact is present.
When asserting an ``rl-action`` fact, the slots ``node``, ``id``,
``name``, and ``params`` must be specified.
Note that the slot ``id`` should uniquely identify ``rl-action`` facts, while the slot ``name`` and ``params`` have to match those used when creating the action space.

The slots ``is-selected`` and ``assigned-to`` are managed automatically.
They indicate whether the action has been selected for execution and
which :ref:`rl-robot` is assigned to execute it.

Once an action is selected, the user is responsible for carrying out
its execution. Upon completion, the user must update the slots
``is-finished`` and ``reward`` to signal termination and provide the
reward obtained from the execution.


.. code-block:: lisp

  (deftemplate rl-action
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot id (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL))
    (slot is-finished (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
    (slot reward (type INTEGER) (default 0))
    (slot is-selected (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
    (slot assigned-to (type SYMBOL) (default nil))
  )


.. _rl-ros-action-meta-get-free-robot:

rl-ros-action-meta-get-free-robot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Represents internal metadata associated with the ``get_free_robot`` ROS action.
Facts of this type are asserted and managed automatically by the framework.

.. code-block:: lisp

  (deftemplate rl-ros-action-meta-get-free-robot
    (slot uuid (type STRING))
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot robot (type STRING))
    (slot last-search (type FLOAT))
    (slot found (type SYMBOL) (allowed-values TRUE FALSE))
    (slot abort-action (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
  )


.. _rl-ros-action-meta-action-selection:

rl-ros-action-meta-action-selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Internal metadata for the ``action_selection`` ROS action.
Facts of this type are asserted and managed automatically by the framework.

.. code-block:: lisp

  (deftemplate rl-ros-action-meta-action-selection
    (slot uuid (type STRING))
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot action-id (type SYMBOL))
    (slot abort-action (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
  )


.. _rl-action-request-meta:

rl-action-request-meta
~~~~~~~~~~~~~~~~~~~~~~

Internal request tracking for ROS service calls.
Facts of this type are asserted and managed automatically by the framework.

.. code-block:: lisp

  (deftemplate rl-action-request-meta
    (slot node (type STRING) (default "/cx_rl_node"))
    (slot service (type STRING))
    (slot request-id (type INTEGER))
    (slot action-id (type SYMBOL))
  )
