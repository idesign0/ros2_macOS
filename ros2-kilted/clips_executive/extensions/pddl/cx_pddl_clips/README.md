# cx_pddl_clips

Augment CLIPS-based reasoning with [PDDL](https://planning.wiki/guide/whatis/pddl).

As PDDL is an expressive formalism, this module relies on an external pddl manager to do the heavy lifting.

## Prerequisites

1. A running instance of the external pddl manager node
2. The [RosMsgsPlugin](https://planning.wiki/guide/whatis/pddl)
3. (for planning:) [Generated CLIPS plugins](https://fawkesrobotics.github.io/ros2-clips-executive/clips_executive/plugins/ros_msgs_plugin.html) `cx::CXPDDLMsgsPlanTemporalPlugin` and `cx::CXPDDLMsgsTimedPlanActionPlugin`
4. Open the respective ROS interfaces

## Basic Usage
1. Store the information on the external node using a `pddl-manager` fact.
2. Load pddl instances using `pddl-instance` facts.
3. Maintain up-to-date information of a current situation modeled via `pddl-fluent` and `pddl-numeric-fluent` facts.
  - Fetch the initial situation using `pddl-get-fluents` and `pddl-get-numeric-fluents` facts.
  - Update the situation using `pending-pddl-fluent` and `pending-pddl-numeric-fluent` facts.
  - Add new objects using `pending-pddl-object` facts.
4. Specify desired future states through pddl goals
 - Describe goals using `pddl-goal-fluent` and `pddl-goal-numeric-fluent` facts.
 - Register the goal uisng the `pddl-set-goals` fact.
 - Clear goals using the `pddl-clear-goals` fact.
5. Obtain a viable plan using the `pddl-planner-call` fact.
 - Plans consist of `pddl-action` facts
6. Inspect actions to ensure smooth execution
 - Check whether an action is viable for execution by checking it's precondition (start conditions).
 - Retrieve the projected effects.
 - TBD
