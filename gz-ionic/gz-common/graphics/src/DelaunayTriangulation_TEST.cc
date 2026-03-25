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
#include <memory>

#include "gz/common/DelaunayTriangulation.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"

#include "gz/common/testing/AutoLogFixture.hh"

class DelaunayTriangulation: public gz::common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(DelaunayTriangulation, DelaunayTriangulation)
{

  // test triangulation of a path with two subpaths:
  // a smaller square inside a bigger square.
  // The smaller square should be treated as a hole inside the bigger square.

  std::vector<gz::math::Vector2d> vertices;
  // outside square
  vertices.push_back(gz::math::Vector2d(0, 0));
  vertices.push_back(gz::math::Vector2d(1, 0));
  vertices.push_back(gz::math::Vector2d(1, 1));
  vertices.push_back(gz::math::Vector2d(0, 1));
  // inside square
  vertices.push_back(gz::math::Vector2d(0.25, 0.25));
  vertices.push_back(gz::math::Vector2d(0.25, 0.75));
  vertices.push_back(gz::math::Vector2d(0.75, 0.75));
  vertices.push_back(gz::math::Vector2d(0.75, 0.25));

  std::vector<gz::math::Vector2i> edges;
  edges.push_back(gz::math::Vector2i(0, 1));
  edges.push_back(gz::math::Vector2i(1, 2));
  edges.push_back(gz::math::Vector2i(2, 3));
  edges.push_back(gz::math::Vector2i(3, 0));

  edges.push_back(gz::math::Vector2i(4, 5));
  edges.push_back(gz::math::Vector2i(5, 6));
  edges.push_back(gz::math::Vector2i(6, 7));
  edges.push_back(gz::math::Vector2i(7, 4));

  auto mesh = std::make_unique<gz::common::Mesh>();
  mesh->SetName("extruded");
  gz::common::SubMesh subMesh;

  bool result = gz::common::DelaunayTriangulation(vertices,
                                                  edges,
                                                  &subMesh);
  EXPECT_TRUE(result);

  // same as number of vertices in the path
  EXPECT_EQ(subMesh.VertexCount(), 8u);

  // there should be 8 triangles.
  EXPECT_EQ(subMesh.IndexCount() / 3u, 8u);

  // verify that triangles have clockwise winding
  for (unsigned int t = 0; t < subMesh.IndexCount() / 3u; ++t)
  {
    int vertexIndex1 = subMesh.Index(t*3 + 0);
    int vertexIndex2 = subMesh.Index(t*3 + 1);
    int vertexIndex3 = subMesh.Index(t*3 + 2);
    // compute displacement from vertex 1 to 2
    auto displacement12 =
        subMesh.Vertex(vertexIndex2) - subMesh.Vertex(vertexIndex1);
    // compute displacement from vertex 2 to 3
    auto displacement23 =
        subMesh.Vertex(vertexIndex3) - subMesh.Vertex(vertexIndex2);
    // compute cross product (v2-v1) x (v3 - v2)
    auto crossProduct_12_23 = displacement12.Cross(displacement23);
    // X and Y components should be zero
    EXPECT_DOUBLE_EQ(0.0, crossProduct_12_23.X());
    EXPECT_DOUBLE_EQ(0.0, crossProduct_12_23.Y());
    // Z component should be negative for a clockwise winding
    EXPECT_LT(crossProduct_12_23.Z(), 0.0);
  }
}
