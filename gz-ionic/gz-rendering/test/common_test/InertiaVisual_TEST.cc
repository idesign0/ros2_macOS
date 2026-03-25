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

#include "gz/rendering/InertiaVisual.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class InertiaVisualTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(InertiaVisualTest, InertiaVisual)
{
  ScenePtr scene = engine->CreateScene("scene");

  // create visual
  InertiaVisualPtr inertiaVisual = scene->CreateInertiaVisual();
  ASSERT_NE(nullptr, inertiaVisual);

  // check initial values
  EXPECT_EQ(nullptr, inertiaVisual->BoxVisual());

  gz::math::MassMatrix3d massMatrix(
      2.0, {2.0, 1.5, 1.0}, {0.0, 0.0, 0.0});
  gz::math::Pose3d p(0.0, 1.0, 2.5, 1.0, 0.4, 0.4);
  gz::math::Inertiald inertial;

  inertiaVisual->SetInertial(inertial);
  EXPECT_EQ(nullptr, inertiaVisual->BoxVisual());

  inertial.SetMassMatrix(massMatrix);
  inertial.SetPose(p);
  inertiaVisual->SetInertial(inertial);
  EXPECT_NE(nullptr, inertiaVisual->BoxVisual());

  // Clean up
  engine->DestroyScene(scene);
}
