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

#include "gz/math/Vector3.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/MeshManager.hh"

#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

class SubMeshTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(SubMeshTest, SubMesh)
{
  auto submesh = std::make_shared<common::SubMesh>();
  ASSERT_NE(nullptr, submesh);

  submesh->SetName("new_submesh");
  EXPECT_EQ(submesh->Name(), "new_submesh");

  submesh->SetPrimitiveType(common::SubMesh::TRIANGLES);
  EXPECT_EQ(submesh->SubMeshPrimitiveType(), common::SubMesh::TRIANGLES);

  EXPECT_FALSE(submesh->GetMaterialIndex().has_value());
  submesh->SetMaterialIndex(3u);
  ASSERT_TRUE(submesh->GetMaterialIndex().has_value());
  EXPECT_EQ(submesh->GetMaterialIndex(), 3u);

  // verify empty submesh
  EXPECT_EQ(submesh->Min(), math::Vector3d::Zero);
  EXPECT_EQ(submesh->Max(), math::Vector3d::Zero);
  EXPECT_EQ(submesh->VertexCount(), 0u);
  EXPECT_EQ(submesh->NormalCount(), 0u);
  EXPECT_EQ(submesh->TexCoordCount(), 0u);
  EXPECT_EQ(submesh->IndexCount(), 0u);
  EXPECT_EQ(submesh->NodeAssignmentsCount(), 0u);
  EXPECT_EQ(submesh->MaxIndex(), 0u);

  // create geometry test data
  math::Vector3d v0(0, 0, 1);
  math::Vector3d v1(2, 0, 0);
  math::Vector3d v2(0, 3, 3);
  math::Vector3d n0(1, 0, 0);
  math::Vector3d n1(0, 1, 0);
  math::Vector3d n2(0.7, 0.7, 0);
  math::Vector2d uv0(0, 0);
  math::Vector2d uv1(0, 1);
  math::Vector2d uv2(1, 0);

  // add vertices
  submesh->AddVertex(v0);
  EXPECT_EQ(submesh->VertexCount(), 1u);
  EXPECT_EQ(submesh->Vertex(0u), v0);
  EXPECT_TRUE(submesh->HasVertex(v0));
  EXPECT_EQ(submesh->IndexOfVertex(v0), 0);
  EXPECT_EQ(submesh->VertexPtr()[0], v0);

  submesh->AddVertex(v1);
  EXPECT_EQ(submesh->VertexCount(), 2u);
  EXPECT_EQ(submesh->Vertex(1u), v1);
  EXPECT_TRUE(submesh->HasVertex(v1));
  EXPECT_EQ(submesh->IndexOfVertex(v1), 1);
  EXPECT_EQ(submesh->VertexPtr()[1], v1);

  submesh->AddVertex(v2.X(), v2.Y(), v2.Z());
  EXPECT_EQ(submesh->VertexCount(), 3u);
  EXPECT_EQ(submesh->Vertex(2u), v2);
  EXPECT_TRUE(submesh->HasVertex(v2));
  EXPECT_EQ(submesh->IndexOfVertex(v2), 2);
  EXPECT_EQ(submesh->VertexPtr()[2], v2);

  // max / min
  math::Vector3d max(2, 3, 3);
  math::Vector3d min(0, 0, 0);
  EXPECT_EQ(submesh->Max(), max);
  EXPECT_EQ(submesh->Min(), min);

  // add normals
  submesh->AddNormal(n0);
  EXPECT_EQ(submesh->NormalCount(), 1u);
  EXPECT_EQ(submesh->Normal(0u), n0);
  EXPECT_TRUE(submesh->HasNormal(0u));

  submesh->AddNormal(n1);
  EXPECT_EQ(submesh->NormalCount(), 2u);
  EXPECT_EQ(submesh->Normal(1u), n1);
  EXPECT_TRUE(submesh->HasNormal(1u));

  submesh->AddNormal(n2.X(), n2.Y(), n2.Z());
  EXPECT_EQ(submesh->NormalCount(), 3u);
  EXPECT_EQ(submesh->Normal(2u), n2);
  EXPECT_TRUE(submesh->HasNormal(2u));

  // add texcoord
  submesh->AddTexCoord(uv0);
  EXPECT_EQ(submesh->TexCoordCount(), 1u);
  EXPECT_EQ(submesh->TexCoord(0u), uv0);
  EXPECT_TRUE(submesh->HasTexCoord(0u));

  submesh->AddTexCoord(uv1);
  EXPECT_EQ(submesh->TexCoordCount(), 2u);
  EXPECT_EQ(submesh->TexCoord(1u), uv1);
  EXPECT_TRUE(submesh->HasTexCoord(1u));

  submesh->AddTexCoord(uv2.X(), uv2.Y());
  EXPECT_EQ(submesh->TexCoordCount(), 3u);
  EXPECT_EQ(submesh->TexCoord(2u), uv2);
  EXPECT_TRUE(submesh->HasTexCoord(2u));

  // add index
  submesh->AddIndex(0u);
  EXPECT_EQ(submesh->IndexCount(), 1u);
  EXPECT_EQ(submesh->Index(0), 0);
  EXPECT_EQ(submesh->IndexPtr()[0], 0u);

  submesh->AddIndex(2u);
  EXPECT_EQ(submesh->IndexCount(), 2u);
  EXPECT_EQ(submesh->Index(1u), 2);
  EXPECT_EQ(submesh->IndexPtr()[1], 2u);

  submesh->AddIndex(1u);
  EXPECT_EQ(submesh->IndexCount(), 3u);
  EXPECT_EQ(submesh->Index(2u), 1);
  EXPECT_EQ(submesh->IndexPtr()[2], 1u);

  // add node assignment
  submesh->AddNodeAssignment(1u, 0u, 0.5f);
  EXPECT_EQ(submesh->NodeAssignmentsCount(), 1u);

  common::NodeAssignment node = submesh->NodeAssignmentByIndex(0);
  EXPECT_EQ(node.vertexIndex, 1u);
  EXPECT_EQ(node.nodeIndex, 0u);
  EXPECT_DOUBLE_EQ(node.weight, 0.5f);

  common::NodeAssignment nodeInvalid = submesh->NodeAssignmentByIndex(1);
  EXPECT_EQ(nodeInvalid.vertexIndex, 0u);
  EXPECT_EQ(nodeInvalid.nodeIndex, 0u);
  EXPECT_DOUBLE_EQ(nodeInvalid.weight, 0.0f);

  // test directly setting values and failure cases
  submesh->SetVertex(2u, v0);
  EXPECT_EQ(submesh->Vertex(2u), v0);
  EXPECT_EQ(submesh->VertexPtr()[2u], v0);
  submesh->SetVertex(2u, v2);
  EXPECT_EQ(submesh->Vertex(2u), v2);
  EXPECT_EQ(submesh->VertexPtr()[2u], v2);

  // Failure case: write out of bounds should be ignored
  submesh->SetVertex(3u, math::Vector3d(0.9, 2, 4));
  EXPECT_EQ(submesh->Vertex(3u), math::Vector3d::Zero);
  // Out-of-bounds read of a raw pointer is UB. We can't test it.
  // EXPECT_EQ(submesh->VertexPtr()[3u], math::Vector3d::Zero);
  EXPECT_FALSE(submesh->HasVertex(math::Vector3d(0.9, 2, 4)));
  EXPECT_EQ(submesh->IndexOfVertex(math::Vector3d(0.9, 2, 4)), -1);

  submesh->SetNormal(2u, n0);
  EXPECT_EQ(submesh->Normal(2u), n0);
  submesh->SetNormal(2u, n2);
  EXPECT_EQ(submesh->Normal(2u), n2);
  submesh->SetNormal(3u, math::Vector3d(20, 3, 0.4));
  EXPECT_EQ(submesh->Normal(3u), math::Vector3d::Zero);
  EXPECT_FALSE(submesh->HasNormal(4u));

  submesh->SetTexCoord(2u, uv1);
  EXPECT_EQ(submesh->TexCoord(2u), uv1);
  submesh->SetTexCoord(2u, uv2);
  EXPECT_EQ(submesh->TexCoord(2u), uv2);
  submesh->SetTexCoord(3u, math::Vector2d(0.3, 0.4));
  EXPECT_EQ(submesh->TexCoord(3u), math::Vector2d(0, 0));

  submesh->SetIndex(2u, 9u);
  EXPECT_EQ(submesh->Index(2u), 9);
  submesh->SetIndex(2u, 2u);
  EXPECT_EQ(submesh->Index(2u), 2);
  submesh->SetIndex(3u, 999);
  EXPECT_EQ(submesh->Index(3u), -1);
  EXPECT_EQ(submesh->MaxIndex(), 2u);

  // scale
  math::Vector3d scale(1, 0.5, 2);
  submesh->Scale(scale);

  EXPECT_EQ(submesh->Vertex(0), v0 * scale);
  EXPECT_EQ(submesh->Vertex(1), v1 * scale);
  EXPECT_EQ(submesh->Vertex(2), v2 * scale);

  EXPECT_EQ(submesh->VertexPtr()[0], v0 * scale);
  EXPECT_EQ(submesh->VertexPtr()[1], v1 * scale);
  EXPECT_EQ(submesh->VertexPtr()[2], v2 * scale);

  EXPECT_EQ(submesh->Normal(0), n0);
  EXPECT_EQ(submesh->Normal(1), n1);
  EXPECT_EQ(submesh->Normal(2), n2);

  EXPECT_EQ(submesh->TexCoord(0), uv0);
  EXPECT_EQ(submesh->TexCoord(1), uv1);
  EXPECT_EQ(submesh->TexCoord(2), uv2);

  math::Vector3d scaleInv(1.0 / scale.X(), 1.0 / scale.Y(), 1.0 / scale.Z());
  submesh->Scale(scaleInv);

  EXPECT_EQ(submesh->Vertex(0), v0);
  EXPECT_EQ(submesh->Vertex(1), v1);
  EXPECT_EQ(submesh->Vertex(2), v2);

  EXPECT_EQ(submesh->VertexPtr()[0], v0);
  EXPECT_EQ(submesh->VertexPtr()[1], v1);
  EXPECT_EQ(submesh->VertexPtr()[2], v2);

  EXPECT_EQ(submesh->Normal(0), n0);
  EXPECT_EQ(submesh->Normal(1), n1);
  EXPECT_EQ(submesh->Normal(2), n2);

  EXPECT_EQ(submesh->TexCoord(0), uv0);
  EXPECT_EQ(submesh->TexCoord(1), uv1);
  EXPECT_EQ(submesh->TexCoord(2), uv2);

  // translate
  math::Vector3d t0(2, 3, 4);
  submesh->Translate(t0);

  EXPECT_EQ(submesh->Vertex(0), v0 + t0);
  EXPECT_EQ(submesh->Vertex(1), v1 + t0);
  EXPECT_EQ(submesh->Vertex(2), v2 + t0);

  EXPECT_EQ(submesh->VertexPtr()[0], v0 + t0);
  EXPECT_EQ(submesh->VertexPtr()[1], v1 + t0);
  EXPECT_EQ(submesh->VertexPtr()[2], v2 + t0);

  EXPECT_EQ(submesh->Normal(0), n0);
  EXPECT_EQ(submesh->Normal(1), n1);
  EXPECT_EQ(submesh->Normal(2), n2);

  EXPECT_EQ(submesh->TexCoord(0), uv0);
  EXPECT_EQ(submesh->TexCoord(1), uv1);
  EXPECT_EQ(submesh->TexCoord(2), uv2);

  // center
  math::Vector3d c0(8, 20, 3);
  submesh->Center(c0);

  math::Vector3d t = c0 - (min + t0 + (max-min)/2);
  EXPECT_EQ(submesh->Vertex(0), v0 + t0 + t);
  EXPECT_EQ(submesh->Vertex(1), v1 + t0 + t);
  EXPECT_EQ(submesh->Vertex(2), v2 + t0 + t);

  EXPECT_EQ(submesh->VertexPtr()[0], v0 + t0 + t);
  EXPECT_EQ(submesh->VertexPtr()[1], v1 + t0 + t);
  EXPECT_EQ(submesh->VertexPtr()[2], v2 + t0 + t);

  // copy constructor
  auto submeshCopy = std::make_shared<common::SubMesh>(*submesh.get());
  ASSERT_NE(nullptr, submeshCopy);
  EXPECT_EQ(submeshCopy->Name(), submesh->Name());
  EXPECT_EQ(submeshCopy->GetMaterialIndex(), submesh->GetMaterialIndex());
  EXPECT_EQ(submeshCopy->SubMeshPrimitiveType(),
      submesh->SubMeshPrimitiveType());
  EXPECT_EQ(submeshCopy->VertexCount(), submesh->VertexCount());
  EXPECT_EQ(submeshCopy->NormalCount(), submesh->NormalCount());
  EXPECT_EQ(submeshCopy->TexCoordCount(), submesh->TexCoordCount());
  EXPECT_EQ(submeshCopy->IndexCount(), submesh->IndexCount());
  EXPECT_EQ(submeshCopy->NodeAssignmentsCount(),
      submesh->NodeAssignmentsCount());

  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
    EXPECT_EQ(submeshCopy->VertexPtr()[i], submesh->VertexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_EQ(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_EQ(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
    EXPECT_EQ(submeshCopy->IndexPtr()[i], submesh->IndexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }

  // fill array
  double *vertices = NULL;
  int *indices = NULL;
  auto submeshEmpty = std::make_shared<common::SubMesh>();
  submeshEmpty->FillArrays(&vertices, &indices);
  EXPECT_TRUE(vertices == NULL);
  EXPECT_TRUE(indices == NULL);

  vertices = new double[9];
  indices = new int[3];
  submesh->FillArrays(&vertices, &indices);
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
  {
    EXPECT_DOUBLE_EQ(vertices[i*3], submesh->Vertex(i).X());
    EXPECT_DOUBLE_EQ(vertices[i*3+1], submesh->Vertex(i).Y());
    EXPECT_DOUBLE_EQ(vertices[i*3+2], submesh->Vertex(i).Z());

    EXPECT_DOUBLE_EQ(vertices[i*3], submesh->VertexPtr()[i].X());
    EXPECT_DOUBLE_EQ(vertices[i*3+1], submesh->VertexPtr()[i].Y());
    EXPECT_DOUBLE_EQ(vertices[i*3+2], submesh->VertexPtr()[i].Z());
  }
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
  {
    EXPECT_EQ(indices[i], submesh->Index(i));
    EXPECT_EQ(indices[i], submesh->IndexPtr()[i]);
  }

  delete[] vertices;
  delete[] indices;

  // recalculate normal and verify they are different
  submesh->RecalculateNormals();

  EXPECT_NE(submeshCopy->VertexPtr(), submesh->VertexPtr());
  EXPECT_NE(submeshCopy->IndexPtr(), submesh->IndexPtr());

  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_NE(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
    EXPECT_EQ(submeshCopy->VertexPtr()[i], submesh->VertexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_EQ(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
    EXPECT_EQ(submeshCopy->IndexPtr()[i], submesh->IndexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }
  submeshCopy->AddNormal(math::Vector3d(0, 1, 0));
  submeshCopy->RecalculateNormals();
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_EQ(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
    EXPECT_EQ(submeshCopy->VertexPtr()[i], submesh->VertexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_EQ(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
    EXPECT_EQ(submeshCopy->IndexPtr()[i], submesh->IndexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }

  // GenSphericalTexCoord and verify they are different
  submesh->GenSphericalTexCoord(math::Vector3d::Zero);
  for (unsigned int i = 0; i < submeshCopy->TexCoordCount(); ++i)
    EXPECT_NE(submeshCopy->TexCoord(i), submesh->TexCoord(i));
  for (unsigned int i = 0; i < submeshCopy->NormalCount(); ++i)
    EXPECT_EQ(submeshCopy->Normal(i), submesh->Normal(i));
  for (unsigned int i = 0; i < submeshCopy->VertexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Vertex(i), submesh->Vertex(i));
    EXPECT_EQ(submeshCopy->VertexPtr()[i], submesh->VertexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->IndexCount(); ++i)
  {
    EXPECT_EQ(submeshCopy->Index(i), submesh->Index(i));
    EXPECT_EQ(submeshCopy->IndexPtr()[i], submesh->IndexPtr()[i]);
  }
  for (unsigned int i = 0; i < submeshCopy->NodeAssignmentsCount(); ++i)
  {
    common::NodeAssignment nodeCopy =
      submeshCopy->NodeAssignmentByIndex(i);
    EXPECT_EQ(nodeCopy.vertexIndex, 1u);
    EXPECT_EQ(nodeCopy.nodeIndex, 0u);
    EXPECT_DOUBLE_EQ(nodeCopy.weight, 0.5f);
  }
}

/////////////////////////////////////////////////
void checkIndexes(const common::Mesh *_mesh)
{
  for (auto j = 0u; j < _mesh->SubMeshCount(); j++)
  {
    auto submesh = _mesh->SubMeshByIndex(j).lock();
    for (auto i = 0u; i < submesh->IndexCount(); i++)
    {
      EXPECT_GE(submesh->Index(i), 0);
    }
  }
}

/////////////////////////////////////////////////
TEST_F(SubMeshTest, Volume)
{
  // Box mesh tests
  {
    common::MeshManager::Instance()->CreateBox("unit_box",
        gz::math::Vector3d::One, gz::math::Vector2d::One);

    const common::Mesh *unitBox =
      common::MeshManager::Instance()->MeshByName("unit_box");
    ASSERT_TRUE(unitBox != nullptr);
    EXPECT_DOUBLE_EQ(1.0, unitBox->Volume());

    common::MeshManager::Instance()->CreateBox("other_box",
        gz::math::Vector3d(2, 3, 4), gz::math::Vector2d::One);
    const common::Mesh *otherBox =
      common::MeshManager::Instance()->MeshByName("other_box");
    ASSERT_TRUE(otherBox != nullptr);
    EXPECT_DOUBLE_EQ(24.0, otherBox->Volume());
  }

  // Sphere mesh tests
  {
    common::MeshManager::Instance()->CreateSphere("unit_sphere", 0.5, 10, 10);

    const common::Mesh *unitSphere =
      common::MeshManager::Instance()->MeshByName("unit_sphere");
    ASSERT_TRUE(unitSphere != nullptr);
    EXPECT_NEAR(4.0/3.0 * GZ_PI * std::pow(0.5, 3), unitSphere->Volume(),
        1e-2);

    // A larger sphere needs to have higher resolution in order to get the
    // volume close to (4/3) * Pi * r^3.
    common::MeshManager::Instance()->CreateSphere("other_sphere",
        2.5, 500, 500);

    const common::Mesh *otherSphere =
      common::MeshManager::Instance()->MeshByName("other_sphere");
    ASSERT_TRUE(otherSphere != nullptr);
    EXPECT_NEAR(4.0/3.0 * GZ_PI * std::pow(2.5, 3), otherSphere->Volume(),
        1e-2);
  }

  // Ellipsoid mesh tests
  {
    common::MeshManager::Instance()->CreateEllipsoid("ellipsoid",
        gz::math::Vector3d(2, 1.4, 7), 10, 10);

    const common::Mesh *unitEllipsoid =
      common::MeshManager::Instance()->MeshByName("ellipsoid");
    ASSERT_TRUE(unitEllipsoid != nullptr);

    checkIndexes(unitEllipsoid);

    // Checking that we can not add or modify a new mesh with the name
    common::MeshManager::Instance()->CreateEllipsoid("ellipsoid",
        gz::math::Vector3d(2, 1.4, 7), 100, 100);
    const common::Mesh *unitEllipsoid2 =
      common::MeshManager::Instance()->MeshByName("ellipsoid");

    checkIndexes(unitEllipsoid2);

    // The new mesh should have more vertex, but it should be introduced in the
    // meshmanager. It should be the same number because it was not modified.
    EXPECT_EQ(unitEllipsoid->VertexCount(), unitEllipsoid2->VertexCount());

    // A larger cylinder needs to have higher resolution in order to get the
    // volume close to Pi * r^2 * h.
    common::MeshManager::Instance()->CreateEllipsoid("other_ellipsoid",
        gz::math::Vector3d(0.8, 2.4, 7.5), 10, 10);

    const common::Mesh *otherEllipsoid =
      common::MeshManager::Instance()->MeshByName("other_ellipsoid");
    ASSERT_TRUE(otherEllipsoid != nullptr);

    checkIndexes(otherEllipsoid);
  }

  // Capsule mesh tests
  {
    common::MeshManager::Instance()->CreateCapsule("capsule",
      1, 1, 10, 10);

    const common::Mesh *unitCapsule =
      common::MeshManager::Instance()->MeshByName("capsule");
    ASSERT_TRUE(unitCapsule != nullptr);

    // Checking that we can not add or modify a new mesh with the same name
    common::MeshManager::Instance()->CreateCapsule("capsule",
      1, 1, 100, 100);
    const common::Mesh *unitCapsule2 =
      common::MeshManager::Instance()->MeshByName("capsule");

    // the new mesh should have more vertex, but it should be introduced in the
    // meshmanager. It should be the same number because it was not modified.
    EXPECT_EQ(unitCapsule->VertexCount(), unitCapsule2->VertexCount());

    common::MeshManager::Instance()->CreateCapsule("other_capsule",
      1.5, 7.5, 10, 10);

    const common::Mesh *otherCapsule =
      common::MeshManager::Instance()->MeshByName("other_capsule");
    ASSERT_TRUE(otherCapsule != nullptr);
  }

  // Cylinder mesh tests
  {
    common::MeshManager::Instance()->CreateCylinder("unit_cylinder",
        0.5, 1, 10, 10);

    const common::Mesh *unitCylinder =
      common::MeshManager::Instance()->MeshByName("unit_cylinder");
    ASSERT_TRUE(unitCylinder != nullptr);
    EXPECT_NEAR(GZ_PI * std::pow(0.5, 2) * 1.0, unitCylinder->Volume(),
        1e-2);

    // A larger cylinder needs to have higher resolution in order to get the
    // volume close to Pi * r^2 * h.
    common::MeshManager::Instance()->CreateCylinder("other_cylinder",
        2.5, 12, 500, 500);

    const common::Mesh *otherCylinder =
      common::MeshManager::Instance()->MeshByName("other_cylinder");
    ASSERT_TRUE(otherCylinder != nullptr);
    EXPECT_NEAR(GZ_PI * std::pow(2.5, 2) * 12, otherCylinder->Volume(),
        1e-2);
  }

  // Test another Primitive Type, which should result in a zero volume
  const common::Mesh *boxMesh =
    common::MeshManager::Instance()->MeshByName("unit_box");
  common::SubMesh boxSub(*(boxMesh->SubMeshByIndex(0).lock().get()));
  EXPECT_EQ(24u, boxSub.VertexCount());
  EXPECT_DOUBLE_EQ(1.0, boxSub.Volume());
  boxSub.SetPrimitiveType(common::SubMesh::LINES);
  EXPECT_DOUBLE_EQ(0.0, boxSub.Volume());

  // Test adding another index, which would make the submesh invalid.
  boxSub.SetPrimitiveType(common::SubMesh::TRIANGLES);
  EXPECT_DOUBLE_EQ(1.0, boxSub.Volume());
  boxSub.AddIndex(1);
  EXPECT_DOUBLE_EQ(0.0, boxSub.Volume());
}

/////////////////////////////////////////////////
TEST_F(SubMeshTest, NormalsRecalculation)
{
  auto submesh = std::make_shared<common::SubMesh>();
  submesh->SetPrimitiveType(common::SubMesh::TRIANGLES);

  constexpr unsigned int triangles = 16384;
  for (unsigned int i = 0; i < triangles; ++i) {
    // sub to X less than _epsilon from even triangles
    // expect that the 2nd vertex should be matched with
    // the 1st of next triangle
    const auto jitter = i % 2 ? 1e-7 : 0.0;
    submesh->AddVertex(i-jitter, i, i);
    submesh->AddVertex(i+1, i+1, i+1);
    submesh->AddVertex(i, i, -static_cast<double>(i));

    submesh->AddIndex(3*i);
    submesh->AddIndex(3*i+1);
    submesh->AddIndex(3*i+2);
  }

  ASSERT_EQ(submesh->IndexCount() % 3, 0u);
  submesh->RecalculateNormals();
  ASSERT_EQ(submesh->NormalCount(), submesh->VertexCount());
  // Same triangle, but different normals
  // because of neighbour vertex
  ASSERT_NE(submesh->Normal(0), submesh->Normal(1));
}

/////////////////////////////////////////////////
TEST_F(SubMeshTest, HasValidIndices)
{
  // Check empty submesh
  auto submesh = std::make_shared<common::SubMesh>();
  EXPECT_FALSE(submesh->HasValidIndices());

  // Check box submesh
  const common::Mesh *unitBox =
    common::MeshManager::Instance()->MeshByName("unit_box");
  ASSERT_TRUE(unitBox != nullptr);
  auto unitBoxSubmesh = unitBox->SubMeshByIndex(0).lock();

  // Submesh has valid indices and we should be able to calculate normals
  // and volume
  EXPECT_TRUE(unitBoxSubmesh->HasValidIndices());
  unitBoxSubmesh->RecalculateNormals();
  ASSERT_EQ(unitBoxSubmesh->VertexCount(), unitBoxSubmesh->NormalCount());
  EXPECT_EQ(24u, unitBoxSubmesh->NormalCount());
  EXPECT_DOUBLE_EQ(1.0, unitBoxSubmesh->Volume());

  // Add some invalid indices
  unitBoxSubmesh->AddIndex(100);
  unitBoxSubmesh->AddIndex(101);
  unitBoxSubmesh->AddIndex(102);

  // Submesh has invalid indices so it is no longer able to update normals or
  // compute volume.
  EXPECT_FALSE(unitBoxSubmesh->HasValidIndices());
  unitBoxSubmesh->RecalculateNormals();
  EXPECT_EQ(24u, unitBoxSubmesh->NormalCount());
  EXPECT_DOUBLE_EQ(0.0, unitBoxSubmesh->Volume());
}
