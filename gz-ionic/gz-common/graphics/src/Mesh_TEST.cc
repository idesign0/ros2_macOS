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

#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SubMesh.hh"
#include "gz/math/Vector3.hh"

#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

class MeshTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(MeshTest, Mesh)
{
  auto mesh = std::make_shared<common::Mesh>();
  EXPECT_TRUE(mesh != nullptr);

  mesh->SetName("new_mesh");
  EXPECT_EQ(mesh->Name(), "new_mesh");

  mesh->SetPath(".");
  EXPECT_EQ(mesh->Path(), ".");

  // verify empty submesh
  EXPECT_EQ(mesh->Min(), math::Vector3d::Zero);
  EXPECT_EQ(mesh->Max(), math::Vector3d::Zero);
  EXPECT_EQ(mesh->VertexCount(), 0u);
  EXPECT_EQ(mesh->NormalCount(), 0u);
  EXPECT_EQ(mesh->TexCoordCount(), 0u);
  EXPECT_EQ(mesh->IndexCount(), 0u);
  EXPECT_EQ(mesh->MaterialCount(), 0u);
  EXPECT_EQ(mesh->SubMeshCount(), 0u);

  // add a submesh and verify
  common::SubMesh submesh0;

  submesh0.SetName("new_submesh");
  EXPECT_EQ(submesh0.Name(), "new_submesh");

  math::Vector3d v0(0, 0, 2);
  math::Vector3d n0(1, 0, 0);
  math::Vector2d uv0(0, 0.1);
  submesh0.AddVertex(v0);
  submesh0.AddNormal(n0);
  submesh0.AddTexCoord(uv0);
  submesh0.AddIndex(0);

  auto submesh = mesh->AddSubMesh(submesh0);
  EXPECT_EQ(mesh->VertexCount(), 1u);
  EXPECT_EQ(mesh->NormalCount(), 1u);
  EXPECT_EQ(mesh->TexCoordCount(), 1u);
  EXPECT_EQ(mesh->IndexCount(), 1u);
  EXPECT_EQ(mesh->SubMeshCount(), 1u);
  EXPECT_EQ(mesh->SubMeshByIndex(0).lock().get(), submesh.lock().get());
  EXPECT_EQ(mesh->SubMeshByName("new_submesh").lock().get(),
      submesh.lock().get());
  EXPECT_TRUE(mesh->SubMeshByIndex(1u).lock() == nullptr);
  EXPECT_TRUE(mesh->SubMeshByName("no_such_submesh").lock() == nullptr);

  EXPECT_EQ(mesh->Min(), v0);
  EXPECT_EQ(mesh->Max(), v0);

  // AABB
  math::Vector3d c;
  math::Vector3d min;
  math::Vector3d max;
  mesh->AABB(c, min, max);
  EXPECT_EQ(c, v0);
  EXPECT_EQ(min, v0);
  EXPECT_EQ(max, v0);

  // scale
  math::Vector3d scale(2, 0.25, 4);
  mesh->SetScale(scale);
  EXPECT_EQ(submesh.lock()->Vertex(0), v0 * scale);
  EXPECT_EQ(submesh.lock()->Normal(0), n0);
  EXPECT_EQ(submesh.lock()->TexCoord(0), uv0);
  EXPECT_EQ(submesh.lock()->VertexPtr()[0], v0 * scale);

  mesh->Scale(scale);
  EXPECT_EQ(submesh.lock()->Vertex(0), v0 * scale * scale);
  EXPECT_EQ(submesh.lock()->Normal(0), n0);
  EXPECT_EQ(submesh.lock()->TexCoord(0), uv0);
  EXPECT_EQ(submesh.lock()->VertexPtr()[0], v0 * scale * scale);

  // translate
  math::Vector3d t0(2, 3, -12);
  mesh->Translate(t0);
  EXPECT_EQ(submesh.lock()->Vertex(0), v0 * scale * scale + t0);
  EXPECT_EQ(submesh.lock()->Normal(0), n0);
  EXPECT_EQ(submesh.lock()->TexCoord(0), uv0);
  EXPECT_EQ(submesh.lock()->VertexPtr()[0], v0 * scale * scale + t0);

  // center
  math::Vector3d c0(0.1, 3, 1);
  mesh->Center(c0);

  math::Vector3d t = c0 - (min + t0 + (max-min)/2);
  EXPECT_EQ(submesh.lock()->Vertex(0), v0 + t0 + t);

  // add material
  common::MaterialPtr material(new common::Material());
  EXPECT_TRUE(material != nullptr);

  mesh->AddMaterial(material);
  EXPECT_EQ(mesh->MaterialCount(), 1u);
  EXPECT_EQ(mesh->MaterialByIndex(0u), material);
  EXPECT_TRUE(mesh->MaterialByIndex(1u) == nullptr);
  EXPECT_EQ(mesh->IndexOfMaterial(material.get()), 0);
  EXPECT_EQ(mesh->IndexOfMaterial(nullptr), -1);

  // skeleton
  common::SkeletonPtr skeleton(new common::Skeleton());
  EXPECT_TRUE(skeleton != nullptr);
  EXPECT_FALSE(mesh->HasSkeleton());
  mesh->SetSkeleton(skeleton);
  EXPECT_TRUE(mesh->HasSkeleton());
  EXPECT_EQ(mesh->MeshSkeleton(), skeleton);

  // recalculate normals
  // (should not have changed since it requires at least 3 normals)
  mesh->RecalculateNormals();
  EXPECT_EQ(submesh.lock()->Normal(0), n0);

  // GenSphericalTexCoord
  mesh->GenSphericalTexCoord(math::Vector3d::Zero);
  EXPECT_NE(submesh.lock()->TexCoord(0), uv0);

  // fill array
  double *vertices = new double[3];
  int *indices = new int[1];
  mesh->FillArrays(&vertices, &indices);
  EXPECT_TRUE(math::equal(vertices[0], submesh.lock()->Vertex(0).X()));
  EXPECT_TRUE(math::equal(vertices[1], submesh.lock()->Vertex(0).Y()));
  EXPECT_TRUE(math::equal(vertices[2], submesh.lock()->Vertex(0).Z()));
  EXPECT_EQ(indices[0], submesh.lock()->Index(0));
  EXPECT_EQ(indices[0], submesh.lock()->IndexPtr()[0]);

  delete [] vertices;
  delete [] indices;
}
