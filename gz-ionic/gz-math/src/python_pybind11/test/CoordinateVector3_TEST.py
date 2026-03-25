# Copyright (C) 2024 Open Source Robotics Foundation
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

import copy
import math
import unittest

from gz.math8 import Angle, CoordinateVector3, Vector3d


class TestCoordinateVector3(unittest.TestCase):

    def test_construction(self):
        # Empty constructor
        vec = CoordinateVector3()
        self.assertFalse(vec.is_spherical())
        self.assertTrue(vec.is_metric())
        self.assertIsNotNone(vec.x())
        self.assertIsNotNone(vec.y())
        self.assertIsNotNone(vec.z())
        self.assertIsNone(vec.lat())
        self.assertIsNone(vec.lon())
        self.assertAlmostEqual(0.0, vec.x())
        self.assertAlmostEqual(0.0, vec.y())
        self.assertAlmostEqual(0.0, vec.z())

        # Metric constructor
        vec_m = CoordinateVector3.metric(1, 2, 3)
        self.assertFalse(vec_m.is_spherical())
        self.assertTrue(vec_m.is_metric())
        self.assertIsNotNone(vec_m.x())
        self.assertIsNotNone(vec_m.y())
        self.assertIsNotNone(vec_m.z())
        self.assertIsNone(vec_m.lat())
        self.assertIsNone(vec_m.lon())
        self.assertAlmostEqual(1.0, vec_m.x())
        self.assertAlmostEqual(2.0, vec_m.y())
        self.assertAlmostEqual(3.0, vec_m.z())

        # Metric constructor from vector
        vec_m2 = CoordinateVector3.metric(Vector3d(1, 2, 3))
        self.assertFalse(vec_m2.is_spherical())
        self.assertTrue(vec_m2.is_metric())
        self.assertIsNotNone(vec_m2.x())
        self.assertIsNotNone(vec_m2.y())
        self.assertIsNotNone(vec_m2.z())
        self.assertIsNone(vec_m2.lat())
        self.assertIsNone(vec_m2.lon())
        self.assertAlmostEqual(1.0, vec_m2.x())
        self.assertAlmostEqual(2.0, vec_m2.y())
        self.assertAlmostEqual(3.0, vec_m2.z())

        # Spherical constructor
        vec_s = CoordinateVector3.spherical(Angle(1), Angle(2), 3)
        self.assertTrue(vec_s.is_spherical())
        self.assertFalse(vec_s.is_metric())
        self.assertIsNone(vec_s.x())
        self.assertIsNone(vec_s.y())
        self.assertIsNotNone(vec_s.z())
        self.assertIsNotNone(vec_s.lat())
        self.assertIsNotNone(vec_s.lon())
        self.assertAlmostEqual(1.0, vec_s.lat().radian())
        self.assertAlmostEqual(2.0, vec_s.lon().radian())
        self.assertAlmostEqual(3.0, vec_s.z())

        vec2 = CoordinateVector3(vec_m)
        self.assertEqual(vec2, vec_m)

        # Copy
        vec3 = vec_m
        self.assertEqual(vec3, vec_m)

        # Inequality
        vec4 = CoordinateVector3()
        self.assertNotEqual(vec_m, vec4)

    def test_setters(self):
        vec1 = CoordinateVector3.metric(0.1, 0.2, 0.4)

        vec1.set_metric(1.1, 2.2, 3.4)
        self.assertTrue(vec1, CoordinateVector3.metric(1.1, 2.2, 3.4))

        vec1.set_metric(-1.1, -2.2, -3.4)
        self.assertTrue(vec1, CoordinateVector3.metric(-1.1, -2.2, -3.4))

        vec1.set_spherical(Angle(1.1), Angle(2.2), 3.4)
        self.assertTrue(
            vec1, CoordinateVector3.spherical(Angle(1.1), Angle(2.2), 3.4))

    def test_as_metric_vector(self):
        vec1 = CoordinateVector3.metric(0.1, 0.2, 0.4)
        self.assertIsNotNone(vec1.as_metric_vector())
        self.assertEqual(vec1.as_metric_vector(), Vector3d(0.1, 0.2, 0.4))

        vec2 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.4)
        self.assertIsNone(vec2.as_metric_vector())

    def test_add_metric(self):
        vec1 = CoordinateVector3.metric(0.1, 0.2, 0.4)
        vec2 = CoordinateVector3.metric(1.1, 2.2, 3.4)

        vec3 = copy.deepcopy(vec1)
        vec3 += vec2

        self.assertEqual(vec1 + vec2, CoordinateVector3.metric(1.2, 2.4, 3.8))
        self.assertEqual(vec3, CoordinateVector3.metric(1.2, 2.4, 3.8))

    def test_add_spherical(self):
        vec1 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.4)
        vec2 = CoordinateVector3.spherical(Angle(1.1), Angle(2.2), 3.4)

        vec3 = copy.deepcopy(vec1)
        vec3 += vec2

        self.assertEqual(
            vec1 + vec2,
            CoordinateVector3.spherical(Angle(1.2), Angle(2.4), 3.8))
        self.assertEqual(
            vec3,
            CoordinateVector3.spherical(Angle(1.2), Angle(2.4), 3.8))

    def test_add_mismatch(self):
        vec1 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.4)
        vec2 = CoordinateVector3.metric(1.1, 2.2, 3.4)

        vec3 = copy.deepcopy(vec1)
        vec3 += vec2
        vec4 = copy.deepcopy(vec2)
        vec4 += vec1

        vec12 = vec1 + vec2
        vec21 = vec2 + vec1

        self.assertFalse(vec3.is_metric())
        self.assertFalse(vec12.is_metric())
        self.assertFalse(vec4.is_spherical())
        self.assertFalse(vec21.is_spherical())
        self.assertTrue(vec3.is_spherical())
        self.assertTrue(vec12.is_spherical())
        self.assertTrue(vec21.is_metric())
        self.assertTrue(vec4.is_metric())

        self.assertTrue(math.isnan(vec3.lat().radian()))
        self.assertTrue(math.isnan(vec12.lat().radian()))
        self.assertTrue(math.isnan(vec21.x()))
        self.assertTrue(math.isnan(vec4.x()))
        
        self.assertTrue(math.isnan(vec3.lon().radian()))
        self.assertTrue(math.isnan(vec12.lon().radian()))
        self.assertTrue(math.isnan(vec21.y()))
        self.assertTrue(math.isnan(vec4.y()))
        
        self.assertTrue(math.isnan(vec3.z()))
        self.assertTrue(math.isnan(vec12.z()))
        self.assertTrue(math.isnan(vec21.z()))
        self.assertTrue(math.isnan(vec4.z()))

    def test_sub_metric(self):
        vec1 = CoordinateVector3.metric(0.1, 0.2, 0.4)
        vec2 = CoordinateVector3.metric(1.1, 2.2, 3.4)

        vec3 = copy.deepcopy(vec1)
        vec3 -= vec2

        self.assertEqual(vec1 - vec2, CoordinateVector3.metric(-1, -2, -3))
        self.assertEqual(vec3, CoordinateVector3.metric(-1, -2, -3))

    def test_sub_spherical(self):
        vec1 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.4)
        vec2 = CoordinateVector3.spherical(Angle(1.1), Angle(2.2), 3.4)

        vec3 = copy.deepcopy(vec1)
        vec3 -= vec2

        self.assertEqual(
            vec1 - vec2,
            CoordinateVector3.spherical(Angle(-1), Angle(-2), -3))
        self.assertEqual(
            vec3,
            CoordinateVector3.spherical(Angle(-1), Angle(-2), -3))

    def test_sub_mismatch(self):
        vec1 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.4)
        vec2 = CoordinateVector3.metric(1.1, 2.2, 3.4)

        vec3 = copy.deepcopy(vec1)
        vec3 -= vec2
        vec4 = copy.deepcopy(vec2)
        vec4 -= vec1

        vec12 = vec1 - vec2
        vec21 = vec2 - vec1

        self.assertFalse(vec3.is_metric())
        self.assertFalse(vec12.is_metric())
        self.assertFalse(vec4.is_spherical())
        self.assertFalse(vec21.is_spherical())
        self.assertTrue(vec3.is_spherical())
        self.assertTrue(vec12.is_spherical())
        self.assertTrue(vec21.is_metric())
        self.assertTrue(vec4.is_metric())

        self.assertTrue(math.isnan(vec3.lat().radian()))
        self.assertTrue(math.isnan(vec12.lat().radian()))
        self.assertTrue(math.isnan(vec21.x()))
        self.assertTrue(math.isnan(vec4.x()))

        self.assertTrue(math.isnan(vec3.lon().radian()))
        self.assertTrue(math.isnan(vec12.lon().radian()))
        self.assertTrue(math.isnan(vec21.y()))
        self.assertTrue(math.isnan(vec4.y()))

        self.assertTrue(math.isnan(vec3.z()))
        self.assertTrue(math.isnan(vec12.z()))
        self.assertTrue(math.isnan(vec21.z()))
        self.assertTrue(math.isnan(vec4.z()))

    def test_not_equal(self):
        vec1 = CoordinateVector3.metric(0.1, 0.2, 0.3)
        vec2 = CoordinateVector3.metric(0.2, 0.2, 0.3)
        vec3 = CoordinateVector3.metric(0.1, 0.2, 0.3)
        vec4 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.3)
        vec5 = CoordinateVector3.spherical(Angle(0.2), Angle(0.2), 0.3)
        vec6 = CoordinateVector3.spherical(Angle(0.1), Angle(0.2), 0.3)
    
        self.assertTrue(vec1 != vec2)
        self.assertFalse(vec1 != vec3)
    
        self.assertTrue(vec4 != vec5)
        self.assertFalse(vec4 != vec6)
    
        self.assertTrue(vec1 != vec4)
        self.assertTrue(vec1 != vec5)
        self.assertTrue(vec1 != vec6)
        self.assertTrue(vec2 != vec4)
        self.assertTrue(vec2 != vec5)
        self.assertTrue(vec2 != vec6)
        self.assertTrue(vec3 != vec4)
        self.assertTrue(vec3 != vec5)
        self.assertTrue(vec3 != vec6)
        self.assertTrue(vec4 != vec1)
        self.assertTrue(vec4 != vec2)
        self.assertTrue(vec4 != vec3)
        self.assertTrue(vec5 != vec1)
        self.assertTrue(vec5 != vec2)
        self.assertTrue(vec5 != vec3)
        self.assertTrue(vec6 != vec1)
        self.assertTrue(vec6 != vec2)
        self.assertTrue(vec6 != vec3)

    def test_equal_tolerance_metric(self):
        zero = CoordinateVector3()
        one = CoordinateVector3.metric(1, 1, 1)
        self.assertFalse(zero.equal(one, 1e-6, Angle(1e-6)))
        self.assertFalse(zero.equal(one, 1e-3, Angle(1e-3)))
        self.assertFalse(zero.equal(one, 1e-1, Angle(1e-1)))
        self.assertTrue(zero.equal(one, 1, Angle(1)))
        self.assertTrue(zero.equal(one, 1.1, Angle(1.1)))
        self.assertTrue(zero.equal(one, 1.1, Angle(1e-6)))
        self.assertFalse(zero.equal(one, 1e-6, Angle(1.1)))

    def test_equal_tolerance_spherical(self):
        zero = CoordinateVector3.spherical(Angle(0), Angle(0), 0)
        one = CoordinateVector3.spherical(Angle(1), Angle(1), 1)
        self.assertFalse(zero.equal(one, 1e-6, Angle(1e-6)))
        self.assertFalse(zero.equal(one, 1e-3, Angle(1e-3)))
        self.assertFalse(zero.equal(one, 1e-1, Angle(1e-1)))
        self.assertTrue(zero.equal(one, 1, Angle(1)))
        self.assertTrue(zero.equal(one, 1.1, Angle(1.1)))
        self.assertFalse(zero.equal(one, 1.1, Angle(1e-6)))
        self.assertFalse(zero.equal(one, 1e-6, Angle(1.1)))

    def test_equal_tolerance_mismatch(self):
        zero = CoordinateVector3.spherical(Angle(0), Angle(0), 0)
        one = CoordinateVector3.metric(1, 1, 1)
        self.assertFalse(zero.equal(one, 1e-6, Angle(1e-6)))
        self.assertFalse(zero.equal(one, 1e-3, Angle(1e-3)))
        self.assertFalse(zero.equal(one, 1e-1, Angle(1e-1)))
        self.assertFalse(zero.equal(one, 1, Angle(1)))
        self.assertFalse(zero.equal(one, 1.1, Angle(1.1)))
        self.assertFalse(zero.equal(one, 1.1, Angle(1e-6)))
        self.assertFalse(zero.equal(one, 1e-6, Angle(1.1)))

    def test_finite(self):
        self.assertTrue(CoordinateVector3.metric(0.1, 0.2, 0.3).is_finite())
        self.assertTrue(CoordinateVector3.spherical(
            Angle(0.1), Angle(0.2), 0.3).is_finite())
        self.assertFalse(CoordinateVector3.metric(
            math.nan, math.nan, math.nan).is_finite())
        self.assertFalse(CoordinateVector3.spherical(
            Angle(math.nan), Angle(math.nan), math.nan).is_finite())
        self.assertFalse(CoordinateVector3.metric(
            math.inf, math.inf, math.inf).is_finite())
        self.assertFalse(CoordinateVector3.spherical(
            Angle(math.inf), Angle(math.inf), math.inf).is_finite())
        self.assertFalse(CoordinateVector3.metric(
            math.inf, 0, 0).is_finite())
        self.assertFalse(CoordinateVector3.spherical(
            Angle(math.inf), Angle(0), 0).is_finite())

    def test_nan_metric(self):
        nanVec = CoordinateVector3.metric(math.nan, math.nan, math.nan)
        self.assertFalse(nanVec.is_finite())
        self.assertIsNotNone(nanVec.x())
        self.assertIsNotNone(nanVec.y())
        self.assertIsNotNone(nanVec.z())
        self.assertIsNone(nanVec.lat())
        self.assertIsNone(nanVec.lon())
        self.assertTrue(math.isnan(nanVec.x()))
        self.assertTrue(math.isnan(nanVec.y()))
        self.assertTrue(math.isnan(nanVec.z()))

    def test_nan_spherical(self):
        nanVec = CoordinateVector3.spherical(
            Angle(math.nan), Angle(math.nan), math.nan)
        self.assertFalse(nanVec.is_finite())
        self.assertIsNone(nanVec.x())
        self.assertIsNone(nanVec.y())
        self.assertIsNotNone(nanVec.z())
        self.assertIsNotNone(nanVec.lat())
        self.assertIsNotNone(nanVec.lon())
        self.assertTrue(math.isnan(nanVec.lat().radian()))
        self.assertTrue(math.isnan(nanVec.lon().radian()))
        self.assertTrue(math.isnan(nanVec.z()))

    def test_str_metric(self):
        v = CoordinateVector3.metric(0.1234, 1.234, 2.3456)
        self.assertEqual(str(v), "0.1234 1.234 2.3456")

    def test_str_spherical(self):
        v = CoordinateVector3.spherical(Angle(0.1234), Angle(1.234), 2.3456)
        self.assertEqual(str(v), "7.0703° 70.703° 2.3456")

if __name__ == '__main__':
    unittest.main()
