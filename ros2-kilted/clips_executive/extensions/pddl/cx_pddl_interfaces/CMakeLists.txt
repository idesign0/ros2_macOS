cmake_minimum_required(VERSION 3.16...3.28)
project(cx_pddl_interfaces)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(action_msgs REQUIRED)

set(MSGS
    "msg/Fluent.msg"
    "msg/Function.msg"
    "msg/FunctionEffect.msg"
    "msg/FluentEffect.msg"
    "msg/TimedPlanAction.msg"
    "msg/Action.msg"
    "msg/Object.msg"
    "msg/Predicate.msg")
set(SERVICES
    "srv/CheckActionCondition.srv"
    "srv/GetActionEffects.srv"
    "srv/GetActionNames.srv"
    "srv/GetPredicates.srv"
    "srv/GetTypeObjects.srv"
    "srv/AddPddlInstance.srv"
    "srv/AddObjects.srv"
    "srv/RemoveObjects.srv"
    "srv/RemoveFluents.srv"
    "srv/SetFunctions.srv"
    "srv/SetGoals.srv"
    "srv/ClearGoals.srv"
    "srv/GetFluents.srv"
    "srv/GetFunctions.srv"
    "srv/SetActionFilter.srv"
    "srv/SetFluentFilter.srv"
    "srv/SetObjectFilter.srv"
    "srv/CreateGoalInstance.srv"
    "srv/AddFluents.srv")
set(ACTIONS "action/PlanTemporal.action")

# set(ACTIONS

# )
rosidl_generate_interfaces(
  ${PROJECT_NAME}
  ${MSGS}
  ${SERVICES}
  ${ACTIONS}
  DEPENDENCIES
  builtin_interfaces
  action_msgs)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
