/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#include <fstream>

#include <gtest/gtest.h>

#include "gz/common/ColladaLoader.hh"
#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/MeshManager.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/math/Vector3.hh"

#include <gz/common/testing/AutoLogFixture.hh>
#include <gz/common/testing/TestPaths.hh>

using namespace gz;

class MeshTest : public common::testing::AutoLogFixture { };

constexpr const char asciiSTLBox[] =
R"(solid MYSOLID
  facet normal  0.0   0.0  -1.0
    outer loop
      vertex    0.0   0.0   0.0
      vertex    1.0   1.0   0.0
      vertex    1.0   0.0   0.0
    endloop
  endfacet
  facet normal  0.0   0.0  -1.0
    outer loop
      vertex    0.0   0.0   0.0
      vertex    0.0   1.0   0.0
      vertex    1.0   1.0   0.0
    endloop
  endfacet
  facet normal -1.0   0.0   0.0
    outer loop
      vertex    0.0   0.0   0.0
      vertex    0.0   1.0   1.0
      vertex    0.0   1.0   0.0
    endloop
  endfacet
  facet normal -1.0   0.0   0.0
    outer loop
      vertex    0.0   0.0   0.0
      vertex    0.0   0.0   1.0
      vertex    0.0   1.0   1.0
    endloop
  endfacet
  facet normal  0.0   1.0   0.0
    outer loop
      vertex    0.0   1.0   0.0
      vertex    1.0   1.0   1.0
      vertex    1.0   1.0   0.0
    endloop
  endfacet
  facet normal  0.0   1.0   0.0
    outer loop
      vertex    0.0   1.0   0.0
      vertex    0.0   1.0   1.0
      vertex    1.0   1.0   1.0
    endloop
  endfacet
  facet normal  1.0   0.0   0.0
    outer loop
      vertex    1.0   0.0   0.0
      vertex    1.0   1.0   0.0
      vertex    1.0   1.0   1.0
    endloop
  endfacet
  facet normal  1.0   0.0   0.0
    outer loop
      vertex    1.0   0.0   0.0
      vertex    1.0   1.0   1.0
      vertex    1.0   0.0   1.0
    endloop
  endfacet
  facet normal  0.0  -1.0   0.0
    outer loop
      vertex    0.0   0.0   0.0
      vertex    1.0   0.0   0.0
      vertex    1.0   0.0   1.0
    endloop
  endfacet
  facet normal  0.0  -1.0   0.0
    outer loop
      vertex    0.0   0.0   0.0
      vertex    1.0   0.0   1.0
      vertex    0.0   0.0   1.0
    endloop
  endfacet
  facet normal  0.0   0.0   1.0
    outer loop
      vertex    0.0   0.0   1.0
      vertex    1.0   0.0   1.0
      vertex    1.0   1.0   1.0
    endloop
  endfacet
  facet normal  0.0   0.0   1.0
    outer loop
      vertex    0.0   0.0   1.0
      vertex    1.0   1.0   1.0
      vertex    0.0   1.0   1.0
    endloop
  endfacet
endsolid MYSOLID)";


/////////////////////////////////////////////////
TEST_F(MeshTest, Load)
{
  // Load things that we know are invalid extensions
  EXPECT_EQ(nullptr, common::MeshManager::Instance()->Load("break.mesh"));
  EXPECT_EQ(nullptr, common::MeshManager::Instance()->Load("break.3ds"));
  EXPECT_EQ(nullptr, common::MeshManager::Instance()->Load("break.xml"));

  common::systemPaths()->AddFilePaths(common::testing::TestFile("data"));

  // Loading should be successful
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.dae"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.obj"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.fbx"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.glb"));

  // Reloading should not cause errors
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.dae"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.obj"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.fbx"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.glb"));

  // Forget about previously loaded meshes
  EXPECT_TRUE(common::MeshManager::Instance()->RemoveMesh("box.dae"));
  EXPECT_TRUE(common::MeshManager::Instance()->RemoveMesh("box.obj"));
  EXPECT_TRUE(common::MeshManager::Instance()->RemoveMesh("box.fbx"));
  EXPECT_TRUE(common::MeshManager::Instance()->RemoveMesh("box.glb"));

  // When forcing assimp, loading should still be successful for all formats
  common::setenv("GZ_MESH_FORCE_ASSIMP", "true");
  common::MeshManager::Instance()->SetAssimpEnvs();
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.dae"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.obj"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.fbx"));
  EXPECT_NE(nullptr, common::MeshManager::Instance()->Load("box.glb"));
  EXPECT_EQ(nullptr, common::MeshManager::Instance()->Load("break.xml"));
  common::unsetenv("GZ_MESH_FORCE_ASSIMP");
}

/////////////////////////////////////////////////
TEST_F(MeshTest, Access)
{
  auto mesh = common::MeshManager::Instance()->MeshByName("unit_box");
  ASSERT_NE(nullptr, mesh);
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(0u, mesh->MaterialCount());

  gz::math::Vector3d center, min, max;
  mesh->AABB(center, min, max);
  EXPECT_TRUE(center == gz::math::Vector3d(0, 0, 0));
  EXPECT_TRUE(min == gz::math::Vector3d(-.5, -.5, -.5));
  EXPECT_TRUE(max == gz::math::Vector3d(.5, .5, .5));

  double *vertArray = nullptr;
  int *indArray = nullptr;
  mesh->FillArrays(&vertArray, &indArray);

  int i = 0;
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(.5, vertArray[i++]);
  EXPECT_FLOAT_EQ(-.5, vertArray[i++]);

  delete [] indArray;
  delete [] vertArray;

  EXPECT_FALSE(common::MeshManager::Instance()->HasMesh(""));
  EXPECT_TRUE(common::MeshManager::Instance()->HasMesh("unit_box"));

  mesh = common::MeshManager::Instance()->MeshByName("foo_box_doesnt_exist");
  EXPECT_EQ(nullptr, mesh);
  EXPECT_FALSE(
      common::MeshManager::Instance()->RemoveMesh("foo_box_doesnt_exist"));
  EXPECT_TRUE(common::MeshManager::Instance()->RemoveMesh("unit_box"));
}

/////////////////////////////////////////////////
TEST_F(MeshTest, RoundtripStl)
{
  std::ofstream stlFile("gz_stl_test.stl", std::ios::out);
  stlFile << asciiSTLBox;
  stlFile.close();

  auto mesh =
    common::MeshManager::Instance()->Load("gz_stl_test-bad.stl");
  EXPECT_EQ(nullptr, mesh);

  common::systemPaths()->AddFilePaths(common::cwd());
  mesh = common::MeshManager::Instance()->Load("gz_stl_test.stl");
  ASSERT_NE(nullptr, mesh);

  math::Vector3d center, min, max;
  mesh->AABB(center, min, max);
  EXPECT_TRUE(center == gz::math::Vector3d(0.5, 0.5, 0.5));
  EXPECT_TRUE(min == gz::math::Vector3d(0, 0, 0));
  EXPECT_TRUE(max == gz::math::Vector3d(1, 1, 1));
}

/////////////////////////////////////////////////
TEST_F(MeshTest, Export)
{
  std::ofstream stlFile("gz_stl_test.stl", std::ios::out);
  stlFile << asciiSTLBox;
  stlFile.close();

  common::systemPaths()->AddFilePaths(common::cwd());
  auto mesh = common::MeshManager::Instance()->Load("gz_stl_test.stl");
  ASSERT_NE(nullptr, mesh);

  common::MeshManager::Instance()->Export(mesh,
      common::joinPaths(common::cwd(), "gz_stl_test2"), "stl", false);
  common::MeshManager::Instance()->Export(mesh,
      common::joinPaths(common::cwd(), "gz_stl_test2"), "dae", false);

  EXPECT_FALSE(common::exists("gz_stl_test2.stl"));
  EXPECT_TRUE(common::exists("gz_stl_test2.dae"));
}
