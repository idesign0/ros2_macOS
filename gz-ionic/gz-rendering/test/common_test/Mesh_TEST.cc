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
#include <memory>
#include <string>

#include "CommonRenderingTest.hh"

#include <gz/common/MeshManager.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SkeletonAnimation.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Mesh.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class MeshTest : public CommonRenderingTest
{
  public: const std::string TEST_MEDIA_PATH =
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media", "meshes");
};

/////////////////////////////////////////////////
TEST_F(MeshTest, MeshSubMesh)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // create the mesh using mesh descriptor
  MeshDescriptor descriptor("unit_box");
  MeshPtr mesh = scene->CreateMesh(descriptor);
  ASSERT_NE(nullptr, mesh);

  // test mesh API
  EXPECT_EQ(mesh->SubMeshCount(), 1u);

  SubMeshPtr submesh = mesh->SubMeshByIndex(0u);
  ASSERT_NE(nullptr, submesh);

  EXPECT_TRUE(mesh->HasSubMesh(submesh));
  EXPECT_TRUE(mesh->HasSubMeshName(submesh->Name()));

  EXPECT_EQ(submesh, mesh->SubMeshByName(submesh->Name()));

  EXPECT_FALSE(mesh->HasSkeleton());

  // test submesh API
  MaterialPtr mat = submesh->Material();
  ASSERT_NE(nullptr, mat);

  // set submesh non-unique material
  MaterialPtr matClone = mat->Clone();
  submesh->SetMaterial(matClone, false);

  EXPECT_EQ(matClone, submesh->Material());
  EXPECT_NE(mat, submesh->Material());

  submesh->SetMaterial("Default/White", false);
  EXPECT_EQ("Default/White", submesh->Material()->Name());

  // unique material
  submesh->SetMaterial(matClone, true);
  EXPECT_NE(matClone, submesh->Material());

  // Setting the same material but do not clone
  MaterialPtr newMatClone = submesh->Material();
  submesh->SetMaterial(newMatClone, false);
  EXPECT_EQ(newMatClone, submesh->Material());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(MeshTest, MeshSkeleton)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();

  // create a visual for the actor, attach mesh and get skeleton
  // Skeleton will be animated by GlutWindow
  VisualPtr actorVisual = scene->CreateVisual("actor");
  actorVisual->SetLocalPosition(0, 0, 0);
  actorVisual->SetLocalRotation(0, 0, 0);

  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(TEST_MEDIA_PATH, "walk.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  ASSERT_NE(nullptr, descriptor.mesh);

  MeshPtr mesh = scene->CreateMesh(descriptor);
  actorVisual->AddGeometry(mesh);
  root->AddChild(actorVisual);

  common::SkeletonPtr skel;

  if (mesh && descriptor.mesh->HasSkeleton())
  {
    skel = descriptor.mesh->MeshSkeleton();

    if (!skel || skel->AnimationCount() == 0)
    {
      FAIL();
    }
  }
  else
  {
    FAIL();
  }

  EXPECT_TRUE(mesh->HasSkeleton());

  std::string bvhFile = common::joinPaths(TEST_MEDIA_PATH, "cmu-13_26.bvh");

  double scale = 0.055;
  skel->AddBvhAnimation(bvhFile, scale);

  int g_animIdx = 1;
  auto * skelAnim = skel->Animation(g_animIdx);
  for (double i = 0; i < 10.0; i+=0.01)
  {
    std::map<std::string, math::Matrix4d> animFrames =
            skelAnim->PoseAt(i, true);

    std::map<std::string, math::Matrix4d> skinFrames;

    for (const auto &pair : animFrames)
    {
      const auto &animName = pair.first;
      const auto &animTf = pair.second;

      std::string skinName = skel->NodeNameAnimToSkin(g_animIdx, animName);
      math::Matrix4d skinTf =
              skel->AlignTranslation(g_animIdx, animName)
              * animTf * skel->AlignRotation(g_animIdx, animName);

      skinFrames[skinName] = skinTf;
    }

    mesh->SetSkeletonLocalTransforms(skinFrames);
  }

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(MeshTest, MeshSkeletonAnimation)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // test box mesh with no skeleton animation
  MeshDescriptor boxDescriptor("unit_box");
  MeshPtr boxMesh = scene->CreateMesh(boxDescriptor);
  ASSERT_NE(nullptr, boxMesh);

  EXPECT_FALSE(boxMesh->HasSkeleton());
  EXPECT_NO_THROW(boxMesh->SetSkeletonAnimationEnabled("invalid", false));
  EXPECT_FALSE(boxMesh->SkeletonAnimationEnabled("invalid"));
  EXPECT_TRUE(boxMesh->SkeletonLocalTransforms().empty());

  // test mesh with skeleton animation
  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(TEST_MEDIA_PATH, "walk.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  ASSERT_NE(nullptr, descriptor.mesh);
  MeshPtr mesh = scene->CreateMesh(descriptor);

  EXPECT_TRUE(mesh->HasSkeleton());
  EXPECT_FALSE(mesh->SkeletonLocalTransforms().empty());

  auto skel = descriptor.mesh->MeshSkeleton();
  ASSERT_EQ(2u, skel->AnimationCount());

  std::string animName = skel->Animation(0u)->Name();
  EXPECT_FALSE(mesh->SkeletonAnimationEnabled(animName));

  mesh->SetSkeletonAnimationEnabled(animName, true);
  EXPECT_TRUE(mesh->SkeletonAnimationEnabled(animName));

  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(0.0))));
  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(1.0))));
  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(-11.0))));
  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(1234.5))));

  // verify default skeleton bone weight
  std::unordered_map<std::string, float> weights = mesh->SkeletonWeights();
  EXPECT_FALSE(weights.empty());
  EXPECT_EQ(skel->NodeCount(), weights.size());
  for (auto &it : weights)
  {
    EXPECT_NE(nullptr, skel->NodeByName(it.first));
    EXPECT_FLOAT_EQ(1.0, it.second);
  }

  // change a bone weight and verify
  std::string nodeName  = skel->RootNode()->Name();
  EXPECT_EQ(1u, weights.count(nodeName));
  weights[nodeName] = 0.5;
  mesh->SetSkeletonWeights(weights);

  auto newWeights = mesh->SkeletonWeights();
  EXPECT_FALSE(newWeights.empty());
  EXPECT_EQ(skel->NodeCount(), newWeights.size());
  EXPECT_FLOAT_EQ(0.5, newWeights[nodeName]);
  for (auto &it : newWeights)
  {
    EXPECT_NE(nullptr, skel->NodeByName(it.first));
    if (it.first != nodeName)
    {
      EXPECT_FLOAT_EQ(1.0, it.second);
    }
  }

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(MeshTest, MeshClone)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // create the mesh using mesh descriptor
  MeshDescriptor descriptor("unit_box");
  MeshPtr mesh = scene->CreateMesh(descriptor);
  ASSERT_NE(nullptr, mesh);

  // clone the mesh
  auto clonedMesh = std::dynamic_pointer_cast<rendering::Mesh>(mesh->Clone());
  ASSERT_NE(nullptr, clonedMesh);

  // Compare mesh descriptors. The pointer to the mesh isn't included, but all
  // other fields should be equal
  auto clonedMeshDescriptor = clonedMesh->Descriptor();
  auto originalMeshDescriptor = mesh->Descriptor();
  EXPECT_EQ(clonedMeshDescriptor.meshName, originalMeshDescriptor.meshName);
  EXPECT_EQ(clonedMeshDescriptor.subMeshName,
      originalMeshDescriptor.subMeshName);
  EXPECT_EQ(clonedMeshDescriptor.centerSubMesh,
      originalMeshDescriptor.centerSubMesh);
  EXPECT_EQ(nullptr, clonedMeshDescriptor.mesh);
  EXPECT_EQ(nullptr, originalMeshDescriptor.mesh);

  // Helper function for comparing materials.
  std::function<void(MaterialPtr, MaterialPtr, bool)> compareMaterials =
    [&](const MaterialPtr _mat1, const MaterialPtr _mat2, bool _unique)
    {
      ASSERT_NE(nullptr, _mat1);
      ASSERT_NE(nullptr, _mat2);
      if (_unique)
      {
        EXPECT_NE(_mat1, _mat2);
        EXPECT_NE(_mat1->Name(), _mat2->Name());
      }
      else
      {
        EXPECT_EQ(_mat1, _mat2);
        EXPECT_EQ(_mat1->Name(), _mat2->Name());
      }
      EXPECT_EQ(_mat1->Type(), _mat2->Type());
      EXPECT_EQ(_mat1->Ambient(), _mat2->Ambient());
      EXPECT_EQ(_mat1->Diffuse(), _mat2->Diffuse());
      EXPECT_EQ(_mat1->Specular(),
          _mat2->Specular());
      EXPECT_DOUBLE_EQ(_mat1->Transparency(),
          _mat2->Transparency());
    };

  // compare materials and submeshes
  compareMaterials(clonedMesh->Material(), mesh->Material(), true);
  ASSERT_EQ(clonedMesh->SubMeshCount(), mesh->SubMeshCount());
  for (unsigned int i = 0; i < clonedMesh->SubMeshCount(); ++i)
  {
    // since the "top level mesh" has a material, the submesh materials are not
    // unique copies:
    // https://github.com/gazebosim/gz-rendering/blob/696885518a6b3ef461b06af094b7f32a51e8ad83/include/gz/rendering/base/BaseMesh.hh#L305
    auto clonedSubMesh = clonedMesh->SubMeshByIndex(i);
    auto originalSubMesh = clonedMesh->SubMeshByIndex(i);
    compareMaterials(clonedSubMesh->Material(), originalSubMesh->Material(),
        false);
  }

  // Setting the same material but do not clone
  MaterialPtr newMatClone = clonedMesh->Material();
  clonedMesh->SetMaterial(newMatClone, false);
  EXPECT_EQ(newMatClone, clonedMesh->Material());

  // Clean up
  engine->DestroyScene(scene);
}
