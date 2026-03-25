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
#include <gz/common/KeyFrame.hh>

namespace gz {
namespace common {

/////////////////////////////////////////////////
class KeyFrame::Implementation
{
  /// \brief time of key frame
  public: double time;
};

/////////////////////////////////////////////////
class PoseKeyFrame::Implementation
{
  /// \brief the translation vector
  public: math::Vector3d translate;

  /// \brief the rotation quaternion
  public: math::Quaterniond rotate;
};

/////////////////////////////////////////////////
class NumericKeyFrame::Implementation
{
  /// \brief numeric value
  public: double value;
};

/////////////////////////////////////////////////
KeyFrame::KeyFrame(const double _time)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->time = _time;
}

/////////////////////////////////////////////////
KeyFrame::~KeyFrame()
{
}

/////////////////////////////////////////////////
double KeyFrame::Time() const
{
  return this->dataPtr->time;
}


/////////////////////////////////////////////////
PoseKeyFrame::PoseKeyFrame(const double _time)
: KeyFrame(_time),
  dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
PoseKeyFrame::~PoseKeyFrame()
{
}

/////////////////////////////////////////////////
void PoseKeyFrame::Translation(const math::Vector3d &_trans)
{
  this->dataPtr->translate = _trans;
}

/////////////////////////////////////////////////
const math::Vector3d &PoseKeyFrame::Translation() const
{
  return this->dataPtr->translate;
}

/////////////////////////////////////////////////
void PoseKeyFrame::Rotation(const math::Quaterniond &_rot)
{
  this->dataPtr->rotate = _rot;
}

/////////////////////////////////////////////////
const math::Quaterniond &PoseKeyFrame::Rotation() const
{
  return this->dataPtr->rotate;
}

/////////////////////////////////////////////////
NumericKeyFrame::NumericKeyFrame(const double _time)
: KeyFrame(_time),
  dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
NumericKeyFrame::~NumericKeyFrame()
{
}

/////////////////////////////////////////////////
void NumericKeyFrame::Value(const double &_value)
{
  this->dataPtr->value = _value;
}

/////////////////////////////////////////////////
const double &NumericKeyFrame::Value() const
{
  return this->dataPtr->value;
}
}  // namespace gz
}  // namespace common
