/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#include <string>

#include "CommonRenderingTest.hh"

#include "gz/rendering/COMVisual.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class COMVisualTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(COMVisualTest, COMVisual)
{
  ScenePtr scene = engine->CreateScene("scene");

  // create visual
  COMVisualPtr comVisual = scene->CreateCOMVisual();
  ASSERT_NE(nullptr, comVisual);

  gz::math::MassMatrix3d massMatrix(
      0.0, {2.0, 1.5, 1.0}, {0.0, 0.0, 0.0});
  gz::math::Pose3d p(0.0, 1.0, 2.5, 1.0, 0.4, 0.4);
  gz::math::Inertiald inertial;
  inertial.SetMassMatrix(massMatrix);
  inertial.SetPose(p);

  // check initial values
  EXPECT_EQ(nullptr, comVisual->SphereVisual());
  EXPECT_EQ(gz::math::Pose3d::Zero, comVisual->InertiaPose());
  EXPECT_DOUBLE_EQ(1.0, comVisual->Mass());

  // set invalid mass
  comVisual->SetMass(-1.0);
  comVisual->PreRender();
  EXPECT_EQ(nullptr, comVisual->SphereVisual());
  EXPECT_DOUBLE_EQ(1.0, comVisual->Mass());
  EXPECT_EQ(gz::math::Pose3d::Zero, comVisual->InertiaPose());

  // set invalid inertial
  comVisual->SetInertial(inertial);
  comVisual->PreRender();
  EXPECT_EQ(nullptr, comVisual->SphereVisual());
  EXPECT_DOUBLE_EQ(1.0, comVisual->Mass());
  EXPECT_EQ(inertial.Pose(), comVisual->InertiaPose());

  // set valid mass
  comVisual->SetMass(2.0);
  EXPECT_EQ(nullptr, comVisual->SphereVisual());
  EXPECT_DOUBLE_EQ(2.0, comVisual->Mass());

  // set parent
  VisualPtr parentVisual = scene->CreateVisual("parent_visual");
  parentVisual->AddChild(comVisual);
  comVisual->PreRender();
  EXPECT_NE(nullptr, comVisual->SphereVisual());

  // Clean up
  engine->DestroyScene(scene);
}
