.. _usage_rl_interfaces_plugin:

Interfaces for RL-based Action Selection
########################################

Source code on .

===================== ============= =============================================
Interface Name        Type          Plugin Class
--------------------- ------------- ---------------------------------------------
create_rl_env_state   Service       cx::CXCxRlInterfacesCreateRLEnvStatePlugin
get_domain_objects    Service       cx::CXCxRlInterfacesGetDomainObjectsPlugin
get_domain_predicates Service       cx::CXCxRlInterfacesGetDomainPredicatesPlugin
get_action_list       Service       cx::CXCxRlInterfacesGetActionListPlugin
get_action_list_robot Service       cx::CXCxRlInterfacesGetActionListRobotPlugin
set_rl_mode           Service       cx::CXCxRlInterfacesSetRLModePlugin
get_free_robot        Action Server cx::CXCxRlInterfacesGetFreeRobotPlugin
action_selection      Action Server cx::CXCxRlInterfacesActionSelectionPlugin
reset_cx              Action Server cx::CXCxRlInterfacesResetCXPlugin
exec_action_selection Client        cx::CXCxRlInterfacesExecActionSelectionPlugin
===================== ============= =============================================

This plugin provides interfaces used in reinforcement learning based action selection to either access information from the CLIPS environment or to communicate a selected action to the reinforcement learning plugin. In the table above, the type specifies which part of the ROS 2 communication is implemented, if the user wants to use these interfaces, the corresponding counterpart needs to be used (Service <-> Client, Action Server <-> Action Client).

It is intended to use this plugin together with the cxrl_gym environment which serves as a base class in python implementing the counterparts to the interfaces specified here and most of the basic gym-functions (step, reset, action_mask, ...) used in (maskable action) reinforcement learning.

Configuration
*************

This plugin has no specific configuration options.

Features
********

Interfaces
~~~~~~~~~~

In the following each interface is presented and explained:

:`create_rl_env_state`:

  Type
    Service
  Description
    Creates a string containing all asserted rl-observation facts which have to be asserted by the user by overriding rl-generate-observation function. Format: {"OBSERVATION_1_NAME(PARAM_1,PARAM_2,...)", ...}
  Specification
    .. code-block:: lisp

        ---
        string state

:`get_observable_objects`:

  Type
    Service
  Description
    Returns string list of all objects set to be observable by the user (asserted rl-observable-predicate fact) and matching a given type. If there does not exist an object for the type, "Not found" is returned.
  Specification
    .. code-block:: lisp

        string type
        ---
        string[] objects

:`get_observable_predicates`:

  Type
    Service
  Description
    Returns information on all predicates set to be observable by the user (asserted rl-observable-predicate fact). ``predicatenames`` lists all the names. ``paramcounts`` specifies for every predicate name with the same index how many parameters the predicate features. ``paramnames`` and ``paramtypes`` list the names and types of all predicates where thee name and type share the same index and both lists are constructed in a way that the first paramcounts[0] entries in both lists are the parameters of predicate predicatenames[0] and so on.
  Specification
    .. code-block:: lisp

        ---
        string[] predicatenames
        uint8[] paramcounts
        string[] paramnames
        string[] paramtypes

:`get_predfined_observables`:

  Type
    Service
  Description
    Returns a list of all observation space entries which have been pre-defined (not defined using combinations of observable predicates and observable objects) which are asserted as rl-predefined-observable fact.
  Specification
    .. code-block:: lisp

        ---
        string[] observables

:`get_action_list`:

  Type
    Service
  Description
    Returns a list of ids and names of all existing rl-action facts i.e. all executable actions. Format: "actionid|actionname".
  Specification
    .. code-block:: lisp

        ---
        string[] actions

:`get_action_list_robot`:

  Type
    Service
  Description
    Returns a list of ids and names of all existing rl-action facts which have the specified robot as value in their ``assigned-to`` slot i.e. all executable actions for a certain robot. Format: "actionid|actionname".
  Specification
    .. code-block:: lisp

        string robot
        ---
        string[] actions

:`set_rl_mode`:

  Type
    Service
  Description
    Allows the client to set the mode of the reinforcement learning plugin. Available modes are TRAINING and EXECUTION. If successful, the message "Set mode to (MODE)" is returned, otherwise "Couldn't set mode".
  Specification
    .. code-block:: lisp

        string mode
        ---
        string confirmation

:`get_free_robot`:

  Type
    Action Server
  Description
    Calling this action starts a search for a robot which is currently not working on an action and thus waiting. When a free robot has been found, its name is returned. During the search, feedback may be returned stating that no free robot has been found and the search is continued.
  Specification
    .. code-block:: lisp

        ---
        string robot
        ---
        string feedback

:`action_selection`:

  Type
    Action Server
  Description
    Calling this action with a valid actionid starts the action selection process in CLIPS. In the matching rl-action fact, the ``is-selected`` flag gets set, signaling to the user generated agent which action has been selected. When the execution of the action has been finished, the user must set the ``is-finished`` flag which leads to the actionid, the earned reward and an info-string are returned. This string is currently only used to signal if an episode has finished in which case it says "Done" otherwise it is empty.
  Specification
    .. code-block:: lisp

        string actionid
        ---
        string actionid
        int32 reward
        string info
        ---
        string feedback

:`reset_cx`:

  Type
    Action Server
  Description
    Calling this action starts a reset procedure for the environment. The reset is done by loading the fact base at an initial state. After this reset, a string is returned confirming the successful reset ("Reset completed").
  Specification
    .. code-block:: lisp

        ---
        string confirmation
        ---
        string feedback

:`exec_action_selection`:

  Type
    Client
  Description
    This client is used if the reinforcement learning plugin is in EXECUTION mode. When an action selection is needed (i.e. there are executable actions and robots without a task), the client sends a request to a service implemented by the user. This request consists of the current state string (as specified in **_create_rl_env_state_**) and a list of executable action (as specified in **_get_action_list_**). Using this information a trained RL agent should be able to predict the next action which then must be returned to the client via its actionid.
  Specification
    .. code-block:: lisp

        string state
        string[] actions
        ---
        string actionid

Facts
~~~~~

.. code-block:: lisp

  ; asserted by the user whenever a corresponding user action is executable
  (deftemplate rl-action
      (slot id (type SYMBOL))
      (slot name (type SYMBOL))
      (slot is-selected   (type SYMBOL)
                          (allowed-values TRUE FALSE)
                          (default FALSE))
      (slot is-finished   (type SYMBOL)
                          (allowed-values TRUE FALSE)
                          (default FALSE))
      (slot assigned-to   (type SYMBOL)
                          (default nil))
      (slot points    (type INTEGER)
                      (default 0))
  )

  ; asserted by the user if the current episode has finished
  (deftemplate rl-episode-end
      (slot success   (type SYMBOL)
                      (allowed-values TRUE FALSE)
                      (default TRUE))
  )

  ; asserted by the user to include an object in the automatic observation space generation
  (deftemplate rl-observable-object
    (slot name (type SYMBOL))
    (slot type (type SYMBOL))
  )

  ; asserted by the user to include a predicate in the automatic observation space generation
  (deftemplate rl-observable-predicate
    (slot name (type SYMBOL))
    (multislot param-names (type SYMBOL))
    (multislot param-types (type SYMBOL))
  )

  ; asserted by the user to define a single observation space entry (without automatic generation)
  (deftemplate rl-predefined-observable
    (slot name (type SYMBOL))
    (multislot params (type SYMBOL))
  )

  ; asserted by the user to define a single observation (predicate name and object parameters)
  (deftemplate rl-observation
    (slot name (type SYMBOL))
    (multislot param-values (type SYMBOL))
  )

  ; asserted by the plugin to signal the beginning of an executability check (state CHECKING)
  ; user must modify the state to CHECKED when the executability check is completed
  (deftemplate rl-executability-check
    (slot state (type SYMBOL) (allowed-values PENDING CHECKING CHECKED) (default PENDING))
  )

  ; asserted by the plugin to keep track of the workers actions are assigned to
  (deftemplate rl-robot
    (slot name (type SYMBOL))
    (slot waiting (type SYMBOL) (allowed-values TRUE FALSE) (default TRUE))
  )

  ; asserted by the plugin, corresponds to a single selection process during training mode
  (deftemplate rl-action-selection
      (slot uuid (type STRING))
      (slot actionid (type SYMBOL))
      (slot is-finished   (type SYMBOL)
                          (allowed-values TRUE FALSE)
                          (default FALSE))
      (slot reward    (type INTEGER)
                      (default 0))
      (slot done  (type SYMBOL)
                  (allowed-values TRUE FALSE)
                  (default FALSE))
  )

  ; asserted by the plugin, corresponds to a single action selection during execution mode
  (deftemplate rl-action-selection-exec
      (slot actionid (type SYMBOL))
  )

  ; asserted by the plugin, shows the current mode
  (deftemplate rl-mode
      (slot mode  (type SYMBOL)
                  (allowed-values TRAINING EXECUTION))
  )

  ; asserted by the plugin whenever a robot is not assigned to a selected rl-action
  (deftemplate robot-waiting
      (slot robot (type SYMBOL))
  )

Functions
~~~~~~~~~

.. code-block:: lisp

  ; must be overwritten so that in this function all rl-observation facts are asserted
  ; that correspond to the current state of the environment
  (rl-generate-observations)

Saliences
~~~~~~~~~

.. code-block:: lisp

  ; must be used in rules checking the executability of an action
  ?*SALIENCE-ACTION-EXECUTABLE-CHECK*

  ; must be used in rules checking if the episode has ended
  ?*SALIENCE-RL-EPISODE-END-SUCCESS* = 500

Usage Example
*************

  A simple usage example can be found in the tutorial Blocksworld with Reinforcement Learning
