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

#include "gz/rendering/JointVisual.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class JointVisualTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(JointVisualTest, JointVisual)
{
  ScenePtr scene = engine->CreateScene("scene");

  // create visual
  JointVisualPtr jointVisual = scene->CreateJointVisual();
  ASSERT_NE(nullptr, jointVisual);

  // create joint child visual
  VisualPtr jointChildVisual = scene->CreateVisual("joint_child");

  // create joint parent visual
  VisualPtr jointParentVisual = scene->CreateVisual("joint_parent");

  // check initial values
  EXPECT_EQ(JointVisualType::JVT_NONE, jointVisual->Type());
  EXPECT_EQ(nullptr, jointVisual->ArrowVisual());
  EXPECT_EQ(nullptr, jointVisual->ParentAxisVisual());
  EXPECT_EQ(math::Vector3d::Zero, jointVisual->Axis());
  EXPECT_EQ(math::Vector3d::Zero, jointVisual->ParentAxis());

  // set joint type
  jointVisual->SetType(JointVisualType::JVT_REVOLUTE2);
  EXPECT_EQ(JointVisualType::JVT_REVOLUTE2, jointVisual->Type());

  // set child axis
  math::Vector3d axis2(0.0, 1.0, 0.0);
  bool useParentFrame = false;
  jointChildVisual->AddChild(jointVisual);
  jointVisual->SetAxis(axis2, useParentFrame);
  jointVisual->PreRender();
  EXPECT_NE(nullptr, jointVisual->ArrowVisual());
  EXPECT_EQ(axis2, jointVisual->Axis());
  EXPECT_EQ(math::Vector3d::Zero, jointVisual->ParentAxis());
  EXPECT_EQ(nullptr, jointVisual->ParentAxisVisual());

  // set parent axis
  math::Vector3d axis1(0.0, 1.0, 0.0);
  useParentFrame = true;
  jointVisual->SetParentAxis(axis1, "joint_parent", useParentFrame);
  jointVisual->PreRender();
  EXPECT_NE(nullptr, jointVisual->ArrowVisual());
  EXPECT_EQ(axis2, jointVisual->Axis());
  EXPECT_EQ(axis1, jointVisual->ParentAxis());
  EXPECT_NE(nullptr, jointVisual->ParentAxisVisual());

  // set visibility
  jointVisual->SetVisible(false);
  jointVisual->SetVisible(true);

  // Clean up
  engine->DestroyScene(scene);
}
