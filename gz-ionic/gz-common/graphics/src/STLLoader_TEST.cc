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

#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/Material.hh"
#include "gz/common/STLLoader.hh"
#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class STLLoaderTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(STLLoaderTest, LoadSTL)
{
  common::STLLoader loader;
  auto mesh = loader.Load("");
  EXPECT_EQ(nullptr, mesh);

  mesh = loader.Load(
      common::testing::TestFile("data", "cube.stl"));
  EXPECT_NE(nullptr, mesh);

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(20, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(0, -20, 0), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_EQ(36u, mesh->VertexCount());
  EXPECT_EQ(36u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(20, 0, 0), subMesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(0, -20, 0), subMesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 0), subMesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(2u));

  EXPECT_STREQ("", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  delete mesh;

  mesh = loader.Load(
      common::testing::TestFile("data", "cube_binary.stl"));
  EXPECT_NE(nullptr, mesh);

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(20, 0, 20), mesh->Max());
  EXPECT_EQ(math::Vector3d(0, -20, 0), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_EQ(36u, mesh->VertexCount());
  EXPECT_EQ(36u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  sm = mesh->SubMeshByIndex(0u);
  subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(20, 0, 0), subMesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(0, -20, 0), subMesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 0), subMesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, -1), subMesh->Normal(2u));

  EXPECT_STREQ("", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  delete mesh;
}
