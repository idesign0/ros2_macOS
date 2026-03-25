/*
 * Copyright 2024 CogniPilot Foundation
 * Copyright 2024 Open Source Robotics Foundation
 * Copyright 2024 Rudis Laboratories
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <gtest/gtest.h>
#include <cmath>

#include "gz/math/Cone.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(ConeTest, Constructor)
{
  // Default constructor
  {
    math::Coned cone;
    EXPECT_DOUBLE_EQ(0.0, cone.Length());
    EXPECT_DOUBLE_EQ(0.0, cone.Radius());
    EXPECT_EQ(math::Quaterniond::Identity, cone.RotationalOffset());
    EXPECT_EQ(math::Material(), cone.Mat());

    math::Coned cone2;
    EXPECT_EQ(cone, cone2);
  }

  // Length and radius constructor
  {
    math::Coned cone(1.0, 2.0);
    EXPECT_DOUBLE_EQ(1.0, cone.Length());
    EXPECT_DOUBLE_EQ(2.0, cone.Radius());
    EXPECT_EQ(math::Quaterniond::Identity, cone.RotationalOffset());
    EXPECT_EQ(math::Material(), cone.Mat());

    math::Coned cone2(1.0, 2.0);
    EXPECT_EQ(cone, cone2);
  }

  // Length, radius, and rot constructor
  {
    math::Coned cone(1.0, 2.0, math::Quaterniond(0.1, 0.2, 0.3));
    EXPECT_DOUBLE_EQ(1.0, cone.Length());
    EXPECT_DOUBLE_EQ(2.0, cone.Radius());
    EXPECT_EQ(math::Quaterniond(0.1, 0.2, 0.3),
        cone.RotationalOffset());
    EXPECT_EQ(math::Material(), cone.Mat());

    math::Coned cone2(1.0, 2.0, math::Quaterniond(0.1, 0.2, 0.3));
    EXPECT_EQ(cone, cone2);
  }

  // Length, radius, mat and rot constructor
  {
    math::Coned cone(1.0, 2.0,
        math::Material(math::MaterialType::WOOD),
        math::Quaterniond(0.1, 0.2, 0.3));
    EXPECT_DOUBLE_EQ(1.0, cone.Length());
    EXPECT_DOUBLE_EQ(2.0, cone.Radius());
    EXPECT_EQ(math::Quaterniond(0.1, 0.2, 0.3),
        cone.RotationalOffset());
    EXPECT_EQ(math::Material(math::MaterialType::WOOD), cone.Mat());

    math::Coned cone2(1.0, 2.0,
        math::Material(math::MaterialType::WOOD),
        math::Quaterniond(0.1, 0.2, 0.3));
    EXPECT_EQ(cone, cone2);
  }
}

//////////////////////////////////////////////////
TEST(ConeTest, Mutators)
{
  math::Coned cone;
  EXPECT_DOUBLE_EQ(0.0, cone.Length());
  EXPECT_DOUBLE_EQ(0.0, cone.Radius());
  EXPECT_EQ(math::Quaterniond::Identity, cone.RotationalOffset());
  EXPECT_EQ(math::Material(), cone.Mat());

  cone.SetLength(100.1);
  cone.SetRadius(.123);
  cone.SetRotationalOffset(math::Quaterniond(1.2, 2.3, 3.4));
  cone.SetMat(math::Material(math::MaterialType::PINE));

  EXPECT_DOUBLE_EQ(100.1, cone.Length());
  EXPECT_DOUBLE_EQ(.123, cone.Radius());
  EXPECT_EQ(math::Quaterniond(1.2, 2.3, 3.4),
    cone.RotationalOffset());
  EXPECT_EQ(math::Material(math::MaterialType::PINE), cone.Mat());
}

//////////////////////////////////////////////////
TEST(ConeTest, VolumeAndDensity)
{
  double mass = 1.0;
  math::Coned cone(1.0, 0.001);
  double expectedVolume = (GZ_PI * std::pow(0.001, 2) * 1.0 / 3.0);
  EXPECT_DOUBLE_EQ(expectedVolume, cone.Volume());

  double expectedDensity = mass / expectedVolume;
  EXPECT_DOUBLE_EQ(expectedDensity, cone.DensityFromMass(mass));

  // Bad density
  math::Coned cone2;
  EXPECT_GT(0.0, cone2.DensityFromMass(mass));
}

//////////////////////////////////////////////////
TEST(ConeTest, Mass)
{
  double mass = 2.0;
  double l = 2.0;
  double r = 0.1;
  math::Coned cone(l, r);
  cone.SetDensityFromMass(mass);

  math::MassMatrix3d massMat;
  double ixxIyy = (3.0/80.0) * mass * (4*r*r + l*l);
  double izz = (3.0/10.0) * mass * r * r;

  math::MassMatrix3d expectedMassMat;
  expectedMassMat.SetInertiaMatrix(ixxIyy, ixxIyy, izz, 0.0, 0.0, 0.0);
  expectedMassMat.SetMass(mass);

  cone.MassMatrix(massMat);
  EXPECT_EQ(expectedMassMat, massMat);
  EXPECT_DOUBLE_EQ(expectedMassMat.Mass(), massMat.Mass());

  auto massMatOpt = cone.MassMatrix();
  ASSERT_NE(std::nullopt, massMatOpt);
  EXPECT_EQ(expectedMassMat, *massMatOpt);
  EXPECT_EQ(expectedMassMat.DiagonalMoments(), massMatOpt->DiagonalMoments());
  EXPECT_DOUBLE_EQ(expectedMassMat.Mass(), massMatOpt->Mass());
}
