/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include "gz/rendering/ArrowVisual.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

class ArrowVisualTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(ArrowVisualTest, ArrowVisual)
{
  ScenePtr scene = engine->CreateScene("scene");

  // create arrow visual
  ArrowVisualPtr visual = scene->CreateArrowVisual();
  ASSERT_NE(nullptr, visual);

  // check scale
  EXPECT_TRUE(visual->InheritScale());
  EXPECT_EQ(math::Vector3d::One, visual->LocalScale());
  EXPECT_EQ(math::Vector3d::One, visual->WorldScale());

  visual->SetLocalScale(0.2, 0.3, 0.4);
  EXPECT_EQ(math::Vector3d(0.2, 0.3, 0.4), visual->LocalScale());

  // check children and geometry
  EXPECT_EQ(3u, visual->ChildCount());

  NodePtr node = visual->Rotation();
  VisualPtr child = std::dynamic_pointer_cast<Visual>(node);
  ASSERT_NE(nullptr, child);
  EXPECT_EQ(1u, child->GeometryCount());
  MeshPtr mesh = std::dynamic_pointer_cast<Mesh>(child->GeometryByIndex(0u));
  ASSERT_NE(nullptr, mesh);
  MeshDescriptor desc = mesh->Descriptor();
  EXPECT_NE(std::string::npos, desc.meshName.find("rotation"));

  node = visual->Shaft();
  child = std::dynamic_pointer_cast<Visual>(node);
  ASSERT_NE(nullptr, child);
  EXPECT_EQ(1u, child->GeometryCount());
  mesh = std::dynamic_pointer_cast<Mesh>(child->GeometryByIndex(0u));
  ASSERT_NE(nullptr, mesh);
  desc = mesh->Descriptor();
  EXPECT_NE(std::string::npos, desc.meshName.find("cylinder"));

  node = visual->Head();
  child = std::dynamic_pointer_cast<Visual>(node);
  ASSERT_NE(nullptr, child);
  EXPECT_EQ(1u, child->GeometryCount());
  // test destroy
  ArrowVisualPtr visual2 = scene->CreateArrowVisual();
  ASSERT_NE(nullptr, visual2);
  visual2->Destroy();
  EXPECT_EQ(0u, visual2->ChildCount());
  mesh = std::dynamic_pointer_cast<Mesh>(child->GeometryByIndex(0u));
  ASSERT_NE(nullptr, mesh);
  desc = mesh->Descriptor();
  EXPECT_NE(std::string::npos, desc.meshName.find("cone"));

  // Clean up
  engine->DestroyScene(scene);
}
