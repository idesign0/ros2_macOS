/*
 * Copyright (C) 2024 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <vector>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>
#include <gz/common/Console.hh>
#include <gz/common/DelaunayTriangulation.hh>

#include "CDT/CDT.h"

//////////////////////////////////////////////////
bool gz::common::DelaunayTriangulation(
    const std::vector<gz::math::Vector2d> &_vertices,
    const std::vector<gz::math::Vector2i> &_edges,
    gz::common::SubMesh *_subMesh)
{
  if (_edges.empty() || _vertices.empty())
  {
    gzerr << "Unable to create an extruded outline mesh with "
      << "no paths\n";
    return false;
  }

  if (_subMesh == nullptr)
  {
    gzerr << "No submesh to populate\n";
    return false;
  }

  std::vector<CDT::V2d<double>> cdtVerts;
  std::vector<CDT::Edge> cdtEdges;

  cdtVerts.reserve(_vertices.size());
  cdtEdges.reserve(_edges.size());

  for (const auto &vert : _vertices)
  {
    cdtVerts.push_back({vert.X(), vert.Y()});
  }

  for (const auto &edge : _edges)
  {
    cdtEdges.emplace_back(edge.X(), edge.Y());
  }

  CDT::Triangulation<double> cdt;
  cdt.insertVertices(cdtVerts);
  cdt.insertEdges(cdtEdges);
  cdt.eraseOuterTrianglesAndHoles();

  for (const auto & vert : cdt.vertices)
  {
    _subMesh->AddVertex(vert.x, vert.y, 0.0);
  }

  // Resulting triangles are counter-clockwise winding
  // Add the indices so that they are clockwise winding
  // in the submesh
  for (const auto & tri : cdt.triangles)
  {
    _subMesh->AddIndex(tri.vertices[2]);
    _subMesh->AddIndex(tri.vertices[1]);
    _subMesh->AddIndex(tri.vertices[0]);
  }

  return true;
}
