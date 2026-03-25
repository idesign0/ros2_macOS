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

#include "gz/rendering/LightVisual.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class LightVisualTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(LightVisualTest, LightVisual)
{
  ScenePtr scene = engine->CreateScene("scene");

  // create visual
  LightVisualPtr lightVisual = scene->CreateLightVisual();
  ASSERT_NE(nullptr, lightVisual);

  // check initial values
  EXPECT_EQ(LightVisualType::LVT_EMPTY, lightVisual->Type());
  EXPECT_DOUBLE_EQ(0, lightVisual->InnerAngle());
  EXPECT_DOUBLE_EQ(0, lightVisual->OuterAngle());

  lightVisual->SetType(LightVisualType::LVT_POINT);
  lightVisual->SetInnerAngle(0.7);
  lightVisual->SetOuterAngle(1.3);
  EXPECT_EQ(LightVisualType::LVT_POINT, lightVisual->Type());
  EXPECT_DOUBLE_EQ(0.7, lightVisual->InnerAngle());
  EXPECT_DOUBLE_EQ(1.3, lightVisual->OuterAngle());

  // Clean up
  engine->DestroyScene(scene);
}
