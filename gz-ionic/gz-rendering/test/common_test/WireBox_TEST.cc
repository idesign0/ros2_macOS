/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include "CommonRenderingTest.hh"

#include "gz/rendering/WireBox.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class WireBoxTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(WireBoxTest, WireBox)
{
  CHECK_UNSUPPORTED_ENGINE("ogre", "ogre2");

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  WireBoxPtr wireBox = scene->CreateWireBox();
  ASSERT_NE(nullptr, wireBox);

  gz::math::AxisAlignedBox box(math::Vector3d(0.1, 0.2, 0.3),
                                     math::Vector3d(5.1, 5.2, 5.3));

  wireBox->SetBox(box);
  gz::math::AxisAlignedBox aabb = wireBox->Box();
  EXPECT_EQ(math::Vector3d(0.1, 0.2, 0.3), aabb.Min());
  EXPECT_EQ(math::Vector3d(5.1, 5.2, 5.3), aabb.Max());

  // create material
  MaterialPtr mat = scene->CreateMaterial();
  mat->SetAmbient(0.6, 0.7, 0.8);
  mat->SetDiffuse(0.3, 0.8, 0.2);
  mat->SetSpecular(0.4, 0.9, 1.0);

  wireBox->SetMaterial(mat);
  MaterialPtr wireBoxMat = wireBox->Material();
  ASSERT_NE(nullptr, wireBoxMat);
  EXPECT_EQ(math::Color(0.6f, 0.7f, 0.8f), wireBoxMat->Ambient());
  EXPECT_EQ(math::Color(0.3f, 0.8f, 0.2f), wireBoxMat->Diffuse());
  EXPECT_EQ(math::Color(0.4f, 0.9f, 1.0f), wireBoxMat->Specular());

  // Clean up
  engine->DestroyScene(scene);
}
