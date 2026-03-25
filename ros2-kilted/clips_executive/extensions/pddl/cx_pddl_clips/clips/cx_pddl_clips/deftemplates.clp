
; Copyright (c) 2025-2026 Carologistics
; SPDX-License-Identifier: Apache-2.0
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

(defglobal
  ?*CX-PDDL-SERVICE-CLIENTS* = (create$
    add_pddl_instance AddPddlInstance
    check_action_condition CheckActionCondition
    get_action_effects GetActionEffects
    get_action_names GetActionNames
    get_fluents GetFluents
    add_fluents AddFluents
    rm_fluents RemoveFluents
    add_objects AddObjects
    rm_objects RemoveObjects
    set_functions SetFunctions
    get_functions GetFunctions
    set_goals SetGoals
    clear_goals ClearGoals
    set_action_filter SetActionFilter
    set_object_filter SetObjectFilter
    set_fluent_filter SetFluentFilter
    create_goal_instance CreateGoalInstance
    get_predictes GetPredicates
    get_type_objects GetTypeObjects
  )
)

(deftemplate pddl-service-request-meta
  (slot service (type STRING))
  (slot request-id (type INTEGER))
  (slot meta (type SYMBOL))
)

(deftemplate pddl-manager
" Store information on the external pddl manager.
"
  (slot node (type STRING) (default "/pddl_manager"))
  (slot ros-comm-init (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
)

(deftemplate pddl-instance
" Interface for instances.clp
  Assert a fact of this type to initialize a pddl instance with the external pddl manager.
  @slot name: unique name to refer to when using this instance
  @slot domain: name of a domain.pddl file to be loaded.
  @slot problem: optional name of the problem.pddl, leave empty if no problem should be loaded initially.
  Slots set automatically:
  @slot state:
   - PENDING: The instance was not registered yet.
   - LOADED: The instance is loaded and ready for usage.
   - ERROR: The fluents were not fetched due to an error.
  @busy-with: Indicates the current operation
  @slot error: provide information on encountered errors.
"
  (slot name (type SYMBOL))
  (slot domain (type STRING))
  (slot problem (type STRING))
  (slot directory (type STRING))
  (slot state (type SYMBOL) (allowed-values PENDING LOADED ERROR) (default PENDING))
  (slot busy-with (type SYMBOL) (allowed-values FALSE OBJECTS FLUENTS NUMERIC-FLUENTS ACTION-EFFECTS CREATE-GOAL-INSTANCE CLEAR-GOALS SET-GOALS CHECK-CONDITIONS GET-FLUENTS GET-NUMERIC-FLUENTS GET-PREDICATES GET-TYPE-OBJECTS GET-ACTION-NAMES SET-ACTION-FILTER SET-OBJECT-FILTER SET-FLUENT-FILTER CREATE-GOAL-INSTANCE) (default FALSE))
  (slot error (type STRING))
)


(deftemplate pddl-action
" Represents a grounded pddl action in a pddl instance.
  @slot instance: pddl instance belonging to the action.
  @slot name: name of the action.
  @slot params: parameters of the  action.
  @slot plan: name of the associated plan
  @slot: planned-start-time: start time of the action in the plan
  @slot: planned-duration: assumed duration of the action according to plan
"
  (slot instance (type SYMBOL))
  (slot id (type SYMBOL)) ; this should be a globally unique ID
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot plan (type SYMBOL))
  (slot planned-start-time (type FLOAT))
  (slot planned-duration (type FLOAT))
)

(deftemplate pddl-goal-fluent
" Facts to represent goal conditions for planning.
  Each fact of this template represent one positive boolean fluent in a
  goal condition.
  Negative goal conditions are currently not supported.
"
  (slot instance (type SYMBOL))
  (slot goal (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
)

(deftemplate pddl-goal-numeric-fluent
" Facts to represent goal conditions for planning.
  Each fact of this template represent one numeric fluent in a goal condition
  with exactly the value specified in here.
  Note that this is a rather limited representation for numeric fluent
  conditions and can therefore only represent a subset of valid conditions.
"
  (slot instance (type SYMBOL))
  (slot goal (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot value (type FLOAT))
)

(deftemplate pddl-effect-fluent
" Facts to represent pddl action effects.
  Each fact of this template represent one boolean fluent in an action effect.
"
  (slot instance (type SYMBOL))
  (slot action (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot effect-type (type SYMBOL) (allowed-values ALL START END) (default ALL))
)

(deftemplate pddl-effect-numeric-fluent
" Facts to represent pddl action effects.
  Each fact of this template represent one numeric fluent in an action effect at a specific time.
  Note that this is a rather limited representation for numeric fluent
  conditions and can therefore only represent a subset of valid conditions.
"
  (slot instance (type SYMBOL))
  (slot action (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot value (type FLOAT))
  (slot effect-type (type SYMBOL) (allowed-values ALL START END) (default ALL))
)



(deftemplate pddl-fluent
" Represents a (boolean) fluent in a pddl instance.
  Do not retract/assert/modify facts of this type directly, rather use
  request-pddl-fluent facts to indicate the desired change.
  This ensures that the external pddl manager stays in sync with the CLIPS
  model.
  @slot instance: pddl instance to add the fluent to.
  @slot name: name of the fluent.
  @slot params: parameters of the fluent.
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
)

(deftemplate pddl-numeric-fluent
" Represents a numeric fluent in a pddl instance.
  Do not retract/assert/modify facts of this type directly, rather use
  request-pddl-numeric-fluent facts to indicate the desired change.
  This ensures that the external pddl manager stays in sync with the CLIPS
  model.
  @slot instance: pddl instance to add the fluent to.
  @slot name: name of the fluent.
  @slot params: parameters of the fluent.
  @slot value: value of the fluent.
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot value (type FLOAT))
)

(deftemplate pddl-predicate
" Represents a predicate in a pddl instance.
  @slot instance: pddl instance to the predicate is part of.
  @slot name: name of the predicate.
  @slot param-types: parameter types of the predicate.
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot param-types (type SYMBOL) (default (create$)))
  (multislot param-names (type SYMBOL) (default (create$)))
)

(deftemplate pddl-type-objects
" Lists all objects of a certain type in a pddl instance.
  @slot instance: pddl instance to the object is part of.
  @slot type: type of the object.
  @slot objects: list of the object names.
"
  (slot instance (type SYMBOL))
  (slot type (type SYMBOL))
  (multislot objects (type STRING) (default (create$)))
)

(deftemplate pddl-action-condition
  (slot instance (type SYMBOL))
  (slot action (type SYMBOL))
  (slot condition-type (type SYMBOL) (allowed-values ALL START OVERALL END) (default START)) ; TODO: only supports START
  (slot state (type SYMBOL) (allowed-values PENDING CHECK-CONDITION CONDITION-SAT CONDITION-UNSAT) (default PENDING))
  (multislot unsatisfied-conditions (type STRING) (default (create$)))
)

(deftemplate pddl-action-get-effect
" request the effect of a grounded pddl action.
  @slot action: id of the action.
  @slot state: TBD
"
  (slot action (type SYMBOL))
  (slot effect-type (type SYMBOL) (allowed-values ALL START END) (default ALL))
  (slot state (type SYMBOL) (allowed-values PENDING WAITING START-EFFECT-APPLIED DONE ERROR) (default PENDING))
  (slot apply (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
)

(deftemplate pddl-action-names
" Retrieve the list of action names.
  @slot instance: pddl instance to retrieve the action names for.
  Slots set automatically:
  @multislot action-names: retrieved list of action names
  @slot state:
   - PENDING: The action names were not retrieved yet.
   - ERROR: The names were not fetched due to an error.
   - DONE: The action-names slot now is filled properly.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (multislot action-names (type SYMBOL) (default (create$)))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-clear-goals
" Assert a fact of this type in order to clear all goal conditions of a given
  pddl instance with the external pddl manager.
  @slot instance: pddl instance which should clear it's goal conditions.
  Slots set automatically:
  @slot state:
   - PENDING: The goal is not cleared in the pddl manager yet.
   - DONE: The goal is cleared with the pddl manager.
   - ERROR: The goal is not cleared due to some error.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot goal (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-create-goal-instance
" Interface for create-goal-instance.clp
  Assert a fact of this type in order to create a new ManagedGoal instance of a given
  pddl instance with the external pddl manager.
  @slot instance: pddl instance for which to add the goal.
  Slots set automatically:
  @slot state:
   - PENDING: The goal instance is not created in the pddl manager yet.
   - DONE: The goal instance is created with the pddl manager.
   - ERROR: The goal instance is not created due to some error.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot goal (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-fluent-change
" Interface for fluents.clp
  Assert a fact of this type in order to indicate that a fluent needs to be
  added to/removed from a pddl instance.
  Acts as a transient layer to pddl-fluent facts to make sure the CLIPS
  representation stays consistant with the externally managed pddl instance.
  @slot instance: pddl instance to add the fluent to.
  @slot name: name of the fluent.
  @slot params: parameters of the fluent.
  @slot delete: if true, remove the fluent, else add it.
  Slots set automatically:
  @slot request-id: id of the associated ros service request
  @slot state:
   - PENDING: The fluent was not added yet.
   - WAITING: The fluent is about to be added and is waiting for confirmation.
   - ERROR: The fluent might not have been fetched due to an error.
   - ON-HOLD: Unused state that can be set in order to defer the fluent update
     to a later time (by switching it manually to PENDING).
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot delete (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
  (slot request-id (type INTEGER))
  (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-get-fluents
" Interface for get-fluents.clp
  Assert a fact of this type in order to fetch all positive boolean fluents
  of a given pddl instance with the external pddl manager.
  This results in the automatic assertion of all positive boolean fluents
  (pddl-fluent template facts) currently present in the given pddl instance.
  @slot instance: pddl instance from which the fluents are fetched.
  Slots set automatically:
  @slot state:
   - PENDING: The fluents were not fetched yet.
   - DONE: The fluents were successfully retrieved
   - ERROR: The fluents were not fetched due to an error.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-get-numeric-fluents
" Interface for get-fluents.clp
  Assert a fact of this type in order to fetch all numeric fluents
  of a given pddl instance with the external pddl manager.
  This results in the automatic assertion of all numeric fluents
  (pddl-numeric-fluent template facts) currently present in the given pddl
  instance.
  @slot instance: pddl instance from which the fluents are fetched.
  Slots set automatically:
  @slot state:
   - PENDING: The fluents were not fetched yet.
   - DONE: The fluents were successfully retrieved
   - ERROR: The fluents were not fetched due to an error.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-get-predicates
" Interface for get-predicates.clp
  Assert a fact of this type in order to fetch all predicates
  of a given pddl instance with the external pddl manager.
  This results in the automatic assertion of all predicates
  (pddl-predicate template facts) currently present in the given pddl
  instance.
  @slot instance: pddl instance from which the predicates are fetched.
  Slots set automatically:
  @slot state:
   - PENDING: The predicates were not fetched yet.
   - DONE: The predicates were successfully retrieved
   - ERROR: The predicates were not fetched due to an error.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-get-type-objects
" Interface for get-type-objects.clp
  Assert a fact of this type in order to fetch all objects of a certain type
  of a given pddl instance with thewith external pddl manager.
  This results in the automatic assertion of all objects of that type
  (pddl-type-objects template facts) currently present in the given pddl
  instance.
  @slot instance: pddl instance from which the objects are fetched.
  Slots set automatically:
  @slot type: type of objects to retrieve
  @slot state:
   - PENDING: The objects were not fetched yet.
   - DONE: The objects were successfully retrieved
   - ERROR: The objects were not fetched due to an error.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot type (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-instance
" Interface for instances.clp
  Assert a fact of this type to initialize a pddl instance with the external pddl manager.
  @slot name: unique name to refer to when using this instance
  @slot domain: name of a domain.pddl file to be loaded.
  @slot problem: optional name of the problem.pddl, leave empty if no problem should be loaded initially.
  Slots set automatically:
  @slot state:
   - PENDING: The instance was not registered yet.
   - LOADED: The instance is loaded and ready for usage.
   - ERROR: The fluents were not fetched due to an error.
  @busy-with: Indicates the current operation
  @slot error: provide information on encountered errors.
"
  (slot name (type SYMBOL))
  (slot domain (type STRING))
  (slot problem (type STRING))
  (slot directory (type STRING))
  (slot state (type SYMBOL) (allowed-values PENDING LOADED ERROR) (default PENDING))
  (slot busy-with (type SYMBOL) (allowed-values FALSE OBJECTS FLUENTS NUMERIC-FLUENTS ACTION-EFFECTS CREATE-GOAL-INSTANCE CLEAR-GOALS SET-GOALS CHECK-CONDITIONS GET-FLUENTS GET-NUMERIC-FLUENTS GET-PREDICATES GET-TYPE-OBJECTS GET-ACTION-NAMES SET-ACTION-FILTER SET-OBJECT-FILTER SET-FLUENT-FILTER CREATE-GOAL-INSTANCE) (default FALSE))
  (slot error (type STRING))
)

(deftemplate pddl-numeric-fluent-change
" Interface for numeric-fluents.clp
  Assert a fact of this type in order to indicate that a numeric fluent needs
  to be added to/removed from a pddl instance.
  Acts as a transient layer to pddl-numeric-fluent facts to make sure the CLIPS
  representation stays consistant with the externally managed pddl instance.
  @slot instance: pddl instance to add the fluent to.
  @slot name: name of the fluent.
  @slot params: parameters of the fluent.
  @slot value: value of the fluent.
  Slots set automatically:
  @slot request-id: id of the associated ros service request
  @slot state:
   - PENDING: The fluent was not added yet.
   - WAITING: The fluent is about to be added and is waiting for confirmation.
   - ERROR: The fluent might not have been fetched due to an error.
   - ON-HOLD: Unused state that can be set in order to defer the fluent update
     to a later time (by switching it manually to PENDING).
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (multislot params (type SYMBOL) (default (create$)))
  (slot value (type FLOAT))
  (slot request-id (type INTEGER))
  (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
)

(deftemplate pddl-object-change
" Interface for objects.clp
  Assert a fact of this type in order to indicate that an object needs to be
  added to/removed from a pddl instance.
  @slot instance: pddl instance to add the object to.
  @slot name: name of the object.
  @slot type: type of the object.
  @slot delete: if true, remove the object, else add it.
  Slots set automatically:
  @slot state:
   - PENDING: The object was not added yet.
   - WAITING: The object is about to be added and is waiting for confirmation.
   - ERROR: The object might not have been fetched due to an error.
   - ON-HOLD: Unused state that can be set in order to defer the object update
     to a later time (by switching it manually to PENDING).
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot name (type SYMBOL))
  (slot type (type SYMBOL))
  (slot delete (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
  (slot request-id (type INTEGER))
  (slot state (type SYMBOL) (allowed-values PENDING WAITING ERROR ON-HOLD) (default PENDING))
  (slot error (type STRING))
)

(deftemplate pddl-plan
"Assert a fact of this fact to plan"
  (slot instance (type SYMBOL))
  (slot id (type SYMBOL))
  (slot goal (type SYMBOL))
  (slot goal-ptr (type EXTERNAL-ADDRESS))
  (slot plan-type (type SYMBOL) (allowed-values CLASSICAL TEMPORAL) (default CLASSICAL))
  (slot goal-handle (type EXTERNAL-ADDRESS))
  (slot type (type SYMBOL) (allowed-values TEMPORAL CLASSICAL))
  (slot state (type SYMBOL) (allowed-values PENDING WAITING PLANNING REQUEST-CANCELING CANCELING CANCELED SUCCESS FAILURE) (default PENDING))
)


(deftemplate pddl-planning-filter
" This currently mainly is a transient layer betweeen the general pddl interface and our domain-specific usage.
  Can be extended later in case different kind of planning filters should be used or if planning is used in varying contexts.
"
  (slot id (type SYMBOL))
  (slot type (type SYMBOL) (allowed-values ACTIONS OBJECTS FLUENTS))
  (multislot filter (type SYMBOL) (default (create$ )))
  (slot instance (type SYMBOL))
  (slot goal (type SYMBOL))
)

(deftemplate pddl-set-goals
" Interface for set-goals.clp
  Assert a fact of this type in order to actually register the respective goal
  conditions represented by pddl-goal-(numeric-)fluent facts with the external
  pddl manager.
  @slot instance: pddl instance to which the respective goal conditions are
        added.
  Slots set automatically:
  @slot goal:  symbol corresponding to pddl-goal-fluent slot
  @slot state:
   - PENDING: The goal is not updated with the pddl manager yet.
   - DONE: The goal is updated with the pddl manager.
   - ERROR: The goal is not (or only partially) updated.
            Note that this leaves the set goal in an undefined state,
            hence it is advised to clear the goal before proceeding.
  @slot error: provide information on encountered errors.
"
  (slot instance (type SYMBOL))
  (slot goal (type SYMBOL))
  (slot state (type SYMBOL) (allowed-values PENDING DONE ERROR) (default PENDING))
  (slot error (type STRING))
)
