/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "gz/math/Helpers.hh"

#include "gz/common/Console.hh"
#include "gz/common/Material.hh"
#include "gz/common/SubMesh.hh"

using namespace gz;
using namespace common;

/// \brief Private data for SubMesh
class gz::common::SubMesh::Implementation
{
  /// \brief the vertex array
  public: std::vector<gz::math::Vector3d> vertices;

  /// \brief the normal array
  public: std::vector<gz::math::Vector3d> normals;

  /// \brief A map of texcoord set index to texture coordinate array
  public: std::map<unsigned int, std::vector<gz::math::Vector2d>>
      texCoords;

  /// \brief the vertex index array
  public: std::vector<unsigned int> indices;

  /// \brief node assignment array
  public: std::vector<NodeAssignment> nodeAssignments;

  /// \brief primitive type for the mesh
  public: SubMesh::PrimitiveType primitiveType = SubMesh::TRIANGLES;

  /// \brief The material index for this mesh. Relates to the parent
  /// mesh material list.
  public: std::optional<unsigned int> materialIndex = std::nullopt;

  /// \brief The name of the sub-mesh
  public: std::string name;
};

//////////////////////////////////////////////////
SubMesh::SubMesh()
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
SubMesh::SubMesh(const std::string &_name)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
SubMesh::~SubMesh()
{
}

//////////////////////////////////////////////////
void SubMesh::SetPrimitiveType(PrimitiveType _type)
{
  this->dataPtr->primitiveType = _type;
}

//////////////////////////////////////////////////
SubMesh::PrimitiveType SubMesh::SubMeshPrimitiveType() const
{
  return this->dataPtr->primitiveType;
}

//////////////////////////////////////////////////
void SubMesh::AddIndex(const unsigned int _index)
{
  this->dataPtr->indices.push_back(_index);
}

//////////////////////////////////////////////////
void SubMesh::AddVertex(const gz::math::Vector3d &_v)
{
  this->dataPtr->vertices.push_back(_v);
}

//////////////////////////////////////////////////
void SubMesh::AddVertex(const double _x, const double _y, const double _z)
{
  this->AddVertex(gz::math::Vector3d(_x, _y, _z));
}

//////////////////////////////////////////////////
void SubMesh::AddNormal(const gz::math::Vector3d &_n)
{
  this->dataPtr->normals.push_back(_n);
}

//////////////////////////////////////////////////
void SubMesh::AddNormal(const double _x, const double _y, const double _z)
{
  this->AddNormal(gz::math::Vector3d(_x, _y, _z));
}

//////////////////////////////////////////////////
void SubMesh::AddTexCoord(const double _u, const double _v)
{
  unsigned firstSetIndex = 0u;
  if (!this->dataPtr->texCoords.empty())
    firstSetIndex = this->dataPtr->texCoords.begin()->first;
  this->AddTexCoordBySet(_u, _v, firstSetIndex);
}

//////////////////////////////////////////////////
void SubMesh::AddTexCoord(const gz::math::Vector2d &_uv)
{
  this->AddTexCoord(_uv.X(), _uv.Y());
}

//////////////////////////////////////////////////
void SubMesh::AddTexCoordBySet(double _u, double _v, unsigned int _setIndex)
{
  this->dataPtr->texCoords[_setIndex].push_back(
      gz::math::Vector2d(_u, _v));
}

//////////////////////////////////////////////////
void SubMesh::AddTexCoordBySet(const gz::math::Vector2d &_uv,
    unsigned int _setIndex)
{
  this->AddTexCoordBySet(_uv.X(), _uv.Y(), _setIndex);
}

//////////////////////////////////////////////////
void SubMesh::AddNodeAssignment(const unsigned int _vertex,
    const unsigned int _node, const float _weight)
{
  NodeAssignment na;
  na.vertexIndex = _vertex;
  na.nodeIndex = _node;
  na.weight = _weight;

  this->dataPtr->nodeAssignments.push_back(na);
}

//////////////////////////////////////////////////
gz::math::Vector3d SubMesh::Vertex(const unsigned int _index) const
{
  if (_index >= this->dataPtr->vertices.size())
  {
    gzerr << "Index too large" << std::endl;
    return math::Vector3d::Zero;
  }

  return this->dataPtr->vertices[_index];
}

//////////////////////////////////////////////////
const gz::math::Vector3d* SubMesh::VertexPtr() const
{
  return this->dataPtr->vertices.data();
}

//////////////////////////////////////////////////
bool SubMesh::HasVertex(const unsigned int _index) const
{
  return _index < this->dataPtr->vertices.size();
}

//////////////////////////////////////////////////
void SubMesh::SetVertex(const unsigned int _index,
    const gz::math::Vector3d &_v)
{
  if (_index >= this->dataPtr->vertices.size())
  {
    gzerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->vertices[_index] = _v;
}

//////////////////////////////////////////////////
gz::math::Vector3d SubMesh::Normal(const unsigned int _index) const
{
  if (_index >= this->dataPtr->normals.size())
  {
    gzerr << "Index too large" << std::endl;
    return math::Vector3d::Zero;
  }

  return this->dataPtr->normals[_index];
}

//////////////////////////////////////////////////
bool SubMesh::HasNormal(const unsigned int _index) const
{
  return _index < this->dataPtr->normals.size();
}

//////////////////////////////////////////////////
bool SubMesh::HasTexCoord(const unsigned int _index) const
{
  if (this->dataPtr->texCoords.empty())
    return false;

  unsigned firstSetIndex = this->dataPtr->texCoords.begin()->first;

  if (this->dataPtr->texCoords.size() > 1u)
  {
    gzwarn << "Multiple texture coordinate sets exist in submesh: "
            << this->dataPtr->name << ". Checking first set with index: "
            << firstSetIndex << std::endl;
  }

  return this->HasTexCoordBySet(_index, firstSetIndex);
}

//////////////////////////////////////////////////
bool SubMesh::HasTexCoordBySet(unsigned int _index,
    unsigned int _setIndex) const
{
  auto it = this->dataPtr->texCoords.find(_setIndex);
  if (it == this->dataPtr->texCoords.end())
    return false;
  return _index < it->second.size();
}

//////////////////////////////////////////////////
bool SubMesh::HasNodeAssignment(const unsigned int _index) const
{
  return _index < this->dataPtr->nodeAssignments.size();
}

//////////////////////////////////////////////////
void SubMesh::SetNormal(const unsigned int _index,
    const gz::math::Vector3d &_n)
{
  if (_index >= this->dataPtr->normals.size())
  {
    gzerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->normals[_index] = _n;
}

//////////////////////////////////////////////////
gz::math::Vector2d SubMesh::TexCoord(const unsigned int _index) const
{
  if (this->dataPtr->texCoords.empty())
  {
    gzerr << "Texture coordinate sets are empty" << std::endl;
    return math::Vector2d::Zero;
  }
  unsigned firstSetIndex = this->dataPtr->texCoords.begin()->first;

  if (this->dataPtr->texCoords.size() > 1u)
  {
    gzwarn << "Multiple texture coordinate sets exist in submesh: "
            << this->dataPtr->name << ". Checking first set with index: "
            << firstSetIndex << std::endl;
  }

  return this->TexCoordBySet(_index, firstSetIndex);
}

//////////////////////////////////////////////////
gz::math::Vector2d SubMesh::TexCoordBySet(unsigned int _index,
    unsigned int _setIndex) const
{
  auto it = this->dataPtr->texCoords.find(_setIndex);
  if (it == this->dataPtr->texCoords.end())
  {
    gzerr << "Texture coordinate set does not exist: " << _setIndex
           << std::endl;
    return math::Vector2d::Zero;
  }

  if (_index >= it->second.size())
  {
    gzerr << "Index too large" << std::endl;
    return math::Vector2d::Zero;
  }

  return it->second[_index];
}

//////////////////////////////////////////////////
void SubMesh::SetTexCoord(const unsigned int _index,
    const gz::math::Vector2d &_t)
{
  unsigned firstSetIndex = 0u;
  if (!this->dataPtr->texCoords.empty())
    firstSetIndex = this->dataPtr->texCoords.begin()->first;

  if (this->dataPtr->texCoords.size() > 1u)
  {
    gzwarn << "Multiple texture coordinate sets exist in submesh: "
            << this->dataPtr->name << ". Checking first set with index: "
            << firstSetIndex << std::endl;
  }

  this->SetTexCoordBySet(_index, _t, firstSetIndex);
}

//////////////////////////////////////////////////
void SubMesh::SetTexCoordBySet(unsigned int _index,
    const gz::math::Vector2d &_t, unsigned int _setIndex)
{
  auto it = this->dataPtr->texCoords.find(_setIndex);
  if (it == this->dataPtr->texCoords.end())
  {
    gzerr << "Texture coordinate set does not exist: " << _setIndex
           << std::endl;
    return;
  }

  if (_index >= it->second.size())
  {
    gzerr << "Index too large" << std::endl;
    return;
  }

  it->second[_index] = _t;
}

//////////////////////////////////////////////////
int SubMesh::Index(const unsigned int _index) const
{
  if (_index >= this->dataPtr->indices.size())
  {
    gzerr << "Index too large" << std::endl;
    return -1;
  }

  return this->dataPtr->indices[_index];
}

//////////////////////////////////////////////////
const unsigned int* SubMesh::IndexPtr() const
{
  return this->dataPtr->indices.data();
}

//////////////////////////////////////////////////
void SubMesh::SetIndex(const unsigned int _index, const unsigned int _i)
{
  if (_index >= this->dataPtr->indices.size())
  {
    gzerr << "Index too large" << std::endl;
    return;
  }

  this->dataPtr->indices[_index] = _i;
}

//////////////////////////////////////////////////
NodeAssignment SubMesh::NodeAssignmentByIndex(
    const unsigned int _index) const
{
  if (_index >= this->dataPtr->nodeAssignments.size())
  {
    gzerr << "Index too large" << std::endl;
    return NodeAssignment();
  }

  return this->dataPtr->nodeAssignments[_index];
}

//////////////////////////////////////////////////
gz::math::Vector3d SubMesh::Max() const
{
  if (this->dataPtr->vertices.empty())
    return gz::math::Vector3d::Zero;

  gz::math::Vector3d max;

  max.X(-gz::math::MAX_F);
  max.Y(-gz::math::MAX_F);
  max.Z(-gz::math::MAX_F);

  for (const auto &v : this->dataPtr->vertices)
  {
    max.X(std::max(max.X(), v.X()));
    max.Y(std::max(max.Y(), v.Y()));
    max.Z(std::max(max.Z(), v.Z()));
  }

  return max;
}

//////////////////////////////////////////////////
gz::math::Vector3d SubMesh::Min() const
{
  if (this->dataPtr->vertices.empty())
    return gz::math::Vector3d::Zero;

  gz::math::Vector3d min;

  min.X(gz::math::MAX_F);
  min.Y(gz::math::MAX_F);
  min.Z(gz::math::MAX_F);

  for (const auto &v : this->dataPtr->vertices)
  {
    min.X(std::min(min.X(), v.X()));
    min.Y(std::min(min.Y(), v.Y()));
    min.Z(std::min(min.Z(), v.Z()));
  }

  return min;
}

//////////////////////////////////////////////////
unsigned int SubMesh::VertexCount() const
{
  return this->dataPtr->vertices.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::NormalCount() const
{
  return this->dataPtr->normals.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::IndexCount() const
{
  return this->dataPtr->indices.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::TexCoordCount() const
{
  if (this->dataPtr->texCoords.empty())
    return 0u;
  unsigned firstSetIndex = this->dataPtr->texCoords.begin()->first;

  if (this->dataPtr->texCoords.size() > 1u)
  {
    gzwarn << "Multiple texture coordinate sets exist in submesh: "
            << this->dataPtr->name << ". Checking first set with index: "
            << firstSetIndex << std::endl;
  }

  return this->TexCoordCountBySet(firstSetIndex);
}

//////////////////////////////////////////////////
unsigned int SubMesh::TexCoordCountBySet(unsigned int _setIndex) const
{
  auto it = this->dataPtr->texCoords.find(_setIndex);
  if (it == this->dataPtr->texCoords.end())
    return 0u;

  return it->second.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::TexCoordSetCount() const
{
  return this->dataPtr->texCoords.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::NodeAssignmentsCount() const
{
  return this->dataPtr->nodeAssignments.size();
}

//////////////////////////////////////////////////
unsigned int SubMesh::MaxIndex() const
{
  auto maxIter = std::max_element(this->dataPtr->indices.begin(),
      this->dataPtr->indices.end());

  if (maxIter != this->dataPtr->indices.end())
    return *maxIter;

  return 0;
}

//////////////////////////////////////////////////
void SubMesh::SetMaterialIndex(const unsigned int _index)
{
  this->dataPtr->materialIndex = _index;
}

//////////////////////////////////////////////////
std::optional<unsigned int> SubMesh::GetMaterialIndex() const
{
  return this->dataPtr->materialIndex;
}

//////////////////////////////////////////////////
bool SubMesh::HasVertex(const gz::math::Vector3d &_v) const
{
  for (const auto &v : this->dataPtr->vertices)
    if (_v.Equal(v))
      return true;

  return false;
}

//////////////////////////////////////////////////
int SubMesh::IndexOfVertex(const gz::math::Vector3d &_v) const
{
  for (auto iter = this->dataPtr->vertices.begin();
      iter != this->dataPtr->vertices.end(); ++iter)
  {
    if (_v.Equal(*iter))
      return iter - this->dataPtr->vertices.begin();
  }
  return -1;
}

//////////////////////////////////////////////////
void SubMesh::FillArrays(double **_vertArr, int **_indArr) const
{
  if (this->dataPtr->vertices.empty() || this->dataPtr->indices.empty())
  {
    gzerr << "No vertices or indices\n";
    return;
  }

  if (*_vertArr)
    delete [] *_vertArr;

  if (*_indArr)
    delete [] *_indArr;

  *_vertArr = new double[this->dataPtr->vertices.size() * 3];
  *_indArr = new int[this->dataPtr->indices.size()];

  unsigned int vi = 0;
  for (auto &v : this->dataPtr->vertices)
  {
    (*_vertArr)[vi++] = static_cast<float>(v.X());
    (*_vertArr)[vi++] = static_cast<float>(v.Y());
    (*_vertArr)[vi++] = static_cast<float>(v.Z());
  }

  unsigned int ii = 0;
  for (auto &i : this->dataPtr->indices)
  {
    (*_indArr)[ii++] = i;
  }
}

namespace {
// Simple way to find neighbors by grouping all vertices
// by X coordinate in (ordered) map. KD-tree maybe better
// but not sure about construction overhead
struct Neighbors
{
  Neighbors(const std::vector<unsigned int> &_indices,
            const std::vector<gz::math::Vector3d> &_vertices)
    : vertices(_vertices)
  {
    for (unsigned int i = 0; i < _indices.size(); ++i)
    {
      const auto index = _indices[i];
      this->groups[_vertices[index].X()].push_back(index);
    }
  }

  // When we have a concrete point to check, we are looking for
  // a group inside a map with a same X.
  // Then we check neighbors with the smaller X until
  // it's in tolerance of the math::equal function.
  // Starting from smallest X, which is in a tolerance range,
  // testing all points in group for equality. In case of equality,
  // call a Visitor with element index as an argument.
  // Continue until a greater side of X tolerance range reached.
  template<typename Visitor>
  void Visit(const gz::math::Vector3d &_point, Visitor _v) const
  {
    auto it = this->groups.find(_point.X());
    // find smaller acceptable value
    while (it != this->groups.begin())
    {
      auto prev = it;
      --prev;
      if (!gz::math::equal(prev->first, _point.X()))
        break;
      it = prev;
    }
    while (it != this->groups.end()
           && gz::math::equal(it->first, _point.X()))
    {
      for (const auto index : it->second)
        if (this->vertices[index] == _point)
          _v(index);
      ++it;
    }
  }

  // Indexes of vertices grouped by X coordinate
  private: std::map<double, std::vector<unsigned int>> groups;
  // Const reference to a vertices vector
  private: const std::vector<gz::math::Vector3d> &vertices;
};
}  // namespace

//////////////////////////////////////////////////
void SubMesh::RecalculateNormals()
{
  if (this->dataPtr->primitiveType != SubMesh::TRIANGLES
      || this->dataPtr->indices.size() % 3u != 0)
    return;

  if (!this->HasValidIndices())
    return;

  // Reset all the normals
  for (auto &n : this->dataPtr->normals)
    n.Set(0, 0, 0);

  if (this->dataPtr->normals.size() != this->dataPtr->vertices.size())
    this->dataPtr->normals.resize(this->dataPtr->vertices.size());

  Neighbors neighbors(this->dataPtr->indices, this->dataPtr->vertices);

  // For each face, which is defined by three indices, calculate the normals
  for (unsigned int i = 0; i < this->dataPtr->indices.size(); i+= 3)
  {
    gz::math::Vector3d v1 =
        this->dataPtr->vertices[this->dataPtr->indices[i]];
    gz::math::Vector3d v2 =
        this->dataPtr->vertices[this->dataPtr->indices[i+1]];
    gz::math::Vector3d v3 =
        this->dataPtr->vertices[this->dataPtr->indices[i+2]];
    gz::math::Vector3d n = gz::math::Vector3d::Normal(v1, v2, v3);

    for (const auto &point : {v1, v2, v3})
      neighbors.Visit(point, [&](const unsigned int index)
      {
        this->dataPtr->normals[index] += n;
      });
  }

  // Normalize the results
  for (auto &n : this->dataPtr->normals)
  {
    n.Normalize();
  }
}

//////////////////////////////////////////////////
void SubMesh::GenSphericalTexCoord(const gz::math::Vector3d &_center)
{
  if (this->dataPtr->texCoords.empty())
    return;

  unsigned firstSetIndex = this->dataPtr->texCoords.begin()->first;
  this->GenSphericalTexCoordBySet(_center, firstSetIndex);
}

//////////////////////////////////////////////////
void SubMesh::GenSphericalTexCoordBySet(const gz::math::Vector3d &_center,
    unsigned int _setIndex)
{
  this->dataPtr->texCoords[_setIndex].clear();

  for (const auto &vert : this->dataPtr->vertices)
  {
    // generate projected texture coordinates, projected from center
    //  x, y, z for computing texture coordinate projections
    double x = vert.X() - _center.X();
    double y = vert.Y() - _center.Y();
    double z = vert.Z() - _center.Z();

    double r = std::max(0.000001, sqrt(x*x+y*y+z*z));
    double s = std::min(1.0, std::max(-1.0, z/r));
    double t = std::min(1.0, std::max(-1.0, y/r));
    double u = acos(s) / GZ_PI;
    double v = acos(t) / GZ_PI;
    this->AddTexCoordBySet(u, v, _setIndex);
  }
}

//////////////////////////////////////////////////
void SubMesh::Scale(const gz::math::Vector3d &_factor)
{
  for (auto &v : this->dataPtr->vertices)
    v *= _factor;
}

//////////////////////////////////////////////////
void SubMesh::Scale(const double &_factor)
{
  for (auto &v : this->dataPtr->vertices)
    v *= _factor;
}

//////////////////////////////////////////////////
void SubMesh::Center(const gz::math::Vector3d &_center)
{
  gz::math::Vector3d min, max, half;
  min = this->Min();
  max = this->Max();
  half = (max - min) * 0.5;

  this->Translate(_center - (min + half));
}

//////////////////////////////////////////////////
void SubMesh::Translate(const gz::math::Vector3d &_vec)
{
  for (auto &v : this->dataPtr->vertices)
    v += _vec;
}

//////////////////////////////////////////////////
void SubMesh::SetName(const std::string &_name)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
std::string SubMesh::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
double SubMesh::Volume() const
{
  if (!this->HasValidIndices())
    return 0.0;

  double volume = 0.0;
  if (this->dataPtr->primitiveType == SubMesh::TRIANGLES)
  {
    if (this->dataPtr->indices.size() % 3 == 0)
    {
      for (unsigned int idx = 0; idx < this->dataPtr->indices.size(); idx += 3)
      {
        gz::math::Vector3d v1 =
          this->dataPtr->vertices[this->dataPtr->indices[idx]];
        gz::math::Vector3d v2 =
          this->dataPtr->vertices[this->dataPtr->indices[idx+1]];
        gz::math::Vector3d v3 =
          this->dataPtr->vertices[this->dataPtr->indices[idx+2]];

        volume += std::abs(v1.Cross(v2).Dot(v3) / 6.0);
      }
    }
    else
    {
      gzerr << "The number of indices is not a multiple of three.\n";
    }
  }
  else
  {
    gzerr << "Volume calculation can only be accomplished on a triangulated "
      << " mesh.\n";
  }

  return volume;
}

//////////////////////////////////////////////////
bool SubMesh::HasValidIndices() const
{
  if (this->dataPtr->indices.empty())
    return false;

  for (unsigned int idx = 0u; idx < this->dataPtr->indices.size(); ++idx)
  {
    if (this->dataPtr->indices[idx] >= this->dataPtr->vertices.size())
      return false;
  }
  return true;
}

//////////////////////////////////////////////////
NodeAssignment::NodeAssignment()
  : vertexIndex(0), nodeIndex(0), weight(0.0)
{
}

