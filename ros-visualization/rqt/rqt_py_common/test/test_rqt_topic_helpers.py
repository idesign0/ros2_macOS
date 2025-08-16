#!/usr/bin/env python3

# Software License Agreement (BSD License)
#
# Copyright (c) 2018, PickNik Robotics
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of PickNik Robotics nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Author: Michael Lautman

import unittest


class TestTopicHelpers(unittest.TestCase):  # noqa: D101

    def test_get_slot_type(self):  # noqa: D102
        from rqt_py_common.topic_helpers import get_slot_type
        from rqt_py_common.message_helpers import get_message_class
        from rqt_py_common_test_interfaces.msg import ArrayVal
        path = 'vals/floats'
        message_class = ArrayVal
        message_type, is_array = get_slot_type(message_class, path)
        self.assertTrue(is_array)
        self.assertEqual(message_type, float)

        path = '/vals'
        message_class = ArrayVal
        message_type, is_array = get_slot_type(message_class, path)
        self.assertTrue(is_array)
        self.assertEqual(message_type, get_message_class('rqt_py_common_test_interfaces/Val'))

    def test_get_field_type(self):  # noqa: D102
        from rqt_py_common.topic_helpers import _get_field_type
        from rqt_py_common_test_interfaces.msg import ArrayVal, Val
        topic_names_and_types = [
            ('/example', ['rqt_py_common_test_interfaces/Val']),
            ('/example/vals', ['rqt_py_common_test_interfaces/Val']),
            ('/example_topic', ['rqt_py_common_test_interfaces/Val']),
            ('/example/topic', ['rqt_py_common_test_interfaces/ArrayVal']),
        ]

        target = '/example/topic/vals/floats'
        target_class, is_array = _get_field_type(topic_names_and_types, target)
        self.assertTrue(is_array)
        self.assertEqual(target_class, float)

        target = '/example/topic'
        target_class, is_array = _get_field_type(topic_names_and_types, target)
        self.assertFalse(is_array)
        self.assertEqual(target_class, ArrayVal)

        target = '/example/topic/vals'
        target_class, is_array = _get_field_type(topic_names_and_types, target)
        self.assertTrue(is_array)
        self.assertEqual(target_class, Val)

        target = '/example/topic/vals/bad_value'
        target_class, is_array = _get_field_type(topic_names_and_types, target)
        self.assertFalse(is_array)
        self.assertEqual(target_class, None)
