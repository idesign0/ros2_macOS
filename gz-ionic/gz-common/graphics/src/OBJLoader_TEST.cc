/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include "gz/common/OBJLoader.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class OBJLoaderTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(OBJLoaderTest, LoadObjBox)
{
  common::OBJLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.obj"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(gz::math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(gz::math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices
  EXPECT_EQ(36u, mesh->VertexCount());
  EXPECT_EQ(36u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube_Cube.001", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.0, 0.0, 0.0, 1.0));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.512f, 0.512f, 0.512f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.25, 0.25, 0.25, 1.0));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);

  delete mesh;
}

/////////////////////////////////////////////////
// This tests opening an OBJ file that has an invalid material reference
TEST_F(OBJLoaderTest, InvalidMaterial)
{
  gz::common::OBJLoader objLoader;

  std::string meshFilename =
    common::testing::TestFile("data", "invalid_material.obj");

  gz::common::Mesh *mesh = objLoader.Load(meshFilename);

  EXPECT_TRUE(mesh != nullptr);
  delete mesh;
}

/////////////////////////////////////////////////
// This tests opening an OBJ file that has PBR fields
TEST_F(OBJLoaderTest, PBR)
{
  gz::common::OBJLoader objLoader;

  // load obj file exported by 3ds max that has pbr extension
  {
    std::string meshFilename =
      common::testing::TestFile("data", "cube_pbr.obj");

    gz::common::Mesh *mesh = objLoader.Load(meshFilename);
    EXPECT_NE(nullptr, mesh);

    const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
    ASSERT_TRUE(mat.get());

    EXPECT_EQ(math::Color(0.0f, 0.0f, 0.0f, 1.0f), mat->Ambient());
    EXPECT_EQ(math::Color(0.5f, 0.5f, 0.5f, 1.0f), mat->Diffuse());
    EXPECT_EQ(math::Color(1.0f, 1.0f, 1.0f, 1.0f), mat->Specular());
    EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
    EXPECT_NE(std::string::npos,
        mat->TextureImage().find("LightDome_Albedo.png"));
    const common::Pbr *pbr = mat->PbrMaterial();
    EXPECT_DOUBLE_EQ(0, pbr->Roughness());
    EXPECT_DOUBLE_EQ(0, pbr->Metalness());
    EXPECT_EQ("LightDome_Metalness.png", pbr->MetalnessMap());
    EXPECT_EQ("LightDome_Roughness.png", pbr->RoughnessMap());
    EXPECT_EQ("LightDome_Normal.png", pbr->NormalMap());
    delete mesh;
  }

  // load obj file exported by blender - it shoves pbr maps into
  // existing fields
  {
    std::string meshFilename =
      common::testing::TestFile("data", "blender_pbr.obj");

    gz::common::Mesh *mesh = objLoader.Load(meshFilename);
    EXPECT_NE(nullptr, mesh);

    const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
    ASSERT_TRUE(mat.get());

    EXPECT_EQ(math::Color(1.0f, 1.0f, 1.0f, 1.0f), mat->Ambient());
    EXPECT_EQ(math::Color(0.8f, 0.8f, 0.8f, 1.0f), mat->Diffuse());
    EXPECT_EQ(math::Color(0.5f, 0.5f, 0.5f, 1.0f), mat->Specular());
    EXPECT_EQ(math::Color(0.0f, 0.0f, 0.0f, 1.0f), mat->Emissive());
    EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
    EXPECT_NE(std::string::npos,
        mat->TextureImage().find("mesh_Diffuse.png"));
    const common::Pbr *pbr = mat->PbrMaterial();
    EXPECT_DOUBLE_EQ(0, pbr->Roughness());
    EXPECT_DOUBLE_EQ(0, pbr->Metalness());
    EXPECT_EQ("mesh_Metal.png", pbr->MetalnessMap());
    EXPECT_EQ("mesh_Rough.png", pbr->RoughnessMap());
    EXPECT_EQ("mesh_Normal.png", pbr->NormalMap());
    delete mesh;
  }
}
