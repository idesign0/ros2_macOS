PDDL CLIPS Interfaces
#####################

The PDDL integration is provided in the form of the `PDDL Manager`, a ROS lifecycle node that provides ROS interfaces, which can be called through the |CX| plugins that handle ROS communication.

In order to reduce the manual overhead, the ``cx_pddl_clips`` package provides a CLIPS-based interface for interacting with the PDDL Manager node in ROS 2.
This allows to interact with the PDDL manager by simply asserting and monitoring CLIPS facts, without the need to do direct ROS communication (e.g., populating ROS messages or waiting for service feedback).

PDDL Manager
************

The PDDL Manager node has the following responsibilities:

- Manage multiple PDDL instances concurrently.
- Load PDDL files (optionally templated via Jinja).
- Manage fluents, objects, functions, and goals.
- Check conditions and apply effects of actions.
- Configure planning goals.
- Applying planning filters to narrow down problems to a subset of actions, objects, and fluents for more detailed execution models.
- Trigger planning a for each goal concurrently.


PDDL Manager Services
*********************

The PDDL Manager exposes a wide range of ROS 2 interfaces.

.. list-table::
   :header-rows: 1
   :widths: 50 50

   * - Service / Topic
     - Corresponding Deftemplate(s)
   * - :abbr:`/add_pddl_instance (Add a new PDDL instance to the manager)`
     - :ref:`pddl-instance`
   * - :abbr:`/check_action_condition (Check if the preconditions of a PDDL action are satisfied)`
     - :ref:`pddl-action-condition`
   * - :abbr:`/get_action_effects (Retrieve the effects of a PDDL action)`
     - :ref:`pddl-action-get-effect`
   * - :abbr:`/get_action_names (Get the list of available action names for a PDDL instance)`
     - :ref:`pddl-action-names`
   * - :abbr:`/get_fluents (Fetch all current boolean fluents of a PDDL instance)`
     - :ref:`pddl-get-fluents`
   * - :abbr:`/add_fluents (Add multiple fluents to a PDDL instance)`
     - :ref:`pddl-fluent-change`
   * - :abbr:`/rm_fluents (Remove multiple fluents from a PDDL instance)`
     - :ref:`pddl-fluent-change`
   * - :abbr:`/add_objects (Add objects to a PDDL instance)`
     - :ref:`pddl-object-change`
   * - :abbr:`/rm_objects (Remove objects from a PDDL instance)`
     - :ref:`pddl-object-change`
   * - :abbr:`/set_functions (Set values for numeric functions in the PDDL instance)`
     - :ref:`pddl-numeric-fluent-change`
   * - :abbr:`/get_functions (Retrieve values of numeric functions)`
     - :ref:`pddl-get-numeric-fluents`
   * - :abbr:`/set_goals (Register goal conditions with the PDDL manager)`
     - :ref:`pddl-set-goals`
   * - :abbr:`/clear_goals (Clear all goal conditions of a PDDL instance)`
     - :ref:`pddl-clear-goals`
   * - :abbr:`/set_action_filter (Apply a filter to restrict planning to specific actions)`
     - :ref:`pddl-planning-filter`
   * - :abbr:`/set_object_filter (Apply a filter to restrict planning to specific objects)`
     - :ref:`pddl-planning-filter`
   * - :abbr:`/set_fluent_filter (Apply a filter to restrict planning to specific fluents)`
     - :ref:`pddl-planning-filter`
   * - :abbr:`/create_goal_instance (Create a new goal instance for planning)`
     - :ref:`pddl-create-goal-instance`
   * - :abbr:`/get_predicates (Fetch all predicates of a PDDL instance)`
     - :ref:`pddl-get-predicates`
   * - :abbr:`/get_type_objects (Fetch all objects of a certain type in a PDDL instance)`
     - :ref:`pddl-get-type-objects`
   * - :abbr:`/temp_plan (Send a temporal planning goal and retrieve the resulting plan)`
     - :ref:`pddl-plan`, :ref:`pddl-action`
   * - :abbr:`/instance_update (Publishes updates about instance changes in the manager)`
     - :ref:`pddl-manager`

CLIPS Integration
*****************

CLIPS needs to interact with the PDDL Manager by creating subscriptions and clients for the endpoints to then utilize the features as desired.
In order to reduce manual overhead, predefined rules and deftemplates are available, which are described in the remainder of this document.

The idea is to have templates for facts that correspond to the individual endpoints of the PDDL manager.
The workflow then is for users to simply assert a fact of a defined template to trigger the request to the PDDL manager, then using rules to observe the provided slots for the outcome.

A comprehensive example for using this integration is given in the :ref:`Tutorial for using the PDDL Manager with cx_pddl_clips <cx_pddl_clips_tutorial>`
An example for using the interfaces directly without this additional integration can be found in the :ref:`Tutorial for using the PDDL Manager directly <raw_pddl_agent>`.

Request Execution Model
~~~~~~~~~~~~~~~~~~~~~~~

The CLIPS interface allows users to assert multiple request facts concurrently
(e.g., adding fluents, creating goal instances, or checking action conditions).
However, most interactions with the external PDDL manager must be processed
**sequentially per PDDL instance**.

To guarantee consistency between CLIPS and the PDDL manager, the rule base
ensures that only **one request affecting a given PDDL instance is active at a
time**. Additional requests remain pending until the current operation has
completed.

This coordination is handled using the ``busy-with`` slot of the
:ref:`pddl-instance` fact. While an instance is marked as busy, other rules that
would trigger conflicting service calls are temporarily blocked.


Example:

* A request to add fluents must finish before conditions are checked.
* Retrieving fluents or predicates waits until previous updates have completed.
* Goal configuration steps (creating goal instances, setting goals, applying
  filters) are executed in order.

Parallel Planning
~~~~~~~~~~~~~~~~~

Planning requests are handled differently.

Once a goal instance has been created and the goals are registered, a planning
request (e.g., via :ref:`pddl-plan`) is sent to the planner asynchronously.
Multiple planning tasks may therefore run **in parallel**, allowing the system
to evaluate different goals or planning configurations concurrently.

When a plan is returned, the CLIPS interface automatically creates the
corresponding :ref:`pddl-plan` and :ref:`pddl-action` facts.

Rule Priorities
~~~~~~~~~~~~~~~

Internally, the rule base uses custom saliences (priorities) to enforce a
deterministic ordering of PDDL-related operations (e.g., instance management,
object updates, fluent updates, goal configuration, and planning). This ensures
that dependent operations are always processed in a consistent order while
allowing CLIPS reasoning to interleave with ROS communication.

THe full list of used rule saliences is depicted below:

.. code-block:: lisp

   (defglobal
    ?*PRIORITY-PDDL-INSTANCES* = -5100
    ?*PRIORITY-PDDL-GET-ACTION-NAMES* = -5300
    ?*PRIORITY-PDDL-OBJECTS* = -5400
    ?*PRIORITY-PDDL-FLUENTS* = -5500
    ?*PRIORITY-PDDL-APPLY-EFFECT* = -5600
    ?*PRIORITY-PDDL-CLEAR-GOALS* = -5700
    ?*PRIORITY-PDDL-CREATE-GOAL-INSTANCE* = -5800
    ?*PRIORITY-PDDL-SET-ACTION-FILTER* = -5900
    ?*PRIORITY-PDDL-SET-FLUENT-FILTER* = -6000
    ?*PRIORITY-PDDL-SET-OBJECT-FILTER* = -6100
    ?*PRIORITY-PDDL-SET-GOALS* = -6200
    ?*PRIORITY-PDDL-PLAN* = -6300
    ?*PRIORITY-PDDL-CHECK-CONDITION* = -6400
    ?*PRIORITY-PDDL-GET-FLUENTS* = -6500
  )

Template Overrides
~~~~~~~~~~~~~~~~~~

The templates provided by the `cx_pddl_clips` package contain a minimal set of slots to provide the functionality.
In practical applications, it might be convenient to store additional information (e.g., context as to why a particular service call is made or hints about how to interpret the results).
Therefore, deftemplate definitions are decoupled from the rules, allowing to first load the deftemplates, then loading re-definitions of them as needed, and finally loading the rule set.
Potential overrides may add more slots, but need to contain all the original slots, as otherwise the predefined rule base will not work.

In the :ref:`cx_pddl_clips Agent Tutorial <cx_pddl_clips_tutorial>`,
this feature is used to extend the notion of PDDL actions.


Using the |CX| with cx_pddl_clips
*********************************

The example configuration below demonstrates how to pre-load the code from the `cx_pddl_clips` package via batch loading, before loading the example code.
In order to integrate the PDDL manager with the |CX|, the following plugins are needed:

- :ref:`cx::ExecutivePlugin <usage_executive_plugin>`: Manages the overall reasoning and control flow, interleaving ROS feedback with CLIPS reasoning.
- :ref:`cx::RosMsgsPlugin <usage_ros_msgs_plugin>`: Provides access to ROS interfaces of the PDDL manager from within CLIPS.
- :ref:`cx::AmentIndexPlugin <usage_ament_index_plugin>`: Resolves package paths via ``ament_index``. While not required, it is very useful in order to load PDDL files.

Also, as the current configuration is compatible with ROS 2 jazzy, action client introspection is not supported, hence the following plugins are needed to trigger temporal planning and obtain the resulting plan:

- ``cx::CXCxPddlMsgsPlanTemporalPlugin``
- ``cx::CXCxPddlMsgsTimedPlanActionPlugin``


.. code-block:: yaml

   /**:
     ros__parameters:
       autostart_node: true
       environments: ["example_agent"]

       structured_pddl_agent:
         plugins: ["executive", "ros_msgs",
                   "pddl_files",
                   "files"]
         log_clips_to_file: true
         watch: ["facts", "rules"]

       ament_index:
         plugin: "cx::AmentIndexPlugin"

       executive:
         plugin: "cx::ExecutivePlugin"

       ros_msgs:
         plugin: "cx::RosMsgsPlugin"

       pddl_files:
         plugin: "cx::FileLoadPlugin"
         pkg_share_dirs: ["cx_pddl_clips"]
         batch: [
           "clips/cx_pddl_clips/pddl.clp"
         ]

       files:
         plugin: "cx::FileLoadPlugin"
         pkg_share_dirs: ["cx_pddl_bringup"]
         load: ["clips/cx_pddl_clips_agent.clp"]

Example: Loading a PDDL Problem and Obtaining the initial State
***************************************************************

The following example rule demonstrates how to initialize the connection to the external
PDDL manager and load a PDDL problem into CLIPS.

.. code-block:: lisp

  (defrule example-pddl-add-instance
    " Setup PDDL instance and fetch initial facts "
    =>
    (assert
      (pddl-manager (node "/pddl_manager"))
      (pddl-instance
        (name test)
        (domain "domain.pddl")
        (problem "problem.pddl")
        (directory "<absolute path to directory>")
      )
      (pddl-get-fluents (instance test))
    )
  )

The first step is to initialize the interfaces to a running PDDL manager instance by asserting a respective :ref:`pddl-manager` fact.
To create an instance, a :ref:`pddl-instance` fact is asserted, providing the domain and
problem files, as well as the directory containing them.
In order to fetch the initial fluents of the instance (as provided in the problem file), a :ref:`pddl-get-fluents` fact is asserted.


Provided Deftemplates
*********************

In the remainder of this document, all provided deftemplates of the `cx_pddl_clips` are described.

.. _pddl-action:

pddl-action
^^^^^^^^^^^

Represents a grounded PDDL action in a PDDL instance.
Automatically created when a plan is received.
Can also be asserted directly, if actions are needed in different contexts (e.g., to apply effects).
Used to track parameters, plan order, and scheduled times.

.. code-block:: lisp

  (deftemplate pddl-action
    (slot instance (type SYMBOL))
    (slot id (type SYMBOL))
    (slot plan (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot plan-order-class (type INTEGER))
    (slot planned-start-time (type FLOAT))
    (slot planned-duration (type FLOAT))
  )

.. _pddl-action-condition:

pddl-action-condition
^^^^^^^^^^^^^^^^^^^^^

Represents a condition of a PDDL action.
Requires a :ref:`pddl-action` fact with ``id`` matching the value in slot ``action``.
Obtains the satisfaction state and unsatisfied conditions.

.. code-block:: lisp

  (deftemplate pddl-action-condition
    (slot instance (type SYMBOL))
    (slot action (type SYMBOL))
    (slot condition-type (type SYMBOL) (allowed-values ALL START OVERALL END) (default START))
    (slot state (type SYMBOL) (allowed-values PENDING CHECK-CONDITION CONDITION-SAT CONDITION-UNSAT) (default PENDING))
    (multislot unsatisfied-conditions (type STRING) (default (create$)))
  )

.. _pddl-action-get-effect:

pddl-action-get-effect
^^^^^^^^^^^^^^^^^^^^^^

Request the effect of a grounded PDDL action.
Requires a :ref:`pddl-action` fact with ``id`` matching the value in slot ``action``.
Optionally, the effect can also be directly applied using the slot ``apply``.
If effectsa are also applied, corresponding :ref:`pddl-fluent-change` and ref:`pddl-numeric-fluent-change` facts are asserted, before the state is set to ``DONE``.

.. todo:

   This means that the effect is not applied yet, when the field is DONE. a better approach would be to apply the effects when processing the original requrest, then DONE would properly indicate that the effects are actually arlready applied

.. code-block:: lisp

  (deftemplate pddl-action-get-effect
    (slot action (type SYMBOL))
    (slot effect-type (type SYMBOL) (allowed-values ALL START END) (default ALL))
    (slot apply (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
    (slot state (type SYMBOL) (allowed-values PENDING WAITING START-EFFECT-APPLIED DONE ERROR) (default PENDING))
  )

.. _pddl-action-names:

pddl-action-names
^^^^^^^^^^^^^^^^^

Retrieve the list of action names for a PDDL instance.
This is particularly useful for defining goal filters.

.. code-block:: lisp

  (deftemplate pddl-action-names
    (slot instance (type SYMBOL))
    (multislot action-names (type SYMBOL) (default (create$)))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-clear-goals:

pddl-clear-goals
^^^^^^^^^^^^^^^^

Clears all goal conditions for a PDDL instance via the external manager.
This assumes that a matching goal instance was created via :ref:`pddl-create-goal-instance` beforehand.

.. code-block:: lisp

  (deftemplate pddl-clear-goals
    (slot instance (type SYMBOL))
    (slot goal (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-create-goal-instance:

pddl-create-goal-instance
^^^^^^^^^^^^^^^^^^^^^^^^^

Create a new managed goal instance for a PDDL instance via the external manager.

.. todo:

   We should probably have a pddl-goal-instance fact similar as to how we have an instance fact (or alernatively, add a goals multifield to each instance)

.. code-block:: lisp

  (deftemplate pddl-create-goal-instance
    (slot instance (type SYMBOL))
    (slot goal (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-effect-fluent:

pddl-effect-fluent
^^^^^^^^^^^^^^^^^^

Represents a boolean effect of a PDDL action.
Asserted when retrieving an action effect via :ref:`pddl-action-get-effect`.

.. code-block:: lisp

  (deftemplate pddl-effect-fluent
    (slot instance (type SYMBOL))
    (slot action (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot effect-type (type SYMBOL) (allowed-values ALL START END) (default ALL))
  )

.. _pddl-effect-numeric-fluent:

pddl-effect-numeric-fluent
^^^^^^^^^^^^^^^^^^^^^^^^^^

Represents a numeric effect of a PDDL action at a specific time.
Asserted when retrieving an action effect via :ref:`pddl-action-get-effect`.

.. todo:

   numeric effects are not supported fully

.. code-block:: lisp

  (deftemplate pddl-effect-numeric-fluent
    (slot instance (type SYMBOL))
    (slot action (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot value (type FLOAT))
    (slot effect-type (type SYMBOL) (allowed-values ALL START END) (default ALL))
  )


.. _pddl-fluent:

pddl-fluent
^^^^^^^^^^^

Represents a boolean fluent in a PDDL instance.
Do not assert/retract/modify directly; use :ref:`pddl-fluent-change` to keep the external PDDL manager in sync.

.. code-block:: lisp

  (deftemplate pddl-fluent
    (slot instance (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
  )

.. _pddl-fluent-change:

pddl-fluent-change
^^^^^^^^^^^^^^^^^^

Indicates that a boolean fluent should be added or removed.
Acts as a transient layer to synchronize with the external PDDL manager.
This automatically updates the corresponding :ref:`pddl-fluent` fact.

.. code-block:: lisp

  (deftemplate pddl-fluent-change
    (slot instance (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot delete (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
    (slot request-id (type INTEGER))
    (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-get-fluents:

pddl-get-fluents
^^^^^^^^^^^^^^^^

Fetch all boolean fluents for a PDDL instance via the external manager.
Asserts the corresponding :ref:`pddl-fluent` facts.

.. code-block:: lisp

  (deftemplate pddl-get-fluents
    (slot instance (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-get-numeric-fluents:

pddl-get-numeric-fluents
^^^^^^^^^^^^^^^^^^^^^^^^

Fetch all numeric fluents for a PDDL instance via the external manager.
Asserts the corresponding :ref:`pddl-numeric-fluent` facts.

.. code-block:: lisp

  (deftemplate pddl-get-numeric-fluents
    (slot instance (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-get-predicates:

pddl-get-predicates
^^^^^^^^^^^^^^^^^^^

Fetch all predicates for a PDDL instance via the external manager.
Asserts the corresponding :ref:`pddl-predicate` facts.

.. code-block:: lisp

  (deftemplate pddl-get-predicates
    (slot instance (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-get-type-objects:

pddl-get-type-objects
^^^^^^^^^^^^^^^^^^^^^

Fetch all objects of a specific type for a PDDL instance via the external manager.
Asserts the corresponding :ref:`pddl-type-objects` fact.

.. code-block:: lisp

  (deftemplate pddl-get-type-objects
    (slot instance (type SYMBOL))
    (slot type (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )

.. _pddl-goal-fluent:

pddl-goal-fluent
^^^^^^^^^^^^^^^^

Represents a positive boolean goal condition for a PDDL instance. Negative conditions are not supported as of now.

.. todo:

   support negative goal fluents

.. code-block:: lisp

  (deftemplate pddl-goal-fluent
    (slot instance (type SYMBOL))
    (slot goal (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
  )

.. _pddl-goal-numeric-fluent:

pddl-goal-numeric-fluent
^^^^^^^^^^^^^^^^^^^^^^^^

Represents a numeric goal condition with a specific value in a PDDL instance.

.. code-block:: lisp

  (deftemplate pddl-goal-numeric-fluent
    (slot instance (type SYMBOL))
    (slot goal (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot value (type FLOAT))
  )

.. _pddl-instance:

pddl-instance
^^^^^^^^^^^^^

Initialize a PDDL instance with the external manager.
Also ensures that no confliction requests are made simultaneously, by always tracking, the current operation.

.. code-block:: lisp

  (deftemplate pddl-instance
    (slot name (type SYMBOL))
    (slot domain (type STRING))
    (slot problem (type STRING))
    (slot directory (type STRING))
    (slot state (type SYMBOL) (allowed-values PENDING LOADED ERROR) (default PENDING))
    (slot busy-with (type SYMBOL) (allowed-values FALSE OBJECTS FLUENTS NUMERIC-FLUENTS ACTION-EFFECTS CREATE-GOAL-INSTANCE CLEAR-GOALS SET-GOALS CHECK-CONDITIONS GET-FLUENTS GET-NUMERIC-FLUENTS GET-PREDICATES GET-TYPE-OBJECTS GET-ACTION-NAMES SET-ACTION-FILTER SET-OBJECT-FILTER SET-FLUENT-FILTER CREATE-GOAL-INSTANCE) (default FALSE))
    (slot error (type STRING))
  )

.. _pddl-manager:

pddl-manager
^^^^^^^^^^^^

Assert a fact of this type to specify the name of the PDDL manager node.

.. code-block:: lisp

  (deftemplate pddl-manager
  " Store information on the external pddl manager.
  "
    (slot node (type STRING) (default "/pddl_manager"))
  )

.. _pddl-numeric-fluent:

pddl-numeric-fluent
^^^^^^^^^^^^^^^^^^^

Represents a numeric fluent in a PDDL instance.
Use `pddl-numeric-fluent-change` to update values safely with the external PDDL manager.

.. code-block:: lisp

  (deftemplate pddl-numeric-fluent
    (slot instance (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot value (type FLOAT))
  )

.. _pddl-numeric-fluent-change:

pddl-numeric-fluent-change
^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that a numeric fluent should be added or removed.
Acts as a transient layer to synchronize with the external PDDL manager.

.. code-block:: lisp

  (deftemplate pddl-numeric-fluent-change
    (slot instance (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL) (default (create$)))
    (slot value (type FLOAT))
    (slot request-id (type INTEGER))
    (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
  )

.. _pddl-object-change:

pddl-object-change
^^^^^^^^^^^^^^^^^^

Indicates that an object should be added or removed from a PDDL instance.
Acts as a transient layer to synchronize with the external PDDL manager.

Note that this does not automatically update the :ref:`pddl-type-objects` facts.

.. code-block:: lisp

  (deftemplate pddl-object-change
    (slot instance (type SYMBOL))
    (slot name (type SYMBOL))
    (slot type (type SYMBOL))
    (slot delete (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
    (slot request-id (type INTEGER))
    (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
    (slot error (type STRING))
  )


.. _pddl-plan:

pddl-plan
^^^^^^^^^

Represents a PDDL plan.

Consists of individual `pddl-action` facts with matching plan id.

.. code-block:: lisp

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


.. _pddl-planning-filter:

pddl-planning-filter
^^^^^^^^^^^^^^^^^^^^

A transient layer for planning filters between the general PDDL interface and domain-specific usage.
Can filter actions, objects, or fluents by defining a whitelist.

.. code-block:: lisp

  (deftemplate pddl-planning-filter
    (slot id (type SYMBOL))
    (slot type (type SYMBOL) (allowed-values ACTIONS OBJECTS FLUENTS))
    (multislot filter (type SYMBOL) (default (create$)))
    (slot instance (type SYMBOL))
    (slot goal (type SYMBOL))
  )

.. _pddl-predicate:

pddl-predicate
^^^^^^^^^^^^^^

Represents a predicate in a PDDL instance.

.. code-block:: lisp

  (deftemplate pddl-predicate
    (slot instance (type SYMBOL))
    (slot name (type SYMBOL))
    (multislot param-types (type SYMBOL) (default (create$)))
    (multislot param-names (type SYMBOL) (default (create$)))
  )


.. _pddl-service-request-meta:

pddl-service-request-meta
^^^^^^^^^^^^^^^^^^^^^^^^^

Facts of these types are used internally and should be ignored.


.. code-block:: lisp

  (deftemplate pddl-service-request-meta
    (slot service (type STRING))
    (slot request-id (type INTEGER))
    (slot meta (type SYMBOL))
  )

.. _pddl-set-goals:

pddl-set-goals
^^^^^^^^^^^^^^

Register goal conditions with the external PDDL manager.
The goal conditions are defined using :ref:`pddl-goal-fluent` and :ref:`pddl-goal-numeric-fluent` facts with matching goal ids.

.. code-block:: lisp

  (deftemplate pddl-set-goals
    (slot instance (type SYMBOL))
    (slot goal (type SYMBOL))
    (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
    (slot error (type STRING))
  )


.. _pddl-type-objects:

pddl-type-objects
^^^^^^^^^^^^^^^^^

Lists all objects of a certain type in a PDDL instance.
Created using :ref:`pddl-get-type-objects`.

.. code-block:: lisp

  (deftemplate pddl-type-objects
    (slot instance (type SYMBOL))
    (slot type (type SYMBOL))
    (multislot objects (type STRING) (default (create$)))
  )
