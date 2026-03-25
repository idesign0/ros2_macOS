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
#include <map>
#include "gz/common/Console.hh"
#include "gz/common/NodeAnimation.hh"

using namespace gz;
using namespace common;

/// \brief NodeAnimation private data
class gz::common::NodeAnimation::Implementation
{
  /// \brief the name of the animation
  public: std::string name;

  /// \brief the dictionary of key frames, indexed by time
  public: std::map<double, math::Matrix4d> keyFrames;

  /// \brief the duration of the animations (time of last key frame)
  public: double length = 0.0;
};

//////////////////////////////////////////////////
NodeAnimation::NodeAnimation(const std::string &_name)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
NodeAnimation::~NodeAnimation()
{
}

//////////////////////////////////////////////////
void NodeAnimation::SetName(const std::string &_name)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
std::string NodeAnimation::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
void NodeAnimation::AddKeyFrame(const double _time,
    const math::Matrix4d &_trans)
{
  if (_time > this->dataPtr->length)
    this->dataPtr->length = _time;

  this->dataPtr->keyFrames[_time] = _trans;
}

//////////////////////////////////////////////////
void NodeAnimation::AddKeyFrame(const double _time, const math::Pose3d &_pose)
{
  math::Matrix4d mat(_pose.Rot());
  mat.SetTranslation(_pose.Pos());

  this->AddKeyFrame(_time, mat);
}

//////////////////////////////////////////////////
unsigned int NodeAnimation::FrameCount() const
{
  return this->dataPtr->keyFrames.size();
}

//////////////////////////////////////////////////
void NodeAnimation::KeyFrame(const unsigned int _i, double &_time,
        math::Matrix4d &_trans) const
{
  if (_i >= this->dataPtr->keyFrames.size())
  {
    gzerr << "Invalid key frame index " << _i << "\n";
    _time = -1.0;
  }
  else
  {
    std::map<double, math::Matrix4d>::const_iterator iter =
      this->dataPtr->keyFrames.begin();

    std::advance(iter, _i);

    _time = iter->first;
    _trans = iter->second;
  }
}

//////////////////////////////////////////////////
std::pair<double, math::Matrix4d> NodeAnimation::KeyFrame(
  const unsigned int _i) const
{
  double t;
  math::Matrix4d mat(math::Matrix4d::Identity);
  this->KeyFrame(_i, t, mat);

  return std::make_pair(t, mat);
}

//////////////////////////////////////////////////
double NodeAnimation::Length() const
{
  return this->dataPtr->length;
}

//////////////////////////////////////////////////
math::Matrix4d NodeAnimation::FrameAt(double _time, bool _loop) const
{
  double time = _time;
  if (time > this->dataPtr->length)
  {
    if (_loop)
    {
      while (time > this->dataPtr->length)
        time = time - this->dataPtr->length;
    }
    else
    {
      time = this->dataPtr->length;
    }
  }

  if (math::equal(time, this->dataPtr->length))
    return this->dataPtr->keyFrames.rbegin()->second;

  std::map<double, math::Matrix4d>::const_iterator it1 =
    this->dataPtr->keyFrames.upper_bound(time);

  if (it1 == this->dataPtr->keyFrames.begin() || math::equal(it1->first, time))
    return it1->second;

  std::map<double, math::Matrix4d>::const_iterator it2 = it1--;

  if (math::equal(it2->first, time))
    return it2->second;

  double nextKey = it2->first;
  math::Matrix4d nextTrans = it2->second;
  double prevKey = it1->first;
  math::Matrix4d prevTrans = it1->second;

  double t = (time - prevKey) / (nextKey - prevKey);

  if (t < 0.0 || t > 1.0)
  {
    gzerr << "Invalid time range\n";
    return math::Matrix4d();
  }

  math::Vector3d nextPos = nextTrans.Translation();
  math::Vector3d prevPos = prevTrans.Translation();
  math::Vector3d pos = math::Vector3d(
      prevPos.X() + ((nextPos.X() - prevPos.X()) * t),
      prevPos.Y() + ((nextPos.Y() - prevPos.Y()) * t),
      prevPos.Z() + ((nextPos.Z() - prevPos.Z()) * t));

  math::Quaterniond nextRot = nextTrans.Rotation();
  math::Quaterniond prevRot = prevTrans.Rotation();
  math::Quaterniond rot = math::Quaterniond::Slerp(t, prevRot, nextRot, true);

  math::Matrix4d trans(rot);
  trans.SetTranslation(pos);

  return trans;
}

//////////////////////////////////////////////////
void NodeAnimation::Scale(const double _scale)
{
  for (std::map<double, math::Matrix4d>::iterator iter =
      this->dataPtr->keyFrames.begin();
      iter != this->dataPtr->keyFrames.end(); ++iter)
  {
    math::Matrix4d *mat = &iter->second;
    math::Vector3d pos = mat->Translation();
    mat->SetTranslation(pos * _scale);
  }
}

//////////////////////////////////////////////////
double NodeAnimation::TimeAtX(const double _x) const
{
  std::map<double, math::Matrix4d>::const_iterator it1 =
    this->dataPtr->keyFrames.begin();

  while (it1->second.Translation().X() < _x)
    ++it1;

  if (it1 == this->dataPtr->keyFrames.begin() ||
      math::equal(it1->second.Translation().X(), _x))
  {
    return it1->first;
  }

  std::map<double, math::Matrix4d>::const_iterator it2 = it1--;
  double x1 = it1->second.Translation().X();
  double x2 = it2->second.Translation().X();
  double t1 = it1->first;
  double t2 = it2->first;

  return t1 + ((t2 - t1) * (_x - x1) / (x2 - x1));
}


