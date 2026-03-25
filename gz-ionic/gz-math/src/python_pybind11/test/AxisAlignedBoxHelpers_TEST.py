# Copyright (C) 2025 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
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
from gz.math8 import Boxd, Sphered, Capsuled, Cylinderd, AxisAlignedBoxHelpers, Vector3d

class TestAxisAlignedBoxHelpers(unittest.TestCase):
    # Basic Box
    def test_convert_box(self):
        box = Boxd(2.0, 4.0, 6.0)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(box)
        self.assertEqual(aabb.min(), Vector3d(-1.0, -2.0, -3.0))
        self.assertEqual(aabb.max(), Vector3d(1.0, 2.0, 3.0))

    # Basic Sphere
    def test_convert_sphere(self):
        sphere = Sphered(3.0)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(sphere)
        self.assertEqual(aabb.min(), Vector3d(-3.0, -3.0, -3.0))
        self.assertEqual(aabb.max(), Vector3d(3.0, 3.0, 3.0))

    # Basic Capsule
    def test_convert_capsule(self):
        capsule = Capsuled(5.0, 2.0)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(capsule)
        self.assertEqual(aabb.min(), Vector3d(-2.0, -2.0, -4.5))
        self.assertEqual(aabb.max(), Vector3d(2.0, 2.0, 4.5))

    # Basic Cylinder
    def test_convert_cylinder(self):
        cylinder = Cylinderd(5.0, 2.0)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(cylinder)
        self.assertEqual(aabb.min(), Vector3d(-2.0, -2.0, -2.5))
        self.assertEqual(aabb.max(), Vector3d(2.0, 2.0, 2.5))

    # Zero-Sized Box
    def test_convert_zero_size_box(self):
        box = Boxd(0.0, 0.0, 0.0)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(box)
        self.assertEqual(aabb.min(), Vector3d(0.0, 0.0, 0.0))
        self.assertEqual(aabb.max(), Vector3d(0.0, 0.0, 0.0))

    # Negative-Sized Box
    def test_convert_negative_size_box(self):
        box = Boxd(-2.0, -4.0, -6.0)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(box)
        self.assertEqual(aabb.min(), Vector3d(-1.0, -2.0, -3.0))
        self.assertEqual(aabb.max(), Vector3d(1.0, 2.0, 3.0))

    # Large Sphere
    def test_convert_large_sphere(self):
        sphere = Sphered(1e6)
        aabb = AxisAlignedBoxHelpers.ConvertToAxisAlignedBox(sphere)
        self.assertEqual(aabb.min(), Vector3d(-1e6, -1e6, -1e6))
        self.assertEqual(aabb.max(), Vector3d(1e6, 1e6, 1e6))


if __name__ == '__main__':
    unittest.main()
