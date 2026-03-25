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
#include "gz/common/MeshManager.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

#ifndef _WIN32
class MeshManager : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(MeshManager, CreateExtrudedPolyline)
{
  // test extrusion of a path with two subpaths:
  // a smaller square inside a bigger square.
  // The smaller square should be treated as a hole inside the bigger square.
  std::vector<std::vector<gz::math::Vector2d> > path;
  std::vector<gz::math::Vector2d> subpath01;
  subpath01.emplace_back(0, 0);
  subpath01.emplace_back(1, 0);
  subpath01.emplace_back(1, 1);
  subpath01.emplace_back(0, 1);
  subpath01.emplace_back(0, 0);

  std::vector<gz::math::Vector2d> subpath02;
  subpath02.emplace_back(0.25, 0.25);
  subpath02.emplace_back(0.25, 0.75);
  subpath02.emplace_back(0.75, 0.75);
  subpath02.emplace_back(0.75, 0.25);
  subpath02.emplace_back(0.25, 0.25);

  path.push_back(subpath01);
  path.push_back(subpath02);

  std::string meshName = "extruded_path";
  double height = 10.0;
  common::MeshManager::Instance()->CreateExtrudedPolyline(
      meshName, path, height);

  // check mesh
  EXPECT_TRUE(common::MeshManager::Instance()->HasMesh(meshName));
  const common::Mesh *mesh =
    common::MeshManager::Instance()->MeshByName(meshName);
  EXPECT_TRUE(mesh != nullptr);

  unsigned int submeshCount = mesh->SubMeshCount();
  EXPECT_EQ(submeshCount, 1u);

  // check submesh bounds
  auto submesh = mesh->SubMeshByIndex(0).lock();
  EXPECT_TRUE(submesh != nullptr);
  EXPECT_EQ(gz::math::Vector3d(0, 0, 0), submesh->Min());
  EXPECT_EQ(gz::math::Vector3d(1.0, 1.0, 10.0), submesh->Max());

  // check vertices
  for (unsigned int i = 0; i < submesh->VertexCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);

    // check no vertices are in the region of the hole
    EXPECT_FALSE((v.X() > 0.25 && v.X() < 0.75));
    EXPECT_FALSE((v.Y() > 0.25 && v.Y() < 0.75));

    // check extruded height
    EXPECT_TRUE((gz::math::equal(v.Z(), 0.0) ||
          gz::math::equal(v.Z(), 10.0)));
  }

  // verify same number of normals and vertices
  EXPECT_EQ(submesh->VertexCount(), submesh->NormalCount());

  // check normals
  for (unsigned int i = 0; i < submesh->NormalCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);
    gz::math::Vector3d n = submesh->Normal(i);

    // vertex at 0 could be a bottom face or side face
    if (gz::math::equal(v.Z(), 0.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
              gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // bottom face - normal in -z direction
        EXPECT_TRUE((n == -gz::math::Vector3d::UnitZ) ||
            (gz::math::equal(n.Z(), 0.0)));
      }
    }

    // vertex at height could be a top face or side face
    if (gz::math::equal(v.Z(), 10.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
              gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // top face - normal in +z direction
        EXPECT_TRUE((n == gz::math::Vector3d::UnitZ) ||
            (gz::math::equal(n.Z(), 0.0)));
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(MeshManager, CreateExtrudedPolylineClosedPath)
{
  // test extrusion of a path that has two closed subpaths, i.e.,
  // first and last vertices are the same.
  // The following two subpaths form the letter 'A'.
  std::vector<std::vector<gz::math::Vector2d> > path2;
  std::vector<gz::math::Vector2d> subpath03;
  subpath03.emplace_back(2.27467, 1.0967);
  subpath03.emplace_back(1.81094, 2.35418);
  subpath03.emplace_back(2.74009, 2.35418);

  std::vector<gz::math::Vector2d> subpath04;
  subpath04.emplace_back(2.08173, 0.7599);
  subpath04.emplace_back(2.4693, 0.7599);
  subpath04.emplace_back(3.4323, 3.28672);
  subpath04.emplace_back(3.07689, 3.28672);
  subpath04.emplace_back(2.84672, 2.63851);
  subpath04.emplace_back(1.7077, 2.63851);
  subpath04.emplace_back(1.47753, 3.28672);
  subpath04.emplace_back(1.11704, 3.28672);

  path2.push_back(subpath03);
  path2.push_back(subpath04);

  std::string meshName = "extruded_path_closed";
  double height = 2.0;
  common::MeshManager::Instance()->CreateExtrudedPolyline(
      meshName, path2, height);

  // check mesh
  EXPECT_TRUE(common::MeshManager::Instance()->HasMesh(meshName));
  const common::Mesh *mesh =
    common::MeshManager::Instance()->MeshByName(meshName);
  EXPECT_TRUE(mesh != nullptr);

  unsigned int submeshCount = mesh->SubMeshCount();
  EXPECT_EQ(submeshCount, 1u);

  // check submesh bounds
  auto submesh = mesh->SubMeshByIndex(0).lock();
  EXPECT_TRUE(submesh != nullptr);
  EXPECT_EQ(submesh->Min(), gz::math::Vector3d(1.11704, 0.7599, 0));
  EXPECT_EQ(submesh->Max(), gz::math::Vector3d(3.4323, 3.28672, 2.0));

  for (unsigned int i = 0; i < submesh->VertexCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);

    // check no vertices are in the region of the hole using a point-in-polygon
    // algorithm
    bool pointInPolygon = false;
    for (unsigned int j = 0, k = subpath03.size()-1; j < subpath03.size();
        k = ++j)
    {
      if (((subpath03[j].Y() > v.Y()) != (subpath03[k].Y() > v.Y())) &&
          (v.X() < (subpath03[k].X()-subpath03[j].X()) *
           (v.Y()-subpath03[j].Y()) /
         (subpath03[k].Y()-subpath03[j].Y()) + subpath03[j].X()) )
      {
       pointInPolygon = !pointInPolygon;
      }
    }
    EXPECT_FALSE(pointInPolygon);

    // check extruded height
    EXPECT_TRUE((gz::math::equal(v.Z(), 0.0) ||
          gz::math::equal(v.Z(), 2.0)));
  }

  // verify same number of normals and vertices
  EXPECT_EQ(submesh->VertexCount(), submesh->NormalCount());

  // check normals
  for (unsigned int i = 0; i < submesh->NormalCount(); ++i)
  {
    gz::math::Vector3d v = submesh->Vertex(i);
    gz::math::Vector3d n = submesh->Normal(i);

    // vertex at 0 could be a bottom face or side face
    if (gz::math::equal(v.Z(), 0.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
                      gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // bottom face - normal in -z direction
        EXPECT_TRUE((n == -gz::math::Vector3d::UnitZ) ||
                    (gz::math::equal(n.Z(), 0.0)));
      }
    }

    // vertex at height could be a top face or side face
    if (gz::math::equal(v.Z(), 10.0))
    {
      if (gz::math::equal(n.Z(), 0.0))
      {
        // side face - check non-zero normal
        EXPECT_TRUE(!(gz::math::equal(n.X(), 0.0) &&
                      gz::math::equal(n.Y(), 0.0)));
      }
      else
      {
        // top face - normal in +z direction
        EXPECT_TRUE((n == gz::math::Vector3d::UnitZ) ||
                    (gz::math::equal(n.Z(), 0.0)));
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(MeshManager, CreateExtrudedPolylineInvalid)
{
  // test extruding invalid polyline
  std::vector<std::vector<gz::math::Vector2d> > path;
  std::vector<gz::math::Vector2d> subpath01;
  subpath01.emplace_back(0, 0);
  subpath01.emplace_back(0, 1);
  subpath01.emplace_back(0, 2);

  path.push_back(subpath01);

  std::string meshName = "extruded_path_invalid";
  double height = 10.0;
  common::MeshManager::Instance()->CreateExtrudedPolyline(
      meshName, path, height);

  // check mesh does not exist due to extrusion failure
  EXPECT_TRUE(!common::MeshManager::Instance()->HasMesh(meshName));
}

/////////////////////////////////////////////////
TEST_F(MeshManager, Remove)
{
  auto *mgr = common::MeshManager::Instance();

  EXPECT_FALSE(mgr->HasMesh("box"));
  mgr->CreateBox("box",
      gz::math::Vector3d(1, 1, 1),
      gz::math::Vector2d(0, 0));
  EXPECT_TRUE(mgr->HasMesh("box"));

  mgr->CreateSphere("sphere", 1.0, 1, 1);
  EXPECT_TRUE(mgr->HasMesh("sphere"));

  EXPECT_TRUE(mgr->RemoveMesh("box"));
  EXPECT_FALSE(mgr->HasMesh("box"));
  EXPECT_TRUE(mgr->HasMesh("sphere"));

  mgr->RemoveAll();
  EXPECT_FALSE(mgr->HasMesh("sphere"));
}

/////////////////////////////////////////////////
TEST_F(MeshManager, ConvexDecomposition)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *boxMesh = mgr->Load(
      common::testing::TestFile("data", "box.dae"));

  ASSERT_NE(nullptr, boxMesh);
  EXPECT_EQ(1u, boxMesh->SubMeshCount());

  std::size_t maxConvexHulls = 4;
  std::size_t resolution = 1000;
  auto submesh = boxMesh->SubMeshByIndex(0u).lock();
  auto decomposed = common::MeshManager::ConvexDecomposition(
      *submesh, maxConvexHulls, resolution);

  // Decomposing a box should just produce a box
  EXPECT_EQ(1u, decomposed.size());
  common::SubMesh &boxSubmesh = decomposed[0];
  // A convex hull of a box should contain exactly 8 vertices
  EXPECT_EQ(8u, boxSubmesh.VertexCount());
  EXPECT_EQ(8u, boxSubmesh.NormalCount());
  EXPECT_EQ(36u, boxSubmesh.IndexCount());

  const common::Mesh *drillMesh = mgr->Load(
      common::testing::TestFile("data", "cordless_drill",
      "meshes", "cordless_drill.dae"));
  ASSERT_NE(nullptr, drillMesh);
  EXPECT_EQ(1u, drillMesh->SubMeshCount());
  submesh = drillMesh->SubMeshByIndex(0u).lock();
  decomposed = common::MeshManager::ConvexDecomposition(
      *submesh, maxConvexHulls, resolution);

  // A drill should be decomposed into multiple submeshes
  EXPECT_LT(1u, decomposed.size());
  EXPECT_GE(maxConvexHulls, decomposed.size());
  // Check submeshes are not empty
  for (const auto &d : decomposed)
  {
    const common::SubMesh &drillSubmesh = d;
    EXPECT_LT(3u, drillSubmesh.VertexCount());
    EXPECT_EQ(drillSubmesh.VertexCount(), drillSubmesh.NormalCount());
    EXPECT_LT(3u, drillSubmesh.IndexCount());
  }
}

/////////////////////////////////////////////////
TEST_F(MeshManager, MergeSubMeshes)
{
  auto *mgr = common::MeshManager::Instance();
  const common::Mesh *mesh = mgr->Load(
      common::testing::TestFile("data",
        "multiple_texture_coordinates_triangle.dae"));
  ASSERT_NE(nullptr, mesh);
  EXPECT_EQ(2u, mesh->SubMeshCount());
  auto submesh = mesh->SubMeshByIndex(0u).lock();
  ASSERT_NE(nullptr, submesh);
  EXPECT_EQ(3u, submesh->VertexCount());
  EXPECT_EQ(3u, submesh->NormalCount());
  EXPECT_EQ(3u, submesh->IndexCount());
  EXPECT_EQ(2u, submesh->TexCoordSetCount());
  EXPECT_EQ(3u, submesh->TexCoordCountBySet(0));
  EXPECT_EQ(3u, submesh->TexCoordCountBySet(1));
  auto submeshB = mesh->SubMeshByIndex(1u).lock();
  ASSERT_NE(nullptr, submeshB);
  EXPECT_EQ(3u, submeshB->VertexCount());
  EXPECT_EQ(3u, submeshB->NormalCount());
  EXPECT_EQ(3u, submeshB->IndexCount());
  EXPECT_EQ(3u, submeshB->TexCoordSetCount());
  EXPECT_EQ(3u, submeshB->TexCoordCountBySet(0));
  EXPECT_EQ(3u, submeshB->TexCoordCountBySet(1));
  EXPECT_EQ(3u, submeshB->TexCoordCountBySet(2));

  // merge all submeshes into one
  auto merged = common::MeshManager::MergeSubMeshes(*mesh);
  ASSERT_NE(nullptr, merged);
  EXPECT_FALSE(merged->Name().empty());
  EXPECT_EQ(1u, merged->SubMeshCount());
  auto mergedSubmesh = merged->SubMeshByIndex(0u).lock();
  ASSERT_NE(nullptr, mergedSubmesh);
  EXPECT_FALSE(mergedSubmesh->Name().empty());

  // Verify vertice, normals, indice, and texcoord values in the
  // final merged submesh
  EXPECT_EQ(6u, mergedSubmesh->VertexCount());
  EXPECT_EQ(6u, mergedSubmesh->NormalCount());
  EXPECT_EQ(6u, mergedSubmesh->IndexCount());
  EXPECT_EQ(3u, mergedSubmesh->TexCoordSetCount());
  EXPECT_EQ(6u, mergedSubmesh->TexCoordCountBySet(0));
  EXPECT_EQ(6u, mergedSubmesh->TexCoordCountBySet(1));
  EXPECT_EQ(6u, mergedSubmesh->TexCoordCountBySet(2));

  EXPECT_EQ(math::Vector3d(0, 0, 0), mergedSubmesh->Vertex(0u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), mergedSubmesh->Vertex(1u));
  EXPECT_EQ(math::Vector3d(10, 10, 0), mergedSubmesh->Vertex(2u));
  EXPECT_EQ(math::Vector3d(10, 0, 0), mergedSubmesh->Vertex(3u));
  EXPECT_EQ(math::Vector3d(20, 0, 0), mergedSubmesh->Vertex(4u));
  EXPECT_EQ(math::Vector3d(20, 10, 0), mergedSubmesh->Vertex(5u));

  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(0u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(1u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(2u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(3u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(4u));
  EXPECT_EQ(math::Vector3d(0, 0, 1), mergedSubmesh->Normal(5u));

  EXPECT_EQ(0u, mergedSubmesh->Index(0u));
  EXPECT_EQ(1u, mergedSubmesh->Index(1u));
  EXPECT_EQ(2u, mergedSubmesh->Index(2u));
  EXPECT_EQ(3u, mergedSubmesh->Index(3u));
  EXPECT_EQ(4u, mergedSubmesh->Index(4u));
  EXPECT_EQ(5u, mergedSubmesh->Index(5u));

  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(0u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(1u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(2u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(3u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(4u, 0u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(5u, 0u));

  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(0u, 1u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(1u, 1u));
  EXPECT_EQ(math::Vector2d(0, 1), mergedSubmesh->TexCoordBySet(2u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.5), mergedSubmesh->TexCoordBySet(3u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.4), mergedSubmesh->TexCoordBySet(4u, 1u));
  EXPECT_EQ(math::Vector2d(0, 0.3), mergedSubmesh->TexCoordBySet(5u, 1u));

  EXPECT_EQ(math::Vector2d(0, 0), mergedSubmesh->TexCoordBySet(0u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0), mergedSubmesh->TexCoordBySet(1u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0), mergedSubmesh->TexCoordBySet(2u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.8), mergedSubmesh->TexCoordBySet(3u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.7), mergedSubmesh->TexCoordBySet(4u, 2u));
  EXPECT_EQ(math::Vector2d(0, 0.6), mergedSubmesh->TexCoordBySet(5u, 2u));
}

#endif
