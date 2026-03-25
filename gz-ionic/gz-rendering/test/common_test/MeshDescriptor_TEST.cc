/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <gz/common/MeshManager.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/MeshDescriptor.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class MeshDescriptorTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(MeshDescriptorTest, Descriptor)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // empty constructor
  MeshDescriptor emptyDescriptor;
  EXPECT_TRUE(emptyDescriptor.meshName.empty());
  EXPECT_EQ(nullptr, emptyDescriptor.mesh);

  // describe mesh by name
  MeshDescriptor sphereDescriptor("unit_sphere");
  EXPECT_EQ("unit_sphere", sphereDescriptor.meshName);
  EXPECT_EQ(nullptr, sphereDescriptor.mesh);
  // load the mesh
  sphereDescriptor.Load();
  EXPECT_EQ("unit_sphere", sphereDescriptor.meshName);
  EXPECT_NE(nullptr, sphereDescriptor.mesh);
  EXPECT_EQ("unit_sphere", sphereDescriptor.mesh->Name());

  // describe mesh by common mesh obj
  common::MeshManager *meshManager = common::MeshManager::Instance();
  ASSERT_NE(nullptr,  meshManager);
  const common::Mesh *comMesh = meshManager->MeshByName("unit_cylinder");
  MeshDescriptor cylinderDescriptor(comMesh);
  EXPECT_TRUE(cylinderDescriptor.meshName.empty());
  EXPECT_NE(nullptr, cylinderDescriptor.mesh);
  EXPECT_EQ("unit_cylinder", cylinderDescriptor.mesh->Name());
  // update the mesh name
  cylinderDescriptor.Load();
  EXPECT_EQ("unit_cylinder", cylinderDescriptor.meshName);
  EXPECT_NE(nullptr, cylinderDescriptor.mesh);
  EXPECT_EQ("unit_cylinder", cylinderDescriptor.mesh->Name());

  // Clean up
  engine->DestroyScene(scene);
}
