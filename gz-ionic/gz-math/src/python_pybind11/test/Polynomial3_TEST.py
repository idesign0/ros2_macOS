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
from gz.math8 import Helpers, Intervald, Polynomial3d, Vector4d, isnan


class TestInterval(unittest.TestCase):

    def test_default_constructor(self):
        poly = Polynomial3d()
        self.assertEqual(poly.coeffs(), Vector4d.ZERO)

    def test_constructor(self):
        poly = Polynomial3d(Vector4d.ONE)
        self.assertEqual(poly.coeffs(), Vector4d.ONE)

    def test_constructor_helpers(self):
        poly = Polynomial3d.constant(1.)
        self.assertEqual(poly.coeffs(), Vector4d(0., 0., 0., 1.))

    def test_evaluate(self):
        p = Polynomial3d.constant(1.)
        self.assertAlmostEqual(p(-1.), 1.)
        self.assertAlmostEqual(p(0.), 1.)
        self.assertAlmostEqual(p(1.), 1.)
        self.assertAlmostEqual(p(Helpers.INF_D), 1.)
        self.assertTrue(isnan(p(Helpers.NAN_D)))

        p = Polynomial3d(Vector4d.ONE)
        self.assertAlmostEqual(p(-1.), 0.)
        self.assertAlmostEqual(p(0.), 1.)
        self.assertAlmostEqual(p(1.), 4.)
        self.assertAlmostEqual(p(-Helpers.INF_D), -Helpers.INF_D)
        self.assertTrue(isnan(p(Helpers.NAN_D)))

    def test_minimum(self):
        p = Polynomial3d.constant(1.)
        xInterval = Intervald.open(0., 0.)
        xMin = 0.
        self.assertTrue(isnan(p.minimum(xInterval)))
        min, xMin = p.minimum(xInterval, xMin)
        self.assertTrue(isnan(min))
        self.assertTrue(isnan(xMin))

        p = Polynomial3d.constant(1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), 1.)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, 1.)
        self.assertFalse(isnan(xMin))

        p = Polynomial3d(Vector4d(0., 0., 1., 1.))
        xInterval = Intervald.open(0., 1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 1.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 1.)
        self.assertAlmostEqual(xMin, 0.)

        p = Polynomial3d( Vector4d(0., 0., 1., 1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, -Helpers.INF_D)

        p = Polynomial3d( Vector4d(0., 0., -1., 1.))
        xInterval = Intervald.open(0., 1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 0.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 0.)
        self.assertAlmostEqual(xMin, 1.)

        p = Polynomial3d(Vector4d(0., 0., -1., 1.))
        xMin = 0
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, Helpers.INF_D)

        p = Polynomial3d( Vector4d(0., 1., 1., 1.))
        xInterval = Intervald.open(1., 2.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 3.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 3.)
        self.assertAlmostEqual(xMin, 1.)

        p = Polynomial3d(Vector4d(0., 1., 1., 1.))
        xInterval = Intervald.open(-1., 0.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 0.75)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 0.75)
        self.assertAlmostEqual(xMin, -0.5)

        p = Polynomial3d(Vector4d(0., 1., 1., 1.))
        xInterval = Intervald.open(-3., -2.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 3.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 3.)
        self.assertAlmostEqual(xMin, -2.)

        p = Polynomial3d(Vector4d(0., 1., 1., 1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), 0.75)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, 0.75)
        self.assertAlmostEqual(xMin, -0.5)

        p = Polynomial3d(Vector4d(0., -1., 1., 1.))
        xInterval = Intervald.open(1., 2.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), -1.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, -1.)
        self.assertAlmostEqual(xMin, 2.)

        p = Polynomial3d(Vector4d(0., -1., 1., 1.))
        xInterval = Intervald.open(-2., -1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), -5.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, -5.)
        self.assertAlmostEqual(xMin, -2.)

        p = Polynomial3d(Vector4d(0., -1., 1., 1.))
        xInterval = Intervald.open(0., 1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 1.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 1.)
        self.assertAlmostEqual(xMin, 0.)

        p = Polynomial3d(Vector4d(0., -1., 1., 1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, -Helpers.INF_D)

        p = Polynomial3d(Vector4d(1., 1., 1., 1.))
        xInterval = Intervald.open(-1., 1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 0.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 0.)
        self.assertAlmostEqual(xMin, -1.)

        p = Polynomial3d(Vector4d(1., 1., 1., 1.))
        xInterval =Intervald.open(-2., -1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), -5.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, -5.)
        self.assertAlmostEqual(xMin, -2.)

        p = Polynomial3d(Vector4d(1., 1., 1., 1.))
        xInterval = Intervald.open(2., 3.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 15.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 15.)
        self.assertAlmostEqual(xMin, 2.)

        p = Polynomial3d(Vector4d(1., 1., 1., 1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, -Helpers.INF_D)

        p = Polynomial3d(Vector4d(-1., 2., 1., 1.))
        xInterval = Intervald.open(-1., 1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 0.8873882090776197)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 0.8873882090776197)
        self.assertAlmostEqual(xMin, -0.2152504370215302)

        p = Polynomial3d(Vector4d(-1., 2., 1., 1.))
        xInterval = Intervald.open(-3., -2.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 15.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 15.)
        self.assertAlmostEqual(xMin, -2.)

        p = Polynomial3d(Vector4d(-1., 2., 1., 1.))
        xInterval = Intervald.open(1., 2.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 3.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 3.)
        self.assertAlmostEqual(xMin, 1.)

        p = Polynomial3d(Vector4d(-1., 2., 1., 1.))
        xInterval = Intervald.open(2., 3.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), -5.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, -5.)
        self.assertAlmostEqual(xMin, 3.)

        p = Polynomial3d(Vector4d(-1., 2., 1., 1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, Helpers.INF_D)

        p = Polynomial3d(Vector4d(1., -2., 1., 1.))
        xInterval = Intervald.open(-1., 1.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), -3.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, -3.)
        self.assertAlmostEqual(xMin, -1.)

        p = Polynomial3d(Vector4d(1., -2., 1., 1.))
        xInterval = Intervald.open(0., 2.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 1.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 1.)
        self.assertAlmostEqual(xMin, 0.)

        p = Polynomial3d(Vector4d(1., -2., 1., 1.))
        xInterval = Intervald.open(2., 3.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), 3.)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, 3.)
        self.assertAlmostEqual(xMin, 2.)

        p = Polynomial3d(Vector4d(1., -2., 1., 1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, -Helpers.INF_D)

        p = Polynomial3d(Vector4d(1., -4., -2., -1.))
        xInterval = Intervald.open(-1., 6.)
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(xInterval), -16.051047904897441)
        min, xMin = p.minimum(xInterval, xMin)
        self.assertAlmostEqual(min, -16.051047904897441)
        self.assertAlmostEqual(xMin, 2.8968052532744766)
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)

        p = Polynomial3d(Vector4d(1., -4., -2., -1.))
        xMin = Helpers.NAN_D
        self.assertAlmostEqual(p.minimum(), -Helpers.INF_D)
        min, xMin = p.minimum(xMin)
        self.assertAlmostEqual(min, -Helpers.INF_D)
        self.assertAlmostEqual(xMin, -Helpers.INF_D)

if __name__ == '__main__':
    unittest.main()
