# Copyright 2024 Juan Carlos Manzanares Serrano
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


import cascade_lifecycle_msgs.msg
import lifecycle_msgs.msg

from rclpy.lifecycle import LifecycleNode, State, \
    TransitionCallbackReturn
from rclpy.qos import QoSDurabilityPolicy, QoSHistoryPolicy, \
    QoSProfile, QoSReliabilityPolicy


class CascadeLifecycleNode(LifecycleNode):

    def __init__(self, node_name='CascadeLifecycleNode', ns='', options=None):
        super().__init__(node_name, namespace=ns)

        qos_profile = QoSProfile(
            history=QoSHistoryPolicy.KEEP_ALL,
            depth=1000,
            reliability=QoSReliabilityPolicy.RELIABLE,
            durability=QoSDurabilityPolicy.TRANSIENT_LOCAL)

        self._allow_duplicate_names = True

        self.declare_parameter('allow_duplicate_names', True)
        self._allow_duplicate_names = self.get_parameter('allow_duplicate_names').get_parameter_value().bool_value

        self._node_name = node_name
        self._namespace = ns

        if (not self._allow_duplicate_names):
            self._activations_pub = self.create_publisher(
                cascade_lifecycle_msgs.msg.Activation,
                'cascade_lifecycle_activations',
                100)
            
            self._activations_sub = self.create_subscription(
                cascade_lifecycle_msgs.msg.Activation,
                'cascade_lifecycle_activations',
                self.activations_callback,
                100)
        else: 
            self._activations_pub = self.create_publisher(
                cascade_lifecycle_msgs.msg.Activation,
                'cascade_lifecycle_activations',
                qos_profile)

            self._activations_sub = self.create_subscription(
                cascade_lifecycle_msgs.msg.Activation,
                'cascade_lifecycle_activations',
                self.activations_callback,
                qos_profile)

        self._states_pub = self.create_publisher(
            cascade_lifecycle_msgs.msg.State,
            'cascade_lifecycle_states',
            100)

        self._states_sub = self.create_subscription(
            cascade_lifecycle_msgs.msg.State,
            'cascade_lifecycle_states',
            self.states_callback,
            100)

        self._timer = self.create_timer(0.5, self.timer_callback)

        if (not self._allow_duplicate_names):
            self._timer_responses = self.create_timer(1, self.timer_responses_callback)

        self._activators = []
        self._activations = []
        self._activators_state = {}
        self._op_pending = []

    def get_activators(self):
        return self._activators

    def get_activations(self):
        return self._activations

    def get_activators_state(self):
        return self._activators_state

    def activations_callback(self, msg):
        if (msg.operation_type == cascade_lifecycle_msgs.msg.Activation.ADD):
            if (msg.activation == self._node_name):
                self._activators.append(msg.activator)

                if (msg.activator not in self._activators_state):
                    self._activators_state[msg.activator] = \
                        lifecycle_msgs.msg.State.PRIMARY_STATE_UNKNOWN
                    
                if (not self._allow_duplicate_names):
                    response = msg
                    response.operation_type = cascade_lifecycle_msgs.msg.Activation.CONFIRM_ADD
                    self._activations_pub.publish(response)
        elif (msg.operation_type ==
              cascade_lifecycle_msgs.msg.Activation.REMOVE):
            if (msg.activation == self._node_name and
                    msg.activator in self._activators):
                remover_state = self._activators_state[msg.activator]
                self._activators.remove(msg.activator)

                if (msg.activator in self._activators_state):
                    self._activators_state.pop(msg.activator)

                if (remover_state ==
                        lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE):
                    any_other_active = False
                    for activator in self._activators_state:
                        any_other_active = any_other_active or \
                            self._activators_state[activator] == \
                            lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE

                    if (not any_other_active):
                        self.trigger_deactivate()

                if (not self._allow_duplicate_names):
                    response = msg
                    response.operation_type = cascade_lifecycle_msgs.msg.Activation.CONFIRM_REMOVE
                    self._activations_pub.publish(response)
        elif (msg.operation_type ==
              cascade_lifecycle_msgs.msg.Activation.CONFIRM_ADD):
            for op in self._op_pending:
                if (op['operation_type'] ==
                        cascade_lifecycle_msgs.msg.Activation.ADD and
                        op['activation'] == msg.activator and
                        op['activator'] == msg.activation):
                    self._op_pending.remove(op)
        elif (msg.operation_type ==
              cascade_lifecycle_msgs.msg.Activation.CONFIRM_REMOVE):
            for op in self._op_pending:
                if (op['operation_type'] ==
                        cascade_lifecycle_msgs.msg.Activation.REMOVE and
                        op['activation'] == msg.activator and
                        op['activator'] == msg.activation):
                    self._op_pending.remove(op)

    def states_callback(self, msg):
        if (msg.node_name in self._activators_state and
                msg.node_name != self._node_name):
            if (self._activators_state[msg.node_name] != msg.state):
                self._activators_state[msg.node_name] = msg.state
                self.update_state()

    def add_activation(self, node_name):
        if (node_name != self._node_name):
            msg = cascade_lifecycle_msgs.msg.Activation()
            msg.operation_type = cascade_lifecycle_msgs.msg.Activation.ADD
            msg.activation = node_name
            msg.activator = self._node_name

            self._activations.append(node_name)

            if (not self._allow_duplicate_names):
                self._op_pending.append(msg)

            self._activations_pub.publish(msg)

    def remove_activation(self, node_name):
        if (node_name != self._node_name):
            msg = cascade_lifecycle_msgs.msg.Activation()
            msg.operation_type = cascade_lifecycle_msgs.msg.Activation.REMOVE
            msg.activation = node_name
            msg.activator = self._node_name

            self._activations.remove(node_name)

            if (not self._allow_duplicate_names):
                self._op_pending.remove(msg)

            self._activations_pub.publish(msg)

    def clear_activation(self):
        for activator in self._activators:
            self.remove_activation(activator)

    def on_configure(self, state: State) -> TransitionCallbackReturn:
        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name
        msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE

        self._states_pub.publish(msg)

        return super().on_configure(state)

    def on_cleanup(self, state: State) -> TransitionCallbackReturn:
        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name
        msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_UNCONFIGURED

        self._states_pub.publish(msg)

        return super().on_cleanup(state)

    def on_shutdown(self, state: State) -> TransitionCallbackReturn:
        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name
        msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_FINALIZED

        self._states_pub.publish(msg)

        return super().on_shutdown(state)

    def on_activate(self, state: State) -> TransitionCallbackReturn:
        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name
        msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE

        self._states_pub.publish(msg)

        return super().on_activate(state)

    def on_deactivate(self, state: State) -> TransitionCallbackReturn:
        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name
        msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE

        self._states_pub.publish(msg)

        return super().on_deactivate(state)

    def on_error(self, state: State) -> TransitionCallbackReturn:
        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name
        msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_FINALIZED

        self._states_pub.publish(msg)

        return super().on_error(state)

    def update_state(self):
        parent_inactive = False
        parent_active = False

        for activator in self._activators_state:
            parent_inactive = parent_inactive or \
                self._activators_state[activator] == \
                lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE
            parent_active = parent_active or \
                self._activators_state[activator] == \
                lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE

        current_state = self._state_machine.current_state[1]

        if (current_state == 'unknown'):
            if (parent_active or parent_inactive):
                self.trigger_configure()
        elif (current_state == 'unconfigured'):
            if (parent_active or parent_inactive):
                self.trigger_configure()
        elif (current_state == 'inactive'):
            if (parent_active):
                self.trigger_activate()
        elif (current_state == 'active'):
            if (not parent_active and parent_inactive):
                self.trigger_deactivate()

    def timer_callback(self):
        nodes = self.get_node_names()
        ns = self._namespace
        current_state = self._state_machine.current_state[1]
        activators_to_remove = []

        if (ns != '/'):
            ns = ns + '/'

        for node_name in self._activators:
            if node_name not in nodes:
                activators_to_remove.append(node_name)

                if current_state == self._activators_state[node_name]:
                    self.update_state()

                self._activators_state.pop(node_name)

        for node_name in activators_to_remove:
            self._activators.remove(node_name)

        msg = cascade_lifecycle_msgs.msg.State()
        msg.node_name = self._node_name

        if (current_state == 'unknown'):
            msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_UNKNOWN
        elif (current_state == 'unconfigured'):
            msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_UNCONFIGURED
        elif (current_state == 'inactive'):
            msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE
        elif (current_state == 'active'):
            msg.state = lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE

        self._states_pub.publish(msg)

        self.update_state()

    def timer_responses_callback(self):
        for msg in self._op_pending:
            self._activations_pub.publish(msg)
