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

#ifndef GZ_COMMON_DELAUNAYTRIANGULATION_HH_
#define GZ_COMMON_DELAUNAYTRIANGULATION_HH_

#include <vector>

#include <gz/math/Vector2.hh>

#include <gz/common/graphics/Export.hh>


namespace gz::common
{
class SubMesh;

/// \brief Perform delaunay triangulation on input vertices.
/// \param[in] _vertices A list of all vertices
/// \param[in] _edges A list of edges. Each edge is made of 2 vertex
/// indices from _vertices
/// \param[out] _submesh A submesh that will be populated with the
/// resulting triangles.
/// \return True on success.
bool GZ_COMMON_GRAPHICS_VISIBLE DelaunayTriangulation(
                  const std::vector<gz::math::Vector2d> &_vertices,
                  const std::vector<gz::math::Vector2i> &_edges,
                  gz::common::SubMesh *_submesh);

}  // namespace gz::common
#endif  // GZ_COMMON_DELAUNAYTRIANGULATION_HH_
