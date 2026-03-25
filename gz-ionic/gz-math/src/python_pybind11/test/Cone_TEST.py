# Copyright 2024 CogniPilot Foundation
# Copyright 2024 Open Source Robotics Foundation
# Copyright 2024 Rudis Laboratories
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math
import unittest

import gz
from gz.math8 import Coned, MassMatrix3d, Material, Quaterniond


class TestCone(unittest.TestCase):

    def test_constructor(self):
        # Default constructor
        cone = Coned()
        self.assertEqual(0.0, cone.length())
        self.assertEqual(0.0, cone.radius())
        self.assertEqual(Quaterniond.IDENTITY, cone.rotational_offset())
        self.assertEqual(Material(), cone.mat())

        cone2 = Coned()
        self.assertEqual(cone, cone2)

        # Length and radius constructor
        cone = Coned(1.0, 2.0)
        self.assertEqual(1.0, cone.length())
        self.assertEqual(2.0, cone.radius())
        self.assertEqual(Quaterniond.IDENTITY, cone.rotational_offset())
        self.assertEqual(Material(), cone.mat())

        cone2 = Coned(1.0, 2.0)
        self.assertEqual(cone, cone2)

        # Length, radius, and rot constructor
        cone = Coned(1.0, 2.0, Quaterniond(0.1, 0.2, 0.3))
        self.assertEqual(1.0, cone.length())
        self.assertEqual(2.0, cone.radius())
        self.assertEqual(Quaterniond(0.1, 0.2, 0.3),
                         cone.rotational_offset())
        self.assertEqual(Material(), cone.mat())

        cone2 = Coned(1.0, 2.0, Quaterniond(0.1, 0.2, 0.3))
        self.assertEqual(cone, cone2)

        # Length, radius, mat and rot constructor
        cone = Coned(1.0, 2.0, Material(gz.math8.MaterialType.WOOD),
                             Quaterniond(0.1, 0.2, 0.3))
        self.assertEqual(1.0, cone.length())
        self.assertEqual(2.0, cone.radius())
        self.assertEqual(Quaterniond(0.1, 0.2, 0.3), cone.rotational_offset())
        self.assertEqual(Material(gz.math8.MaterialType.WOOD), cone.mat())

        cone2 = Coned(1.0, 2.0, Material(gz.math8.MaterialType.WOOD),
                              Quaterniond(0.1, 0.2, 0.3))
        self.assertEqual(cone, cone2)

    def test_mutators(self):
        cone = Coned()
        self.assertEqual(0.0, cone.length())
        self.assertEqual(0.0, cone.radius())
        self.assertEqual(Quaterniond.IDENTITY, cone.rotational_offset())
        self.assertEqual(Material(), cone.mat())

        cone.set_length(100.1)
        cone.set_radius(.123)
        cone.set_rotational_offset(Quaterniond(1.2, 2.3, 3.4))
        cone.set_mat(Material(gz.math8.MaterialType.PINE))

        self.assertEqual(100.1, cone.length())
        self.assertEqual(.123, cone.radius())
        self.assertEqual(Quaterniond(1.2, 2.3, 3.4), cone.rotational_offset())
        self.assertEqual(Material(gz.math8.MaterialType.PINE), cone.mat())

    def test_volume_and_density(self):
        mass = 1.0
        cone = Coned(1.0, 0.001)
        expectedVolume = (math.pi * math.pow(0.001, 2) * 1.0 / 3.0)
        self.assertEqual(expectedVolume, cone.volume())

        expectedDensity = mass / expectedVolume
        self.assertEqual(expectedDensity, cone.density_from_mass(mass))

        # Bad density
        cone2 = Coned()
        self.assertGreater(0.0, cone2.density_from_mass(mass))

    def test_mass(self):
        mass = 2.0
        length = 2.0
        r = 0.1
        cone = Coned(length, r)
        cone.set_density_from_mass(mass)

        massMat = MassMatrix3d()
        ixxIyy = (3/80.0) * mass * (4*r*r + length*length)
        izz = (3/10.0) * mass * r * r

        expectedMassMat = MassMatrix3d()
        expectedMassMat.set_inertia_matrix(ixxIyy, ixxIyy, izz, 0.0, 0.0, 0.0)
        expectedMassMat.set_mass(mass)

        cone.mass_matrix(massMat)
        self.assertEqual(expectedMassMat, massMat)
        self.assertEqual(expectedMassMat.mass(), massMat.mass())

        massMat2 = cone.mass_matrix()
        self.assertEqual(expectedMassMat, massMat2)
        self.assertEqual(expectedMassMat.diagonal_moments(), massMat2.diagonal_moments())
        self.assertEqual(expectedMassMat.mass(), massMat2.mass())


if __name__ == '__main__':
    unittest.main()
