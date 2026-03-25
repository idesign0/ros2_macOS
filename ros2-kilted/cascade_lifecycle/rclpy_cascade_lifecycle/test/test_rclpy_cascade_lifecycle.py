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

import unittest

import lifecycle_msgs.msg
import rclpy
from rclpy_cascade_lifecycle.cascade_lifecycle_node import CascadeLifecycleNode


class NodeTest(CascadeLifecycleNode):

    def __init__(self, name, ns=''):
        super().__init__(name, ns)
        self.get_logger().info('NodeTest created')
        self._my_state = lifecycle_msgs.msg.State.PRIMARY_STATE_UNCONFIGURED

    def on_configure(self, state):
        self.get_logger().info('NodeTest on_configure')
        self._my_state = lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE

        return super().on_configure(state)

    def on_activate(self, state):
        self.get_logger().info('NodeTest on_activate')
        self._my_state = lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE

        return super().on_activate(state)

    def on_deactivate(self, state):
        self.get_logger().info('NodeTest on_deactivate')
        self._my_state = lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE

        return super().on_deactivate(state)

    def on_cleanup(self, state):
        self.get_logger().info('NodeTest on_cleanup')
        self._my_state = lifecycle_msgs.msg.State.PRIMARY_STATE_UNCONFIGURED

        return super().on_cleanup(state)

    def on_shutdown(self, state):
        self.get_logger().info('NodeTest on_shutdown')
        self._my_state = lifecycle_msgs.msg.State.PRIMARY_STATE_FINALIZED

        return super().on_shutdown(state)

    def get_state(self):
        return self._my_state


class TestRclpyCascadeLifecycle(unittest.TestCase):

    def setUp(self) -> None:
        """Set up before each test method."""
        super().setUp()

        rclpy.init()

    def tearDown(self) -> None:
        """Tear down after each test method."""
        rclpy.shutdown()

        super().tearDown()

    def test_activations_managing_basic(self):
        node_a = CascadeLifecycleNode('test1_node_A')
        node_b = CascadeLifecycleNode('test1_node_B')
        node_c = CascadeLifecycleNode('test1_node_C')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)
        executor.add_node(node_c)

        node_a.add_activation('test1_node_B')
        node_a.add_activation('test1_node_C')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 2)
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(len(node_b.get_activators()), 1)
        self.assertEqual(node_c.get_activations(), [])
        self.assertEqual(len(node_c.get_activators()), 1)

    def test_activations_managing_late_joining(self):
        node_a = CascadeLifecycleNode('test2_node_A')
        node_b = CascadeLifecycleNode('test2_node_B')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test2_node_B')
        node_a.add_activation('test2_node_C')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 2)
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(len(node_b.get_activators()), 1)

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        node_b2 = CascadeLifecycleNode('test2_node_B')
        node_c = CascadeLifecycleNode('test2_node_C')
        executor.add_node(node_b2)
        executor.add_node(node_c)

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 2)
        self.assertEqual(node_b2.get_activations(), [])
        self.assertEqual(len(node_b2.get_activators()), 1)
        self.assertEqual(node_c.get_activations(), [])
        self.assertEqual(len(node_c.get_activators()), 1)

    def test_activations_chained(self):
        node_a = CascadeLifecycleNode('test3_node_A')
        node_b = CascadeLifecycleNode('test3_node_B')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test3_node_B')
        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(len(node_b.get_activators()), 1)

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test3_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_UNCONFIGURED)

        node_a.trigger_configure()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test3_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test3_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE)

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test3_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')

    def test_fast_change(self):
        node_a = CascadeLifecycleNode('test4_node_A')
        node_b = CascadeLifecycleNode('test4_node_B')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test4_node_B')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertNotEqual(node_b.get_activators(), [])
        self.assertEqual(len(node_b.get_activations()), 0)
        self.assertNotEqual(node_b.get_activators_state(), [])

        node_a.trigger_configure()
        node_a.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')

    def test_activators_disappearance(self):
        node_a = CascadeLifecycleNode('test5_node_A')
        node_b = CascadeLifecycleNode('test5_node_B')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test5_node_B')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertNotEqual(node_b.get_activators(), [])
        self.assertEqual(len(node_b.get_activations()), 0)
        self.assertNotEqual(node_b.get_activators_state(), [])

        node_a.trigger_configure()
        node_a.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')

        node_a = None
        del node_a

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(len(node_b.get_activations()), 0)

    def test_activators_disappearance_inter(self):
        node_a = CascadeLifecycleNode('test6_node_A')
        node_b = CascadeLifecycleNode('test6_node_B')
        node_c = CascadeLifecycleNode('test6_node_C')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)
        executor.add_node(node_c)

        node_a.trigger_configure()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'unconfigured')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        node_a.trigger_activate()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'unconfigured')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        node_a.add_activation('test6_node_B')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        node_a.remove_activation('test6_node_B')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        node_a.add_activation('test6_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.remove_activation('test6_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.add_activation('test6_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_c.add_activation('test6_node_B')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_c.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.remove_activation('test6_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_c.remove_activation('test6_node_B')
        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(node_a.get_activations(), [])
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(node_b.get_activators(), [])
        self.assertEqual(node_c.get_activations(), [])
        self.assertEqual(node_c.get_activators(), [])

        node_a.add_activation('test6_node_C')
        node_b.add_activation('test6_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()
        node_b.trigger_activate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_c.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_c.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_b.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

    def test_inheritance(self):
        node_1 = NodeTest('test7_node_1')
        node_2 = NodeTest('test7_node_2')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_1)
        executor.add_node(node_2)

        node_1.add_activation('test7_node_2')
        node_1.trigger_configure()

        start = node_1.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_1.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_1._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_2._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_1.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)
        self.assertEqual(node_2.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

        node_1.trigger_activate()

        start = node_1.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_1.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_1._state_machine.current_state[1], 'active')
        self.assertEqual(node_2._state_machine.current_state[1], 'active')
        self.assertEqual(node_1.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE)
        self.assertEqual(node_2.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE)

        node_1.trigger_deactivate()

        start = node_1.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_1.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_1._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_2._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_1.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)
        self.assertEqual(node_2.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

    def test_activations_managing_basic_with_namespace(self):
        node_a = CascadeLifecycleNode('test8_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test8_node_B', 'test_ns')
        node_c = CascadeLifecycleNode('test8_node_C', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)
        executor.add_node(node_c)

        node_a.add_activation('test8_node_B')
        node_a.add_activation('test8_node_C')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 2)
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(len(node_b.get_activators()), 1)
        self.assertEqual(node_c.get_activations(), [])
        self.assertEqual(len(node_c.get_activators()), 1)

    # def test_activations_managing_late_joining_with_namespace(self):
        node_a = CascadeLifecycleNode('test9_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test9_node_B', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test9_node_B')
        node_a.add_activation('test9_node_C')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 2)
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(len(node_b.get_activators()), 1)

        node_b = None
        del node_b

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        node_b2 = CascadeLifecycleNode('test9_node_B', 'test_ns')
        node_c = CascadeLifecycleNode('test9_node_C', 'test_ns')
        executor.add_node(node_b2)
        executor.add_node(node_c)

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 2)
        self.assertEqual(node_b2.get_activations(), [])
        self.assertEqual(len(node_b2.get_activators()), 1)
        self.assertEqual(node_c.get_activations(), [])
        self.assertEqual(len(node_c.get_activators()), 1)

    def test_activations_chained_with_namespace(self):
        node_a = CascadeLifecycleNode('test10_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test10_node_B', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test10_node_B')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(len(node_b.get_activators()), 1)

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test10_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_UNCONFIGURED)

        node_a.trigger_configure()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test10_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test10_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE)

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators_state(), {})
        self.assertNotEqual(node_b.get_activators_state(), {})
        self.assertEqual(len(node_b.get_activators_state()), 1)
        self.assertEqual(next(iter(node_b.get_activators_state().items()))[0], 'test10_node_A')
        self.assertEqual(list(node_b.get_activators_state().values())[0],
                         lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')

    def test_multiple_activations_chained_with_namespace(self):
        node_a = CascadeLifecycleNode('test11_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test11_node_B', 'test_ns')
        node_c = CascadeLifecycleNode('test11_node_C', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)
        executor.add_node(node_c)

        node_a.add_activation('test11_node_C')
        node_b.add_activation('test11_node_C')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertEqual(node_b.get_activators(), [])
        self.assertEqual(len(node_b.get_activations()), 1)
        self.assertEqual(node_c.get_activations(), [])
        self.assertEqual(len(node_c.get_activators()), 2)

        node_a.trigger_configure()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'unconfigured')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_b.trigger_configure()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_b.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_b.trigger_deactivate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()
        node_b.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_b.trigger_deactivate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_c.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.remove_activation('test11_node_C')
        node_b.remove_activation('test11_node_C')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_c.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

    def test_fast_change_with_namespace(self):
        node_a = CascadeLifecycleNode('test12_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test12_node_B', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test12_node_B')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertNotEqual(node_b.get_activators(), [])
        self.assertEqual(len(node_b.get_activations()), 0)
        self.assertNotEqual(node_b.get_activators_state(), [])

        node_a.trigger_configure()
        node_a.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')

    def test_activators_disappearance_with_namespace(self):
        node_a = CascadeLifecycleNode('test13_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test13_node_B', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)

        node_a.add_activation('test13_node_B')

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(len(node_a.get_activations()), 1)
        self.assertNotEqual(node_b.get_activators(), [])
        self.assertEqual(len(node_b.get_activations()), 0)
        self.assertNotEqual(node_b.get_activators_state(), [])

        node_a.trigger_configure()
        node_a.trigger_activate()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')

        node_a = None
        del node_a

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(len(node_b.get_activations()), 0)

    def test_activators_disappearance_inter_with_namespace(self):
        node_a = CascadeLifecycleNode('test14_node_A', 'test_ns')
        node_b = CascadeLifecycleNode('test14_node_B', 'test_ns')
        node_c = CascadeLifecycleNode('test14_node_C', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_a)
        executor.add_node(node_b)
        executor.add_node(node_c)

        node_a.trigger_configure()

        start = node_a.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'unconfigured')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        while (node_a.get_clock().now() - start) < duration:
            executor.spin_once()

        node_a.trigger_activate()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'unconfigured')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        node_a.add_activation('test14_node_B')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        node_a.remove_activation('test14_node_B')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'unconfigured')

        node_a.add_activation('test14_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.remove_activation('test14_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.add_activation('test14_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_c.add_activation('test14_node_B')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_c.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.remove_activation('test14_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_c.remove_activation('test14_node_B')
        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        self.assertEqual(node_a.get_activators(), [])
        self.assertEqual(node_a.get_activations(), [])
        self.assertEqual(node_b.get_activators(), [])
        self.assertEqual(node_b.get_activations(), [])
        self.assertEqual(node_c.get_activators(), [])
        self.assertEqual(node_c.get_activations(), [])

        node_a.add_activation('test14_node_C')
        node_b.add_activation('test14_node_C')

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

        node_a.trigger_activate()
        node_b.trigger_activate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_c.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'active')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_a.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_c.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'active')
        self.assertEqual(node_c._state_machine.current_state[1], 'active')

        node_b.trigger_deactivate()

        start = node_b.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_b.get_clock().now() - start < duration):
            executor.spin_once()

        self.assertEqual(node_a._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_b._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_c._state_machine.current_state[1], 'inactive')

    def test_inheritance_with_namespace(self):
        node_1 = NodeTest('test15_node_1', 'test_ns')
        node_2 = NodeTest('test15_node_2', 'test_ns')

        executor = rclpy.executors.SingleThreadedExecutor()
        executor.add_node(node_1)
        executor.add_node(node_2)

        node_1.add_activation('test15_node_2')
        node_1.trigger_configure()

        start = node_1.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_1.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_1._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_2._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_1.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)
        self.assertEqual(node_2.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)

        node_1.trigger_activate()

        start = node_1.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_1.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_1._state_machine.current_state[1], 'active')
        self.assertEqual(node_2._state_machine.current_state[1], 'active')
        self.assertEqual(node_1.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE)
        self.assertEqual(node_2.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_ACTIVE)

        node_1.trigger_deactivate()

        start = node_1.get_clock().now()
        duration = rclpy.time.Duration(seconds=1.0)

        while (node_1.get_clock().now() - start) < duration:
            executor.spin_once()

        self.assertEqual(node_1._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_2._state_machine.current_state[1], 'inactive')
        self.assertEqual(node_1.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)
        self.assertEqual(node_2.get_state(), lifecycle_msgs.msg.State.PRIMARY_STATE_INACTIVE)
