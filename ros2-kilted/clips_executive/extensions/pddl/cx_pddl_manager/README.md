# pddl_manager

PDDL manager node to create PDDL instances and to interact with it via ROS interfaces.

## Prerequisites
This package has indexed dependencies as listed in the `package.xml` file that can be installed via `rosdep`.
Aside from that it additionally requires the [unified-planning](https://unified-planning.readthedocs.io/en/latest/) (source code on [GitHub](https://github.com/aiplan4eu/unified-planning)) python framework. It can be installed via pip:
```bash
pip install unified-planning
```

## Usage
Run the provided lifecycle node using the provided launch file to automatically configure the node:
```bash
ros2 launch pddl_manager pddl_manager.launch.py
```
Alternatively, the unconfigured node can be used directly:
```bash
ros2 run pddl_manager pddl_manager.py
```

## Features
The `pddl_problem_manager` node offers various ROS interfaces.
### Services
 - **/add_pddl_instance**: add a pddl instance and gives it a name.
 - **/add_fluent**: add a regular fluent to a pddl instance.
 - **/add_fluents**: add multiple fluents to a pddl instance (by setting their boolean value to true).
 - **/rm_fluents**: remove fluents (by setting the boolean value to false).
 - **/check_action_precondition**: check the precondition of a grounded action. This only checks at-start effects of temporal actions for now.
 - **/add_objects**: add objects to a pddl instance.
 - **/rm_objects**: remove objects from a pddl instance. This does currently not work as unified-planning does not support the deletion of objects!
 - **/get_action_effects**: gets all start and end effects of a grounded pddl action. Note that numeric fluents are currently not handled properly with this
 - **/get_fluents**: gets all fluents of a given pddl instance.
 - **/get_functions**: gets all current function values of a given pddl instance
 - **/set_functions**: set functions of the pddl instance to set values.
 - **/set_goals**: adds fluents and function values as goal specification. This service is currently really basic and does not allow for any complex goal conditions or fluent inequality.
 - **/clear_goals**: clear all goals from the given pddl instance.
 - **/check_action_precondition**: ground an action and check it's precondition against the current values in a given pddl instance. If it is not satisfied, it also provides the unsatisfied conditions.
 - **/get_action_effects**: ground an action and provide it`s start and and effect.
### Actions
 - **/pddl_plan**: plan the given pddl instance using nextFLAP.
