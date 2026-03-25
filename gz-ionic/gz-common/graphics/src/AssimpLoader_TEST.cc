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

#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/AssimpLoader.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;
class AssimpLoader : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices, 24 unique, 12 shared.
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read a submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, Material)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat != nullptr);

  // Make sure we read the specular value
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), mat->Ambient());
  EXPECT_EQ(math::Color(0.64f, 0.64f, 0.64f, 1.0f), mat->Diffuse());
  EXPECT_EQ(math::Color(0.5, 0.5, 0.5, 1.0), mat->Specular());
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), mat->Emissive());
  EXPECT_DOUBLE_EQ(50.0, mat->Shininess());
  // transparent: opaque="A_ONE", color=[1 1 1 1]
  // transparency: 1.0
  // resulting transparency value = (1 - color.a * transparency)
  EXPECT_DOUBLE_EQ(0.0, mat->Transparency());
  double srcFactor = -1;
  double dstFactor = -1;
  mat->BlendFactors(srcFactor, dstFactor);
  EXPECT_DOUBLE_EQ(1.0, srcFactor);
  EXPECT_DOUBLE_EQ(0.0, dstFactor);

  common::Mesh *meshOpaque = loader.Load(
      common::testing::TestFile("data", "box_opaque.dae"));
  ASSERT_TRUE(meshOpaque);

  EXPECT_EQ(meshOpaque->MaterialCount(), 1u);

  common::MaterialPtr matOpaque = meshOpaque->MaterialByIndex(0u);
  ASSERT_TRUE(matOpaque != nullptr);

  // Make sure we read the specular value
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), matOpaque->Ambient());
  EXPECT_EQ(math::Color(0.64f, 0.64f, 0.64f, 1.0f), matOpaque->Diffuse());
  EXPECT_EQ(math::Color(0.5, 0.5, 0.5, 1.0), matOpaque->Specular());
  EXPECT_EQ(math::Color(0.0, 0.0, 0.0, 1.0), matOpaque->Emissive());
  EXPECT_DOUBLE_EQ(50.0, matOpaque->Shininess());
  // transparent: opaque="A_ONE", color=[1 1 1 1]
  // transparency: not specified, defaults to 1.0
  // resulting transparency value = (1 - color.a * transparency)
  EXPECT_DOUBLE_EQ(0.0, matOpaque->Transparency());
  srcFactor = -1;
  dstFactor = -1;
  matOpaque->BlendFactors(srcFactor, dstFactor);
  EXPECT_DOUBLE_EQ(1.0, srcFactor);
  EXPECT_DOUBLE_EQ(0.0, dstFactor);
  delete mesh;
  delete meshOpaque;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, ShareVertices)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.dae"));

  // check number of shared vertices
  std::set<unsigned int> uniqueIndices;
  int shared = 0;
  for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
  {
    const std::shared_ptr<common::SubMesh> subMesh =
      mesh->SubMeshByIndex(i).lock();
    for (unsigned int j = 0; j < subMesh->IndexCount(); ++j)
    {
      if (uniqueIndices.find(subMesh->Index(j)) == uniqueIndices.end())
      {
        uniqueIndices.insert(subMesh->Index(j));
      }
      else
      {
        shared++;
      }
    }
  }
  EXPECT_EQ(shared, 12);
  EXPECT_EQ(uniqueIndices.size(), 24u);

  // check all vertices are unique
  for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
  {
    const std::shared_ptr<common::SubMesh> subMesh =
      mesh->SubMeshByIndex(i).lock();
    for (unsigned int j = 0; j < subMesh->VertexCount(); ++j)
    {
      math::Vector3d v = subMesh->Vertex(j);
      math::Vector3d n = subMesh->Normal(j);

      // Verify there is no other vertex with the same position AND normal
      for (unsigned int k = j+1; k < subMesh->VertexCount(); ++k)
      {
        if (v == subMesh->Vertex(k))
        {
          EXPECT_TRUE(n != subMesh->Normal(k));
        }
      }
    }
  }
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadZeroCount)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "zero_count.dae"));
  ASSERT_TRUE(mesh);
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, TexCoordSets)
{
  common::AssimpLoader loader;
  // This triangle mesh has multiple uv sets and vertices separated by
  // line breaks
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.dae"));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(6u, mesh->VertexCount());
  EXPECT_EQ(6u, mesh->NormalCount());
  EXPECT_EQ(6u, mesh->IndexCount());
  EXPECT_EQ(6u, mesh->TexCoordCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(0, 0, 0), subMesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(10, 10, 0), subMesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(2u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(2u));

  auto smb = mesh->SubMeshByIndex(1u);
  auto subMeshB = smb.lock();
  EXPECT_NE(nullptr, subMeshB);
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMeshB->Vertex(0u));
  EXPECT_EQ(math::Vector3d(20, 0, 0), subMeshB->Vertex(1u));
  EXPECT_EQ(math::Vector3d(20, 10, 0), subMeshB->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(2u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(2u));

  EXPECT_TRUE(subMeshB->HasTexCoord(0u));
  EXPECT_TRUE(subMeshB->HasTexCoord(1u));
  EXPECT_TRUE(subMeshB->HasTexCoord(2u));
  EXPECT_FALSE(subMeshB->HasTexCoord(3u));

  // test texture coordinate set API
  EXPECT_EQ(3u, subMeshB->TexCoordSetCount());
  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(0u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(2u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoordBySet(2u, 0u));

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 0u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 0u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 0u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 0u));

  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(1u));
  EXPECT_EQ(math::Vector2d(0, 0.5), subMeshB->TexCoordBySet(0u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.4), subMeshB->TexCoordBySet(1u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.3), subMeshB->TexCoordBySet(2u, 1u));

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 1u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 1u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 1u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 1u));

  EXPECT_EQ(3u, subMeshB->TexCoordCountBySet(2u));
  EXPECT_EQ(math::Vector2d(0, 0.8), subMeshB->TexCoordBySet(0u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.7), subMeshB->TexCoordBySet(1u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.6), subMeshB->TexCoordBySet(2u, 2u));

  EXPECT_TRUE(subMeshB->HasTexCoordBySet(0u, 2u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(1u, 2u));
  EXPECT_TRUE(subMeshB->HasTexCoordBySet(2u, 2u));
  EXPECT_FALSE(subMeshB->HasTexCoordBySet(3u, 2u));

  subMeshB->SetTexCoordBySet(2u, math::Vector2d(0.1, 0.2), 1u);
  EXPECT_EQ(math::Vector2d(0.1, 0.2), subMeshB->TexCoordBySet(2u, 1u));
  delete mesh;
}

// Test fails for assimp below 5.2.0
#ifndef GZ_ASSIMP_PRE_5_2_0
/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithAnimationOutsideSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "box_with_animation_outside_skeleton.dae"));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_LT(0u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, skeleton->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Armature"));
  auto nodeAnimation = anim->NodeAnimationByName("Armature");
  EXPECT_NE(nullptr, nodeAnimation);
  EXPECT_EQ("Armature", nodeAnimation->Name());
  auto poseStart = anim->PoseAt(0.04166662);
  math::Matrix4d expectedTrans = math::Matrix4d(
      1, 0, 0, 1,
      0, 1, 0, -1,
      0, 0, 1, 0,
      0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseStart.at("Armature"));
  auto poseEnd = anim->PoseAt(1.666666);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Armature"));
  delete mesh;
}
#endif

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxInstControllerWithoutSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "box_inst_controller_without_skeleton.dae"));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_LT(0u, skeleton->NodeCount());
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxMultipleInstControllers)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_multiple_inst_controllers.dae"));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(48u, mesh->VertexCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(48u, mesh->TexCoordCount());

  std::shared_ptr<common::SubMesh> submesh = mesh->SubMeshByIndex(0).lock();
  std::shared_ptr<common::SubMesh> submesh2 = mesh->SubMeshByIndex(1).lock();
  EXPECT_EQ(36u, submesh->IndexCount());
  EXPECT_EQ(36u, submesh2->IndexCount());
  EXPECT_EQ(24u, submesh->VertexCount());
  EXPECT_EQ(24u, submesh2->VertexCount());
  EXPECT_EQ(24u, submesh->TexCoordCount());
  EXPECT_EQ(24u, submesh2->TexCoordCount());

  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  EXPECT_LT(0u, skeleton->NodeCount());
  EXPECT_NE(nullptr, skeleton->NodeById("Armature_Bone"));
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxNestedAnimation)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_nested_animation.dae"));

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  // Depends on fix in assimp main branch for nested animation naming
  // TODO(luca) Fix is merged in assimp main, add when it is re-released
  // EXPECT_EQ(anim->Name(), "Armature");
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Armature_Bone"));
  auto nodeAnimation = anim->NodeAnimationByName("Armature_Bone");
  ASSERT_NE(nullptr, nodeAnimation);
  EXPECT_EQ("Armature_Bone", nodeAnimation->Name());
  auto poseStart = anim->PoseAt(0);
  math::Matrix4d expectedTrans = math::Matrix4d(
      1, 0, 0, 1,
      0, 1, 0, -1,
      0, 0, 1, 0,
      0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseStart.at("Armature_Bone"));
  auto poseEnd = anim->PoseAt(1.666666);
  expectedTrans = math::Matrix4d(
        1, 0, 0, 2,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1);
  EXPECT_EQ(expectedTrans, poseEnd.at("Armature_Bone"));
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithDefaultStride)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_default_stride.dae"));
  ASSERT_NE(mesh, nullptr);

  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  ASSERT_NE(mesh->MeshSkeleton(), nullptr);
  // TODO(luca) not working, investigate
  // ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithMultipleGeoms)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_multiple_geoms.dae"));

  EXPECT_EQ(72u, mesh->IndexCount());
  EXPECT_EQ(48u, mesh->VertexCount());
  EXPECT_EQ(1u, mesh->MaterialCount());
  EXPECT_EQ(48u, mesh->TexCoordCount());
  ASSERT_EQ(1u, mesh->MeshSkeleton()->AnimationCount());
  ASSERT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(24u, mesh->SubMeshByIndex(0).lock()->NodeAssignmentsCount());
  EXPECT_EQ(0u, mesh->SubMeshByIndex(1).lock()->NodeAssignmentsCount());
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadBoxWithHierarchicalNodes)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_hierarchical_nodes.dae"));

  ASSERT_EQ(5u, mesh->SubMeshCount());

  // node by itself
  EXPECT_EQ("StaticCube", mesh->SubMeshByIndex(0).lock()->Name());

  // nested node with no name so it takes the parent's name instead
  EXPECT_EQ("StaticCubeParent", mesh->SubMeshByIndex(1).lock()->Name());

  // parent node containing child node with no name
  // CHANGE Assimp assigns the id to the name if the mesh has no name
  EXPECT_EQ("StaticCubeNestedNoName", mesh->SubMeshByIndex(2).lock()->Name());

  // Parent of nested node with name
  EXPECT_EQ("StaticCubeParent2", mesh->SubMeshByIndex(3).lock()->Name());

  // nested node with name
  EXPECT_EQ("StaticCubeNested", mesh->SubMeshByIndex(4).lock()->Name());
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, MergeBoxWithDoubleSkeleton)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_with_double_skeleton.dae"));
  ASSERT_TRUE(mesh->HasSkeleton());
  auto skeleton_ptr = mesh->MeshSkeleton();
  // The two skeletons have been joined and their root is the
  // animation root, called Scene
  EXPECT_EQ(skeleton_ptr->RootNode()->Name(), std::string("Scene"));
  delete mesh;
}

// For assimp below 5.2.0 mesh loading fails because of
// failing to parse the empty <author> tag
#ifndef GZ_ASSIMP_PRE_5_2_0
/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadCylinderAnimatedFrom3dsMax)
{
  // TODO(anyone) This test shows that the mesh loads without crashing, but the
  // mesh animation looks deformed when loaded. That still needs to be
  // addressed.
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "cylinder_animated_from_3ds_max.dae"));

  EXPECT_EQ("unknown", mesh->Name());
  EXPECT_EQ(194u, mesh->VertexCount());
  EXPECT_EQ(194u, mesh->NormalCount());
  EXPECT_EQ(852u, mesh->IndexCount());
  EXPECT_LT(0u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  EXPECT_EQ(1u, mesh->SubMeshCount());
  auto subMesh = mesh->SubMeshByIndex(0);
  ASSERT_NE(nullptr, subMesh.lock());
  EXPECT_EQ("Cylinder01", subMesh.lock()->Name());

  EXPECT_TRUE(mesh->HasSkeleton());
  auto skeleton = mesh->MeshSkeleton();
  ASSERT_NE(nullptr, skeleton);
  ASSERT_EQ(1u, skeleton->AnimationCount());

  auto anim = skeleton->Animation(0);
  ASSERT_NE(nullptr, anim);
  // TODO(luca) Fix is merged in assimp main, add when it is re-released
  // EXPECT_EQ("Bone02", anim->Name());
  EXPECT_EQ(1u, anim->NodeCount());
  EXPECT_TRUE(anim->HasNode("Bone02"));
  delete mesh;
}
#endif

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadObjBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.obj"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());
  // 36 vertices after triangulation, assimp optimizes to 24
  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
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
TEST_F(AssimpLoader, ObjInvalidMaterial)
{
  common::AssimpLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "invalid_material.obj");

  common::Mesh *mesh = loader.Load(meshFilename);

  EXPECT_TRUE(mesh != nullptr);
  delete mesh;
}

/////////////////////////////////////////////////
// Open a non existing file
TEST_F(AssimpLoader, NonExistingMesh)
{
  common::AssimpLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "non_existing_mesh.glb");

  common::Mesh *mesh = loader.Load(meshFilename);

  EXPECT_EQ(mesh->SubMeshCount(), 0);
  delete mesh;
}

/////////////////////////////////////////////////
// This test opens a FBX file
TEST_F(AssimpLoader, LoadFbxBox)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.fbx"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(100, 100, 100), mesh->Max());
  EXPECT_EQ(math::Vector3d(-100, -100, -100), mesh->Min());

  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.8f, 0.8f, 0.8f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.8f, 0.8f, 0.8f, 1.0f));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
  delete mesh;
}

/////////////////////////////////////////////////
// This test opens a GLB file
TEST_F(AssimpLoader, LoadGlTF2Box)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());

  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(mat->Ambient(), math::Color(0.4f, 0.4f, 0.4f, 1.0f));
  EXPECT_EQ(mat->Diffuse(), math::Color(0.8f, 0.8f, 0.8f, 1.0f));
  EXPECT_EQ(mat->Specular(), math::Color(0.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_DOUBLE_EQ(mat->Transparency(), 0.0);
  delete mesh;
}

/////////////////////////////////////////////////
// Open a gltf mesh with an external texture
TEST_F(AssimpLoader, LoadGlTF2BoxExternalTexture)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "gltf", "PurpleCube.gltf"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());

  // Make sure we can read the submesh name
  EXPECT_STREQ("PurpleCube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());
  // Data is now loaded in memory
  EXPECT_NE(nullptr, mat->TextureData());
  auto testTextureFile =
    common::testing::TestFile("data/gltf", "PurpleCube_Diffuse.png");
  EXPECT_EQ(testTextureFile + "_Diffuse", mat->TextureImage());
  delete mesh;
}

/////////////////////////////////////////////////
// Open a gltf mesh with transmission extension
TEST_F(AssimpLoader, LoadGlTF2BoxTransmission)
{
#ifndef GZ_ASSIMP_PRE_5_1_0
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_transmission.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  EXPECT_EQ(mesh->MaterialCount(), 1u);

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());
  // transmission currently modeled as transparency
  EXPECT_FLOAT_EQ(0.1f, mat->Transparency());
  delete mesh;
#endif
}

/////////////////////////////////////////////////
// This test loads a box glb mesh with embedded compressed jpeg texture
TEST_F(AssimpLoader, LoadGlTF2BoxWithJPEGTexture)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "box_texture_jpg.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());
  EXPECT_EQ(math::Vector3d(1, 1, 1), mesh->Max());
  EXPECT_EQ(math::Vector3d(-1, -1, -1), mesh->Min());

  EXPECT_EQ(24u, mesh->VertexCount());
  EXPECT_EQ(24u, mesh->NormalCount());
  EXPECT_EQ(36u, mesh->IndexCount());
  EXPECT_EQ(24u, mesh->TexCoordCount());
  EXPECT_EQ(1u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  // Make sure we can read the submesh name
  EXPECT_STREQ("Cube", mesh->SubMeshByIndex(0).lock()->Name().c_str());

  const common::MaterialPtr mat = mesh->MaterialByIndex(0u);
  ASSERT_TRUE(mat.get());

  // Make sure we read the material color values
  EXPECT_EQ(math::Color(0.4f, 0.4f, 0.4f, 1.0f), mat->Ambient());
  EXPECT_EQ(math::Color(1.0f, 1.0f, 1.0f, 1.0f), mat->Diffuse());
  EXPECT_EQ(math::Color(0.0f, 0.0f, 0.0f, 1.0f), mat->Specular());
  // Use the new globally unique canonical name format
  std::string expectedName = common::testing::TestFile("data",
      "box_texture_jpg.glb") + "#*0_Diffuse";
  EXPECT_EQ(expectedName, mat->TextureImage());
  EXPECT_NE(nullptr, mat->TextureData());
  delete mesh;
}

/////////////////////////////////////////////////
// Use a fully featured glb test asset, including PBR textures, emissive maps
// embedded textures, lightmaps, animations to test advanced glb features
TEST_F(AssimpLoader, LoadGlbPbrAsset)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data", "fully_featured.glb"));

  EXPECT_STREQ("unknown", mesh->Name().c_str());

  EXPECT_EQ(mesh->SubMeshCount(), 7);
  EXPECT_STREQ("Floor", mesh->SubMeshByIndex(0).lock()->Name().c_str());
  EXPECT_STREQ("SquareShelf", mesh->SubMeshByIndex(1).lock()->Name().c_str());
  EXPECT_STREQ("OpenRoboticsLogo.002",
      mesh->SubMeshByIndex(2).lock()->Name().c_str());
  EXPECT_STREQ("OpenRoboticsLogo.001",
      mesh->SubMeshByIndex(3).lock()->Name().c_str());
  EXPECT_STREQ("EmissiveCube", mesh->SubMeshByIndex(4).lock()->Name().c_str());
  EXPECT_STREQ("OpenCola", mesh->SubMeshByIndex(5).lock()->Name().c_str());
  EXPECT_STREQ("OpenRoboticsLogo",
      mesh->SubMeshByIndex(6).lock()->Name().c_str());

  // Emissive cube has an embedded emissive texture
  auto materialId = mesh->SubMeshByIndex(4).lock()->GetMaterialIndex();
  ASSERT_TRUE(materialId.has_value());
  auto material = mesh->MaterialByIndex(materialId.value());
  ASSERT_NE(material, nullptr);
  auto pbr = material->PbrMaterial();
  ASSERT_NE(pbr, nullptr);
  EXPECT_NE(pbr->EmissiveMapData(), nullptr);

  // SquareShelf has full PBR textures, including metallicroughness
  // and ambient occlusion
  materialId = mesh->SubMeshByIndex(1).lock()->GetMaterialIndex();
  ASSERT_TRUE(materialId.has_value());
  material = mesh->MaterialByIndex(materialId.value());
  ASSERT_NE(material, nullptr);
  pbr = material->PbrMaterial();
  ASSERT_NE(pbr, nullptr);

  // Check the texture data itself
  auto img = material->TextureData();
  ASSERT_NE(img, nullptr);
  EXPECT_EQ(img->Width(), 512);
  EXPECT_EQ(img->Height(), 512);
  // A black and a white pixel
  EXPECT_EQ(img->Pixel(0, 0), math::Color(0.0f, 0.0f, 0.0f, 1.0f));
  EXPECT_EQ(img->Pixel(100, 100), math::Color(1.0f, 1.0f, 1.0f, 1.0f));

  EXPECT_NE(pbr->NormalMapData(), nullptr);
  // Metallic roughness and alpha from textures only works in assimp > 5.2.0
#ifndef GZ_ASSIMP_PRE_5_2_0
  // Alpha from textures
  EXPECT_TRUE(material->TextureAlphaEnabled());
  EXPECT_TRUE(material->TwoSidedEnabled());
  EXPECT_EQ(material->AlphaThreshold(), 0.5);
  // Metallic and roughness maps
  EXPECT_NE(pbr->MetalnessMapData(), nullptr);
  EXPECT_NE(pbr->RoughnessMapData(), nullptr);
  // Check pixel values to test metallicroughness texture splitting
  EXPECT_FLOAT_EQ(pbr->MetalnessMapData()->Pixel(256, 256).R(), 0.0f);
  EXPECT_FLOAT_EQ(pbr->RoughnessMapData()->Pixel(256, 256).R(),
                  124.0f / 255.0f);

  // Bug in assimp 5.0.x that doesn't parse coordinate sets properly
  // \todo(iche033) Lightmaps are disabled for glb meshes
  // due to upstream bug
  // EXPECT_EQ(pbr->LightMapTexCoordSet(), 1);
#endif

  // \todo(iche033) Lightmaps are disabled for glb meshes
  // due to upstream bug
  // EXPECT_NE(pbr->LightMapData(), nullptr);

  // Mesh has 3 animations
  auto skel = mesh->MeshSkeleton();
  ASSERT_NE(skel, nullptr);
  ASSERT_EQ(skel->AnimationCount(), 3);
  EXPECT_STREQ("Action1", skel->Animation(0)->Name().c_str());
  EXPECT_STREQ("Action2", skel->Animation(1)->Name().c_str());
  EXPECT_STREQ("Action3", skel->Animation(2)->Name().c_str());
  delete mesh;
}

/////////////////////////////////////////////////
// Checks for null root node animation and valid
// x displacement in non root node's animation.
TEST_F(AssimpLoader, CheckNonRootDisplacement)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(common::testing::TestFile("data",
        "walk.dae"));
  auto meshSkel =  mesh->MeshSkeleton();
  std::string rootNodeName = meshSkel->RootNode()->Name();
  common::SkeletonAnimation *skelAnim = meshSkel->Animation(0);
  common::NodeAnimation *rootNode = skelAnim->NodeAnimationByName(rootNodeName);
  EXPECT_EQ(nullptr, rootNode);
  auto xDisplacement = skelAnim->XDisplacement();
  ASSERT_TRUE(xDisplacement);
  delete mesh;
}

/////////////////////////////////////////////////
TEST_F(AssimpLoader, LoadGLTF2Triangle)
{
  common::AssimpLoader loader;
  common::Mesh *mesh = loader.Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.glb"));
  ASSERT_TRUE(mesh);

  EXPECT_EQ(6u, mesh->VertexCount());
  EXPECT_EQ(6u, mesh->NormalCount());
  EXPECT_EQ(6u, mesh->IndexCount());
  EXPECT_EQ(6u, mesh->TexCoordCount());
  EXPECT_EQ(2u, mesh->SubMeshCount());
  EXPECT_EQ(1u, mesh->MaterialCount());

  auto sm = mesh->SubMeshByIndex(0u);
  auto subMesh = sm.lock();
  EXPECT_NE(nullptr, subMesh);
  EXPECT_EQ(math::Vector3d(0, 0, 0), subMesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(10, 10, 0), subMesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMesh->Normal(2u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMesh->TexCoord(2u));

  auto smb = mesh->SubMeshByIndex(1u);
  auto subMeshB = smb.lock();
  EXPECT_NE(nullptr, subMeshB);
  EXPECT_EQ(math::Vector3d(10, 0, 0), subMeshB->Vertex(0u));
  EXPECT_EQ(math::Vector3d(20, 0, 0), subMeshB->Vertex(1u));
  EXPECT_EQ(math::Vector3d(20, 10, 0), subMeshB->Vertex(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), subMeshB->Normal(2u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(0u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(1u));
  EXPECT_EQ(math::Vector2d(0, 1), subMeshB->TexCoord(2u));
  delete mesh;
}

/////////////////////////////////////////////////
// Load animation without a name
TEST_F(AssimpLoader, NoAnimName)
{
  common::AssimpLoader loader;

  std::string meshFilename =
    common::testing::TestFile("data", "box_with_no_animation_name.dae");

  common::Mesh *mesh = loader.Load(meshFilename);
  common::SkeletonPtr skeleton = mesh->MeshSkeleton();
  ASSERT_EQ(1u, skeleton->AnimationCount());
  common::SkeletonAnimation *anim = skeleton->Animation(0);
  auto animName = anim->Name();
  EXPECT_EQ(animName, "animation1");
}
