# Copyright (C) 2022 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math
import unittest
from gz.math8 import Intervald


class TestInterval(unittest.TestCase):

    def test_default_constructor(self):
        interval = Intervald()
        self.assertAlmostEqual(interval.left_value(), interval.right_value())

    def test_construct(self):
        kClosed = True

        interval = Intervald(0., kClosed, 1., not kClosed)
        self.assertAlmostEqual(interval.left_value(), 0.)
        self.assertTrue(interval.is_left_closed())
        self.assertAlmostEqual(interval.right_value(), 1.)
        self.assertFalse(interval.is_right_closed())

    def test_construction_helpers(self):
        openInterval = Intervald.open(0., 1.)
        self.assertAlmostEqual(openInterval.left_value(), 0.)
        self.assertFalse(openInterval.is_left_closed())
        self.assertAlmostEqual(openInterval.right_value(), 1.)
        self.assertFalse(openInterval.is_right_closed())

        leftClosedInterval = Intervald.left_closed(0., 1.)
        self.assertAlmostEqual(leftClosedInterval.left_value(), 0.)
        self.assertTrue(leftClosedInterval.is_left_closed())
        self.assertAlmostEqual(leftClosedInterval.right_value(), 1.)
        self.assertFalse(leftClosedInterval.is_right_closed())

        rightClosedInterval = Intervald.right_closed(0., 1.)
        self.assertAlmostEqual(rightClosedInterval.left_value(), 0.)
        self.assertFalse(rightClosedInterval.is_left_closed())
        self.assertAlmostEqual(rightClosedInterval.right_value(), 1.)
        self.assertTrue(rightClosedInterval.is_right_closed())

        closedInterval = Intervald.closed(0., 1.)
        self.assertAlmostEqual(closedInterval.left_value(), 0.)
        self.assertTrue(closedInterval.is_left_closed())
        self.assertAlmostEqual(closedInterval.right_value(), 1.)
        self.assertTrue(closedInterval.is_right_closed())

    def test_empty_interval(self):
        self.assertFalse(Intervald.open(0., 1.).empty())
        self.assertTrue(Intervald.open(0., 0.).empty())
        self.assertTrue(Intervald.left_closed(0., 0.).empty())
        self.assertTrue(Intervald.right_closed(0., 0.).empty())
        self.assertFalse(Intervald.closed(0., 0.).empty())
        self.assertTrue(Intervald.closed(1., 0.).empty())

    def test_interval_membership(self):
        emptyInterval = Intervald.open(0., 0.)
        self.assertFalse(emptyInterval.contains(0.))

        openInterval = Intervald.open(0., 1.)
        self.assertFalse(openInterval.contains(0.))
        self.assertTrue(openInterval.contains(0.5))
        self.assertFalse(openInterval.contains(1))

        leftClosedInterval = Intervald.left_closed(0., 1.)
        self.assertTrue(leftClosedInterval.contains(0.))
        self.assertTrue(leftClosedInterval.contains(0.5))
        self.assertFalse(leftClosedInterval.contains(1))

        rightClosedInterval = Intervald.right_closed(0., 1.)
        self.assertFalse(rightClosedInterval.contains(0.))
        self.assertTrue(rightClosedInterval.contains(0.5))
        self.assertTrue(rightClosedInterval.contains(1))

        closedInterval = Intervald.closed(0., 1.)
        self.assertTrue(closedInterval.contains(0.))
        self.assertTrue(closedInterval.contains(0.5))
        self.assertTrue(closedInterval.contains(1))

        degenerateInterval = Intervald.closed(0., 0.)
        self.assertTrue(degenerateInterval.contains(0.))

    def test_interval_subset(self):
        openInterval = Intervald.open(0., 1.)
        self.assertFalse(openInterval.contains(Intervald.open(0., 0.)))
        self.assertFalse(openInterval.contains(Intervald.closed(-1., 0.)))
        self.assertFalse(openInterval.contains(Intervald.closed(1., 2.)))
        self.assertFalse(openInterval.contains(Intervald.open(0.5, 1.5)))
        self.assertFalse(openInterval.contains(Intervald.open(-0.5, 0.5)))
        self.assertTrue(openInterval.contains(Intervald.open(0.25, 0.75)))
        self.assertFalse(openInterval.contains(Intervald.closed(0., 1.)))

        closedInterval = Intervald.closed(0., 1.)
        self.assertFalse(closedInterval.contains(Intervald.open(0., 0.)))
        self.assertTrue(closedInterval.contains(Intervald.closed(0., 0.)))
        self.assertFalse(closedInterval.contains(Intervald.closed(0.5, 1.5)))
        self.assertFalse(closedInterval.contains(Intervald.closed(-0.5, 0.5)))
        self.assertTrue(closedInterval.contains(Intervald.closed(0.25, 0.75)))
        self.assertTrue(closedInterval.contains(Intervald.open(0., 1.)))

    def test_interval_equality(self):
        self.assertNotEqual(Intervald.open(0., 0.), Intervald.open(0., 0.))
        self.assertEqual(Intervald.closed(0., 0.), Intervald.closed(0., 0.))
        self.assertNotEqual(Intervald.open(0., 1.), Intervald.closed(0., 1.))
        self.assertNotEqual(
            Intervald.open(0., 1.), Intervald.left_closed(0., 1.))
        self.assertNotEqual(
          Intervald.open(0., 1.), Intervald.right_closed(0., 1.))
        self.assertNotEqual(
          Intervald.closed(0., 1.), Intervald.left_closed(0., 1.))
        self.assertNotEqual(
          Intervald.closed(0., 1.), Intervald.right_closed(0., 1.))

    def test_interval_intersection(self):
        openInterval = Intervald.open(0., 1.)
        self.assertFalse(openInterval.intersects(Intervald.open(0.5, 0.5)))
        self.assertTrue(openInterval.intersects(Intervald.open(0.5, 1.5)))
        self.assertTrue(openInterval.intersects(Intervald.open(-0.5, 0.5)))
        self.assertFalse(openInterval.intersects(Intervald.closed(1., 1.)))
        self.assertTrue(openInterval.intersects(Intervald.closed(0.5, 0.5)))
        self.assertFalse(openInterval.intersects(Intervald.open(1., 2.)))
        self.assertFalse(openInterval.intersects(Intervald.open(-1., 0.)))
        self.assertFalse(openInterval.intersects(Intervald.left_closed(1., 2.)))
        self.assertFalse(openInterval.intersects(Intervald.right_closed(-1., 0.)))

        closedInterval = Intervald.closed(0., 1.)
        self.assertFalse(closedInterval.intersects(Intervald.open(1., 1.)))
        self.assertTrue(closedInterval.intersects(Intervald.closed(0.5, 1.5)))
        self.assertTrue(closedInterval.intersects(Intervald.closed(-0.5, 0.5)))
        self.assertFalse(closedInterval.intersects(Intervald.closed(1.5, 2.5)))
        self.assertFalse(closedInterval.intersects(Intervald.closed(-1.5, -0.5)))
        self.assertFalse(closedInterval.intersects(Intervald.open(1., 2.)))
        self.assertFalse(closedInterval.intersects(Intervald.open(-1., 0.)))
        self.assertTrue(closedInterval.intersects(Intervald.left_closed(1., 2.)))
        self.assertTrue(closedInterval.intersects(Intervald.right_closed(-1., 0.)))


if __name__ == '__main__':
    unittest.main()
