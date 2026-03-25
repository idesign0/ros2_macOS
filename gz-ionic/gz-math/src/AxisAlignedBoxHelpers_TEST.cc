/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include "gz/math/Box.hh"
#include "gz/math/Sphere.hh"
#include "gz/math/Capsule.hh"
#include "gz/math/Cylinder.hh"
#include "gz/math/AxisAlignedBox.hh"
#include "gz/math/AxisAlignedBoxHelpers.hh"

using namespace gz;

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertBox)
{
  math::Box<double> box(2.0, 4.0, 6.0);
  math::AxisAlignedBox aabb =
          math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(box);
  EXPECT_EQ(aabb.Min(), math::Vector3d(-1.0, -2.0, -3.0));
  EXPECT_EQ(aabb.Max(), math::Vector3d(1.0, 2.0, 3.0));
}

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertSphere)
{
  math::Sphered sphere(3.0);
  math::AxisAlignedBox aabb =
      math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(sphere);
  EXPECT_EQ(aabb.Min(), math::Vector3d(-3.0, -3.0, -3.0));
  EXPECT_EQ(aabb.Max(), math::Vector3d(3.0, 3.0, 3.0));
}

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertCapsule)
{
  math::Capsuled capsule(5.0, 2.0);
  math::AxisAlignedBox aabb =
      math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(capsule);
  EXPECT_EQ(aabb.Min(), math::Vector3d(-2.0, -2.0, -4.5));
  EXPECT_EQ(aabb.Max(), math::Vector3d(2.0, 2.0, 4.5));
}

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertCylinder)
{
  math::Cylinderd cylinder(5.0, 2.0);
  math::AxisAlignedBox aabb =
      math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(cylinder);
  EXPECT_EQ(aabb.Min(), math::Vector3d(-2.0, -2.0, -2.5));
  EXPECT_EQ(aabb.Max(), math::Vector3d(2.0, 2.0, 2.5));
}

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertZeroSizeBox)
{
  math::Box<double> box(0.0, 0.0, 0.0);
  math::AxisAlignedBox aabb =
      math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(box);
  EXPECT_EQ(aabb.Min(), math::Vector3d(0.0, 0.0, 0.0));
  EXPECT_EQ(aabb.Max(), math::Vector3d(0.0, 0.0, 0.0));
}

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertNegativeSizeBox)
{
  math::Box<double> box(-2.0, -4.0, -6.0);
  math::AxisAlignedBox aabb =
      math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(box);
  EXPECT_EQ(aabb.Min(), math::Vector3d(-1.0, -2.0, -3.0));
  EXPECT_EQ(aabb.Max(), math::Vector3d(1.0, 2.0, 3.0));
}

//////////////////////////////////////////////////
TEST(AxisAlignedBoxHelpersTest, ConvertLargeSphere)
{
  math::Sphered sphere(1e6);
  math::AxisAlignedBox aabb =
      math::AxisAlignedBoxHelpers<double>::ConvertToAxisAlignedBox(sphere);
  EXPECT_EQ(aabb.Min(), math::Vector3d(-1e6, -1e6, -1e6));
  EXPECT_EQ(aabb.Max(), math::Vector3d(1e6, 1e6, 1e6));
}
