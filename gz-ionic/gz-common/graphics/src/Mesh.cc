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

#include <string>
#include <algorithm>

#include "gz/math/Helpers.hh"

#include "gz/common/Console.hh"
#include "gz/common/Material.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/Mesh.hh"

using namespace gz;
using namespace common;

/// \brief Private data for Mesh
class gz::common::Mesh::Implementation
{
  /// \brief The name of the mesh
  public: std::string name;

  /// \brief The path of the mesh resource
  public: std::string path;

  /// \brief The sub mesh array.
  public: std::vector<std::shared_ptr<SubMesh>> submeshes;

  /// \brief The materials array.
  public: std::vector<MaterialPtr> materials;

  /// \brief The skeleton (for animation)
  public: SkeletonPtr skeleton;
};

//////////////////////////////////////////////////
Mesh::Mesh()
: dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->name = "unknown";
  this->dataPtr->skeleton = NULL;
}

//////////////////////////////////////////////////
Mesh::~Mesh()
{
}

//////////////////////////////////////////////////
void Mesh::SetPath(const std::string &_path)
{
  this->dataPtr->path = _path;
}

//////////////////////////////////////////////////
std::string Mesh::Path() const
{
  return this->dataPtr->path;
}

//////////////////////////////////////////////////
void Mesh::SetName(const std::string &_name)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
std::string Mesh::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
gz::math::Vector3d Mesh::Max() const
{
  if (this->dataPtr->submeshes.empty())
    return gz::math::Vector3d::Zero;

  gz::math::Vector3d max;

  max.X(-gz::math::MAX_F);
  max.Y(-gz::math::MAX_F);
  max.Z(-gz::math::MAX_F);

  for (const auto &submesh : this->dataPtr->submeshes)
  {
    gz::math::Vector3d smax = submesh->Max();
    max.X(std::max(max.X(), smax.X()));
    max.Y(std::max(max.Y(), smax.Y()));
    max.Z(std::max(max.Z(), smax.Z()));
  }

  return max;
}

//////////////////////////////////////////////////
gz::math::Vector3d Mesh::Min() const
{
  if (this->dataPtr->submeshes.empty())
    return gz::math::Vector3d::Zero;

  gz::math::Vector3d min;

  min.X(gz::math::MAX_F);
  min.Y(gz::math::MAX_F);
  min.Z(gz::math::MAX_F);

  for (const auto &submesh : this->dataPtr->submeshes)
  {
    gz::math::Vector3d smin = submesh->Min();
    min.X(std::min(min.X(), smin.X()));
    min.Y(std::min(min.Y(), smin.Y()));
    min.Z(std::min(min.Z(), smin.Z()));
  }

  return min;
}

//////////////////////////////////////////////////
unsigned int Mesh::VertexCount() const
{
  unsigned int sum = 0;

  for (const auto &submesh : this->dataPtr->submeshes)
    sum += submesh->VertexCount();

  return sum;
}

//////////////////////////////////////////////////
unsigned int Mesh::NormalCount() const
{
  unsigned int sum = 0;

  for (const auto &submesh : this->dataPtr->submeshes)
    sum += submesh->NormalCount();

  return sum;
}

//////////////////////////////////////////////////
unsigned int Mesh::IndexCount() const
{
  unsigned int sum = 0;

  for (const auto &submesh : this->dataPtr->submeshes)
    sum += submesh->IndexCount();

  return sum;
}

//////////////////////////////////////////////////
unsigned int Mesh::TexCoordCount() const
{
  unsigned int sum = 0;

  for (const auto &submesh : this->dataPtr->submeshes)
    sum += submesh->TexCoordCount();

  return sum;
}

//////////////////////////////////////////////////
std::weak_ptr<SubMesh> Mesh::AddSubMesh(const SubMesh &_sub)
{
  auto sub = std::shared_ptr<SubMesh>(new SubMesh(_sub));
  this->dataPtr->submeshes.push_back(sub);
  return sub;
}

//////////////////////////////////////////////////
std::weak_ptr<SubMesh> Mesh::AddSubMesh(std::unique_ptr<SubMesh> _sub)
{
  auto sub = std::shared_ptr<SubMesh>(std::move(_sub));
  this->dataPtr->submeshes.push_back(sub);
  return sub;
}

//////////////////////////////////////////////////
unsigned int Mesh::SubMeshCount() const
{
  return this->dataPtr->submeshes.size();
}

//////////////////////////////////////////////////
std::weak_ptr<SubMesh> Mesh::SubMeshByIndex(unsigned int _index) const
{
  if (_index < this->dataPtr->submeshes.size())
    return this->dataPtr->submeshes[_index];

  gzerr << "Invalid index: " << _index << " >= " <<
      this->dataPtr->submeshes.size() << std::endl;

  return std::shared_ptr<SubMesh>(nullptr);
}

//////////////////////////////////////////////////
std::weak_ptr<SubMesh> Mesh::SubMeshByName(const std::string &_name) const
{
  // Find the submesh with the provided name.
  for (const auto &submesh : this->dataPtr->submeshes)
  {
    if (submesh->Name() == _name)
      return submesh;
  }

  return std::shared_ptr<SubMesh>(nullptr);
}

//////////////////////////////////////////////////
int Mesh::AddMaterial(const MaterialPtr &_mat)
{
  int result = -1;

  if (_mat)
  {
    this->dataPtr->materials.push_back(_mat);
    result = this->dataPtr->materials.size()-1;
  }

  return result;
}

//////////////////////////////////////////////////
unsigned int Mesh::MaterialCount() const
{
  return this->dataPtr->materials.size();
}

//////////////////////////////////////////////////
MaterialPtr Mesh::MaterialByIndex(const unsigned int index) const
{
  if (index < this->dataPtr->materials.size())
    return this->dataPtr->materials[index];

  return NULL;
}

//////////////////////////////////////////////////
int Mesh::IndexOfMaterial(const Material *_mat) const
{
  if (_mat)
  {
    for (unsigned int i = 0; i < this->dataPtr->materials.size(); ++i)
    {
      if (this->dataPtr->materials[i].get() == _mat)
        return i;
    }
  }

  return -1;
}

//////////////////////////////////////////////////
void Mesh::FillArrays(double **_vertArr, int **_indArr) const
{
  unsigned int vertCount = 0;
  unsigned int indCount = 0;

  for (const auto &submesh : this->dataPtr->submeshes)
  {
    vertCount += submesh->VertexCount();
    indCount += submesh->IndexCount();
  }

  if (*_vertArr)
    delete [] *_vertArr;

  if (*_indArr)
    delete [] *_indArr;

  *_vertArr = new double[vertCount * 3];
  *_indArr = new int[indCount];

  double *vPtr = *_vertArr;
  unsigned int index = 0;
  unsigned int offset = 0;

  for (const auto &submesh : this->dataPtr->submeshes)
  {
    double *vertTmp = NULL;
    int *indTmp = NULL;
    submesh->FillArrays(&vertTmp, &indTmp);

    memcpy(vPtr, vertTmp, sizeof(vertTmp[0])*submesh->VertexCount()*3);

    for (unsigned int i = 0; i < submesh->IndexCount(); ++i)
    {
      (*_indArr)[index++] = submesh->Index(i) + offset;
    }

    offset = offset + submesh->MaxIndex() + 1;

    vPtr += submesh->VertexCount()*3;

    delete [] vertTmp;
    delete [] indTmp;
  }
}

//////////////////////////////////////////////////
void Mesh::RecalculateNormals()
{
  for (auto &submesh : this->dataPtr->submeshes)
    submesh->RecalculateNormals();
}

//////////////////////////////////////////////////
void Mesh::SetSkeleton(const SkeletonPtr &_skel)
{
  this->dataPtr->skeleton = _skel;
}

//////////////////////////////////////////////////
SkeletonPtr Mesh::MeshSkeleton() const
{
  return this->dataPtr->skeleton;
}

//////////////////////////////////////////////////
bool Mesh::HasSkeleton() const
{
  return this->dataPtr->skeleton != nullptr;
}

//////////////////////////////////////////////////
void Mesh::Scale(const gz::math::Vector3d &_factor)
{
  for (auto &submesh : this->dataPtr->submeshes)
    submesh->Scale(_factor);
}

//////////////////////////////////////////////////
void Mesh::SetScale(const gz::math::Vector3d &_factor)
{
  for (auto &submesh : this->dataPtr->submeshes)
    submesh->Scale(_factor);
}

//////////////////////////////////////////////////
void Mesh::GenSphericalTexCoord(const gz::math::Vector3d &_center)
{
  for (auto &submesh : this->dataPtr->submeshes)
    submesh->GenSphericalTexCoord(_center);
}

//////////////////////////////////////////////////
void Mesh::Center(const gz::math::Vector3d &_center)
{
  gz::math::Vector3d min, max, half;
  min = this->Min();
  max = this->Max();
  half = (max - min) * 0.5;

  this->Translate(_center - (min + half));
}

//////////////////////////////////////////////////
void Mesh::Translate(const gz::math::Vector3d &_vec)
{
  for (auto &submesh : this->dataPtr->submeshes)
    submesh->Translate(_vec);
}

//////////////////////////////////////////////////
void Mesh::AABB(gz::math::Vector3d &_center,
                gz::math::Vector3d &_minXYZ,
                gz::math::Vector3d &_maxXYZ) const
{
  // find aabb center
  _minXYZ.X(1e15);
  _maxXYZ.X(-1e15);
  _minXYZ.Y(1e15);
  _maxXYZ.Y(-1e15);
  _minXYZ.Z(1e15);
  _maxXYZ.Z(-1e15);
  _center.X(0);
  _center.Y(0);
  _center.Z(0);

  for (auto const &submesh : this->dataPtr->submeshes)
  {
    gz::math::Vector3d max = submesh->Max();
    gz::math::Vector3d min = submesh->Min();

    _minXYZ.X(std::min(_minXYZ.X(), min.X()));
    _maxXYZ.X(std::max(_maxXYZ.X(), max.X()));
    _minXYZ.Y(std::min(_minXYZ.Y(), min.Y()));
    _maxXYZ.Y(std::max(_maxXYZ.Y(), max.Y()));
    _minXYZ.Z(std::min(_minXYZ.Z(), min.Z()));
    _maxXYZ.Z(std::max(_maxXYZ.Z(), max.Z()));
  }
  _center.X(0.5 * (_minXYZ.X() + _maxXYZ.X()));
  _center.Y(0.5 * (_minXYZ.Y() + _maxXYZ.Y()));
  _center.Z(0.5 * (_minXYZ.Z() + _maxXYZ.Z()));
}

//////////////////////////////////////////////////
double Mesh::Volume() const
{
  double volume = 0.0;

  for (const std::shared_ptr<SubMesh> &submesh : this->dataPtr->submeshes)
    volume += submesh->Volume();

  return volume;
}
