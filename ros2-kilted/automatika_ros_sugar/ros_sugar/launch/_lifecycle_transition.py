# This Action file is copied here from OSRF to support distributions prior to Iron
# Copyright 2022 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import functools

from typing import Iterable
from typing import List
from typing import Optional
from typing import Union


import launch
from launch import LaunchContext, SomeSubstitutionsType
from launch.action import Action
from launch.actions import EmitEvent, RegisterEventHandler
from launch.utilities import normalize_to_list_of_substitutions
from launch.utilities import perform_substitutions

from launch_ros.event_handlers import OnStateTransition
from launch_ros.events.lifecycle import ChangeState, StateTransition
from launch_ros.events.matchers import matches_node_name
from lifecycle_msgs.msg import Transition


class LifecycleTransition(Action):
    """An action that simplifies execution of lifecycle transitions."""

    transition_targets = {
        Transition.TRANSITION_CONFIGURE: {
            "start_state": "configuring",
            "goal_state": "inactive",
        },
        Transition.TRANSITION_CLEANUP: {
            "start_state": "cleaningup",
            "goal_state": "unconfigured",
        },
        Transition.TRANSITION_ACTIVATE: {
            "start_state": "activating",
            "goal_state": "active",
        },
        Transition.TRANSITION_DEACTIVATE: {
            "start_state": "deactivating",
            "goal_state": "inactive",
        },
        Transition.TRANSITION_UNCONFIGURED_SHUTDOWN: {
            "start_state": "shuttingdown",
            "goal_state": "finalized",
        },
        Transition.TRANSITION_INACTIVE_SHUTDOWN: {
            "start_state": "shuttingdown",
            "goal_state": "finalized",
        },
        Transition.TRANSITION_ACTIVE_SHUTDOWN: {
            "start_state": "shuttingdown",
            "goal_state": "finalized",
        },
    }

    def __init__(
        self,
        *,
        lifecycle_node_names: Iterable[SomeSubstitutionsType],
        transition_ids: Iterable[Union[int, SomeSubstitutionsType]],
        **kwargs,
    ) -> None:
        """
        Construct a LifecycleTransition action.

        The action will execute the passed in lifecycle transition for the
        lifecycle nodes with the indicated node names. The action will emit
        an event that triggers the first lifecycle transition of each node
        wait that the node reaches the transition goal and trigger the next
        transition in the list.
        You need to make sure, that the sequence of lifecyle transition you
        pass in is possible.

        :param lifecycle_node_names: The names of the lifecycle nodes to transition
        :param transitions_ids: The transitions to be executed.
        """
        super().__init__(**kwargs)
        if len(transition_ids) == 0:
            raise ValueError("No transition_ids provided.")

        if len(lifecycle_node_names) == 0:
            raise ValueError("No lifecycle_node_names provided.")

        self.__lifecycle_node_names = [
            normalize_to_list_of_substitutions(name) for name in lifecycle_node_names
        ]
        transition_ids = [
            str(transition_id) if isinstance(transition_id, int) else transition_id
            for transition_id in transition_ids
        ]
        self.__transition_ids = [
            normalize_to_list_of_substitutions(transition_id)
            for transition_id in transition_ids
        ]

        self.__event_handlers = {}
        self.__logger = launch.logging.get_logger(__name__)

    def _remove_event_handlers(
        self, context: LaunchContext, node_name: str, reason: str = None
    ):
        """Remove all consequent transitions if error occurs."""
        if reason is not None:
            self.__logger.info(
                f"Stopping transitions for {node_name} because '{reason}'"
            )

        for event_handler in self.__event_handlers[node_name]:
            # Unregister event handlers and ignore failures, as these are
            # already unregistered event handlers.
            try:
                context.unregister_event_handler(event_handler=event_handler)
            except ValueError:
                pass

    def execute(self, context: launch.LaunchContext) -> Optional[List[Action]]:
        """
        Execute the LifecycleTransition action.

        :return Returns a list of actions to be executed to achieve specified transitions.
          These are EventHandlers and EventEmitters for ChangeState and
          StateTransition events of the nodes indicated.
        """
        lifecycle_node_names = [
            perform_substitutions(context, name) for name in self.__lifecycle_node_names
        ]
        subs_transition_ids = [
            perform_substitutions(context, id_) for id_ in self.__transition_ids
        ]
        transition_ids = []
        for tid in subs_transition_ids:
            try:
                transition_ids.append(int(tid))
            except ValueError as e:
                raise ValueError(
                    f"expected integer for lifecycle transition, got {tid}"
                ) from e

        emit_actions = {}
        actions: List[Action] = []

        # Create EmitEvents for ChangeStates and store
        for name in lifecycle_node_names:
            own_emit_actions = []
            for tid in transition_ids:
                change_event = ChangeState(
                    lifecycle_node_matcher=matches_node_name(name), transition_id=tid
                )
                emit_action = EmitEvent(event=change_event)
                own_emit_actions.append(emit_action)
            emit_actions[name] = own_emit_actions
            self.__event_handlers[name] = []
        # Create Transition EventHandlers and Registration actions
        i = 1
        for tid in transition_ids:
            # Create Transition handler for all indicated nodes
            for node_name in lifecycle_node_names:
                states = self.transition_targets[tid]
                event_handler = None
                # For all transitions except the last, emit next ChangeState Event
                if i < len(transition_ids):
                    event_handler = OnStateTransition(
                        matcher=match_node_name_start_goal(
                            node_name, states["start_state"], states["goal_state"]
                        ),
                        entities=[emit_actions[node_name][i]],
                        handle_once=True,
                    )
                # For last transition emit Log message and remove untriggered error handlers
                else:
                    event_handler = OnStateTransition(
                        matcher=match_node_name_start_goal(
                            node_name, states["start_state"], states["goal_state"]
                        ),
                        entities=[
                            launch.actions.OpaqueFunction(
                                function=functools.partial(
                                    self._remove_event_handlers, node_name=node_name
                                )
                            ),
                        ],
                        handle_once=True,
                    )
                self.__event_handlers[node_name].append(event_handler)
                # Create register event handler action
                register_action = RegisterEventHandler(event_handler=event_handler)
                # Append to actions
                actions.append(register_action)
            # increment next ChangeState action by one
            i += 1
        # Create Error processing event handlers.
        for node_name in lifecycle_node_names:
            event_handler = launch.EventHandler(
                matcher=match_node_name_goal(node_name, "errorprocessing"),
                entities=[
                    launch.actions.OpaqueFunction(
                        function=functools.partial(
                            self._remove_event_handlers,
                            node_name=node_name,
                            reason="error occured during transitions",
                        )
                    )
                ],
                handle_once=True,
            )
            self.__event_handlers[node_name].append(event_handler)
            context.register_event_handler(event_handler=event_handler)

        # Add first Emit actions to actions
        for node_name in lifecycle_node_names:
            actions.append(emit_actions[node_name][0])

        return actions


def match_node_name_start_goal(node_name: str, start_state: str, goal_state: str):
    if not node_name.startswith("/"):
        node_name = f"/{node_name}"
    return lambda event: (
        isinstance(event, StateTransition)
        and (event.action.node_name == node_name)
        and (event.goal_state == goal_state)
        and (event.start_state == start_state)
    )


def match_node_name_goal(node_name: str, goal_state: str):
    if not node_name.startswith("/"):
        node_name = f"/{node_name}"
    return lambda event: (
        isinstance(event, StateTransition)
        and (event.action.node_name == node_name)
        and (event.goal_state == goal_state)
    )
