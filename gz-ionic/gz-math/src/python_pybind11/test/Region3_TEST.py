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

import unittest

from gz.math8 import Intervald, Region3d, Vector3d


class TestRegion3(unittest.TestCase):

    def test_default_constructor(self):
        region = Region3d()
        self.assertTrue(region.ix().empty())
        self.assertTrue(region.iy().empty())
        self.assertTrue(region.iz().empty())

    def test_constructor(self):
        region = Region3d(
            Intervald.open(0., 1.),
            Intervald.closed(-1., 1.),
            Intervald.open(-1., 0.))
        self.assertEqual(region.ix(), Intervald.open(0., 1.))
        self.assertEqual(region.iy(), Intervald.closed(-1., 1.))
        self.assertEqual(region.iz(), Intervald.open(-1., 0.))

    def test_construction_helpers(self):
        openRegion = Region3d.open(0., 0., 0., 1., 1., 1.)
        self.assertEqual(openRegion.ix(), Intervald.open(0., 1.))
        self.assertEqual(openRegion.iy(), Intervald.open(0., 1.))
        self.assertEqual(openRegion.iz(), Intervald.open(0., 1.))
        closedRegion = Region3d.closed(0., 0., 0., 1., 1., 1.)
        self.assertEqual(closedRegion.ix(), Intervald.closed(0., 1.))
        self.assertEqual(closedRegion.iy(), Intervald.closed(0., 1.))
        self.assertEqual(closedRegion.iz(), Intervald.closed(0., 1.))

    def test_empty_region(self):
        self.assertFalse(Region3d.open(0., 0., 0., 1., 1., 1.).empty())
        self.assertTrue(Region3d.open(0., 0., 0., 0., 0., 0.).empty())
        self.assertTrue(Region3d.open(0., 0., 0., 0., 1., 1.).empty())
        self.assertTrue(Region3d.open(0., 0., 0., 1., 0., 1.).empty())
        self.assertTrue(Region3d.open(0., 0., 0., 1., 1., 0.).empty())
        self.assertFalse(Region3d.closed(0., 0., 0., 0., 0., 0.).empty())
        self.assertTrue(Region3d.closed(1., 1., 1., 0., 0., 0.).empty())

    def test_region_membership(self):
        openRegion = Region3d.open(0., 0., 0., 1., 1., 1.)
        self.assertFalse(openRegion.contains(Vector3d(0., 0., 0.)))
        self.assertTrue(openRegion.contains(Vector3d(0.5, 0.5, 0.5)))
        self.assertFalse(openRegion.contains(Vector3d(1., 1., 1.)))
        closedRegion = Region3d.closed(0., 0., 0., 1., 1., 1.)
        self.assertTrue(closedRegion.contains(Vector3d(0., 0., 0.)))
        self.assertTrue(closedRegion.contains(Vector3d(0.5, 0.5, 0.5)))
        self.assertTrue(closedRegion.contains(Vector3d(1., 1., 1.)))

    def test_region_subset(self):
        openRegion = Region3d.open(0., 0., 0., 1., 1., 1.)
        self.assertTrue(openRegion.contains(
          Region3d.open(0.25, 0.25, 0.25,
                        0.75, 0.75, 0.75)))
        self.assertFalse(openRegion.contains(
          Region3d.open(-1., 0.25, 0.25,
                        0., 0.75, 0.75)))
        self.assertFalse(openRegion.contains(
          Region3d.open(0.25, -1., 0.25,
                        0.75,  0., 0.75)))
        self.assertFalse(openRegion.contains(
          Region3d.open(0.25, 0.25, -1.,
                        0.75, 0.75,  0.)))
        self.assertFalse(openRegion.contains(
          Region3d.closed(0., 0., 0.,
                          1., 1., 1.)))

        closedRegion = Region3d.closed(0., 0., 0., 1., 1., 1.)
        self.assertTrue(closedRegion.contains(
          Region3d.closed(0., 0., 0., 1., 1., 1.)))
        self.assertTrue(closedRegion.contains(
          Region3d.closed(0., 0., 0., 0., 0., 0.)))

    def test_region_equality(self):
        self.assertNotEqual(
            Region3d.open(0., 0., 0., 0., 0., 0.),
            Region3d.open(0., 0., 0., 0., 0., 0.))
        self.assertEqual(
            Region3d.closed(0., 0., 0., 0., 0., 0.),
            Region3d.closed(0., 0., 0., 0., 0., 0.))
        self.assertNotEqual(
            Region3d.open(0., 0., 0., 1., 1., 1.),
            Region3d.closed(0., 0., 0., 1., 1., 1.))

    def test_region_intersection(self):
        region = Region3d.open(0., 0., 0., 1., 1., 1.)
        self.assertTrue(region.intersects(
          Region3d.open(0.5, 0.5, 0.5, 1.5, 1.5, 1.5)))
        self.assertTrue(region.intersects(
          Region3d.open(-0.5, -0.5, -0.5, 0.5, 0.5, 0.5)))
        self.assertFalse(region.intersects(
          Region3d.open(1., 1., 1., 2., 2., 2.)))
        self.assertFalse(region.intersects(
          Region3d.open(-1., -1., -1., 0., 0., 0.)))


if __name__ == '__main__':
    unittest.main()
