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

#include <gz/common/NodeTransform.hh>

#include <iostream>

using namespace gz;
using namespace common;

/// \brief Private data for NodeTransform
class gz::common::NodeTransform::Implementation
{
  /// \brief the sid
  public: std::string sid;

  /// \brief transform type
  public: NodeTransformType type;

  /// \brief transform
  public: math::Matrix4d transform;

  /// \brief source data values (can be a matrix, a position or rotation)
  public: std::vector<double> source;
};

//////////////////////////////////////////////////
NodeTransform::NodeTransform(const NodeTransformType _type)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->sid = "_default_";
  this->dataPtr->type = _type;
  this->dataPtr->transform = math::Matrix4d::Identity;
}

//////////////////////////////////////////////////
NodeTransform::NodeTransform(const math::Matrix4d &_mat,
    const std::string &_sid, const NodeTransformType _type)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->sid = _sid;
  this->dataPtr->type = _type;
  this->dataPtr->transform = _mat;
}

//////////////////////////////////////////////////
NodeTransform::~NodeTransform() = default;

//////////////////////////////////////////////////
void NodeTransform::Set(const math::Matrix4d &_mat)
{
  this->dataPtr->transform = _mat;
}

//////////////////////////////////////////////////
void NodeTransform::SetType(const NodeTransformType _type)
{
  this->dataPtr->type = _type;
}

//////////////////////////////////////////////////
void NodeTransform::SetSID(const std::string &_sid)
{
  this->dataPtr->sid = _sid;
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::Get() const
{
  return this->dataPtr->transform;
}

//////////////////////////////////////////////////
NodeTransformType NodeTransform::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
std::string NodeTransform::SID() const
{
  return this->dataPtr->sid;
}

//////////////////////////////////////////////////
void NodeTransform::SetComponent(const unsigned int _idx, const double _value)
{
  this->dataPtr->source[_idx] = _value;
}

//////////////////////////////////////////////////
void NodeTransform::SetSourceValues(const math::Matrix4d &_mat)
{
  this->dataPtr->source.resize(16);
  unsigned int idx = 0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    for (unsigned int j = 0; j < 4; ++j, ++idx)
    {
      this->dataPtr->source[idx] = _mat(i, j);
    }
  }
}

//////////////////////////////////////////////////
void NodeTransform::SetSourceValues(const math::Vector3d &_vec)
{
  this->dataPtr->source.resize(3);
  this->dataPtr->source[0] = _vec.X();
  this->dataPtr->source[1] = _vec.Y();
  this->dataPtr->source[2] = _vec.Z();
}

//////////////////////////////////////////////////
void NodeTransform::SetSourceValues(const math::Vector3d &_axis,
    const double _angle)
{
  this->dataPtr->source.resize(4);
  this->dataPtr->source[0] = _axis.X();
  this->dataPtr->source[1] = _axis.Y();
  this->dataPtr->source[2] = _axis.Z();
  this->dataPtr->source[3] = _angle;
}

//////////////////////////////////////////////////
void NodeTransform::RecalculateMatrix()
{
  if (this->dataPtr->type == MATRIX)
  {
    this->dataPtr->transform.Set(
        this->dataPtr->source[0], this->dataPtr->source[1],
        this->dataPtr->source[2], this->dataPtr->source[3],
        this->dataPtr->source[4], this->dataPtr->source[5],
        this->dataPtr->source[6], this->dataPtr->source[7],
        this->dataPtr->source[8], this->dataPtr->source[9],
        this->dataPtr->source[10], this->dataPtr->source[11],
        this->dataPtr->source[12], this->dataPtr->source[13],
        this->dataPtr->source[14], this->dataPtr->source[15]);
  }
  else
  {
    if (this->dataPtr->type == TRANSLATE)
    {
      this->dataPtr->transform.SetTranslation(math::Vector3d(
            this->dataPtr->source[0], this->dataPtr->source[1],
            this->dataPtr->source[2]));
    }
    else
    {
      if (this->dataPtr->type == ROTATE)
      {
        math::Matrix3d mat;
        mat.SetFromAxisAngle(math::Vector3d(this->dataPtr->source[0],
                                            this->dataPtr->source[1],
                                            this->dataPtr->source[2]),
                             GZ_DTOR(this->dataPtr->source[3]));
        this->dataPtr->transform = mat;
      }
      else
      {
        this->dataPtr->transform.Scale(math::Vector3d(this->dataPtr->source[0],
            this->dataPtr->source[1], this->dataPtr->source[2]));
      }
    }
  }
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::operator()() const
{
  return this->dataPtr->transform;
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::operator*(const NodeTransform &_t) const
{
  return this->dataPtr->transform * _t();
}

//////////////////////////////////////////////////
math::Matrix4d NodeTransform::operator*(const math::Matrix4d &_m) const
{
  return this->dataPtr->transform * _m;
}

//////////////////////////////////////////////////
void NodeTransform::PrintSource() const
{
  std::cout << this->dataPtr->sid;
  for (unsigned int i = 0; i < this->dataPtr->source.size(); ++i)
    std::cout << " " << this->dataPtr->source[i];
  std::cout << "\n";
}
