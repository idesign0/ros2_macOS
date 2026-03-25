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

#include <memory>

#include "CommonRenderingTest.hh"

#include "gz/rendering/Capsule.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class CapsuleTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(CapsuleTest, Capsule)
{
  CHECK_SUPPORTED_ENGINE("ogre", "ogre2");

  ScenePtr scene = engine->CreateScene("scene");

  CapsulePtr capsule = scene->CreateCapsule();
  ASSERT_NE(nullptr, capsule);

  EXPECT_DOUBLE_EQ(capsule->Radius(), 0.5);
  EXPECT_DOUBLE_EQ(capsule->Length(), 0.5);

  capsule->SetRadius(0.7);
  capsule->SetLength(1.9);

  EXPECT_DOUBLE_EQ(capsule->Radius(), 0.7);
  EXPECT_DOUBLE_EQ(capsule->Length(), 1.9);

  // create material
  MaterialPtr mat = scene->CreateMaterial();
  mat->SetAmbient(0.6, 0.7, 0.8);
  mat->SetDiffuse(0.3, 0.8, 0.2);
  mat->SetSpecular(0.4, 0.9, 1.0);
  mat->SetTransparency(0.3);

  capsule->SetMaterial(mat);
  MaterialPtr capsuleMat = capsule->Material();
  ASSERT_NE(nullptr, capsuleMat);
  EXPECT_EQ(math::Color(0.6f, 0.7f, 0.8f), capsuleMat->Ambient());
  EXPECT_EQ(math::Color(0.3f, 0.8f, 0.2f), capsuleMat->Diffuse());
  EXPECT_EQ(math::Color(0.4f, 0.9f, 1.0f), capsuleMat->Specular());

  // test cloning a capsule
  auto clonedCapsule =
    std::dynamic_pointer_cast<rendering::Capsule>(capsule->Clone());
  ASSERT_NE(nullptr, clonedCapsule);
  EXPECT_DOUBLE_EQ(clonedCapsule->Radius(), capsule->Radius());
  EXPECT_DOUBLE_EQ(clonedCapsule->Length(), capsule->Length());

  // compare materials (the material is cloned, so the name is different but
  // the properties of the material are the same)
  auto clonedMaterial = clonedCapsule->Material();
  ASSERT_NE(nullptr, clonedMaterial);
  auto originalMaterial = capsule->Material();
  ASSERT_NE(nullptr, originalMaterial);
  EXPECT_NE(clonedMaterial, originalMaterial);
  EXPECT_NE(clonedMaterial->Name(), originalMaterial->Name());
  EXPECT_EQ(clonedMaterial->Type(), originalMaterial->Type());
  EXPECT_EQ(clonedMaterial->Ambient(), originalMaterial->Ambient());
  EXPECT_EQ(clonedMaterial->Diffuse(), originalMaterial->Diffuse());
  EXPECT_EQ(clonedMaterial->Specular(), originalMaterial->Specular());
  EXPECT_DOUBLE_EQ(clonedMaterial->Transparency(),
      originalMaterial->Transparency());

  // Clean up
  engine->DestroyScene(scene);
}
