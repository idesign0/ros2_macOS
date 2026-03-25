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
#include <algorithm>
#include <memory>

#include <gz/math/Spline.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/RotationSpline.hh>
#include <gz/common/Console.hh>
#include <gz/common/KeyFrame.hh>
#include <gz/common/Animation.hh>

using namespace gz;
using namespace common;

using KeyFrame_V = std::vector<std::shared_ptr<common::KeyFrame>>;

/// \brief Implementaiton of std::reinterpret_pointer_cast from
/// cppreference.
/// Needed because MacOS Catalina doesn't support this part of the standard
/// \todo(anyone) remove withs future versions of MacOS
template< class T, class U >
std::shared_ptr<T>
reinterpret_pointer_cast( const std::shared_ptr<U>& r ) noexcept
{
    auto p =
      reinterpret_cast<typename std::shared_ptr<T>::element_type*>(r.get());
    return std::shared_ptr<T>(r, p);
}

/////////////////////////////////////////////////
namespace
{
  struct KeyFrameTimeLess
  {
    bool operator() (const std::shared_ptr<common::KeyFrame> &_kf,
        const std::shared_ptr<common::KeyFrame> &_kf2) const
    {
      return _kf->Time() < _kf2->Time();
    }
  };
}

/////////////////////////////////////////////////
namespace gz {
namespace common {

class Animation::Implementation
{
  /// \brief true if the animation is interpolated in x
  public: bool interpolateX{false};

  /// \brief animation name
  public: std::string name;

  /// \brief animation duration
  public: double length;

  /// \brief current time position
  public: double timePos{0};

  /// \brief true if animation repeats
  public: bool loop;

  /// \brief array of key frames
  public: KeyFrame_V keyFrames;
};

/////////////////////////////////////////////////
class PoseAnimation::Implementation
{
  /// \brief smooth interpolation for position
  public: std::shared_ptr<math::Spline> positionSpline;

  /// \brief smooth interpolation for rotation
  public: std::shared_ptr<math::RotationSpline> rotationSpline;

  /// \brief determines if the interpolation splines need building
  public: bool build{false};

  /// \brief Spline tension parameter.
  public: double tension{0.0};
};

/////////////////////////////////////////////////
class NumericAnimation::Implementation
{
};

/////////////////////////////////////////////////
class TrajectoryInfo::Implementation
{
  /// \brief ID of the trajectory
  public: unsigned int id{0};

  /// \brief Type of trajectory. If it matches the name a skeleton
  /// animation, they will be played together
  public: unsigned int animIndex{0};

  /// \brief Start time of the trajectory.
  public: std::chrono::steady_clock::time_point startTime;

  /// \brief End time of the trajectory.
  public: std::chrono::steady_clock::time_point endTime;

  /// \brief True if the trajectory is translated.
  public: bool translated{false};

  /// \brief Waypoints in pose animation format.
  /// Times within the animation should be considered durations counted
  /// from start time.
  public: std::shared_ptr<PoseAnimation> waypoints;

  /// \brief Distance on the XY plane covered by each segment. The key is the
  /// duration from start time, the value is the distance in meters.
  public: std::map<std::chrono::steady_clock::duration, double> segDistance;
};

/////////////////////////////////////////////////
Animation::Animation(const std::string &_name, const double _length,
    const bool _loop)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->name = _name;
  this->dataPtr->length = _length;
  this->dataPtr->loop = _loop;
}

/////////////////////////////////////////////////
double Animation::Length() const
{
  return this->dataPtr->length;
}

/////////////////////////////////////////////////
void Animation::Length(const double _len)
{
  this->dataPtr->length = _len;
}

/////////////////////////////////////////////////
void Animation::Time(const double _time)
{
  if (!math::equal(_time, this->dataPtr->timePos))
  {
    this->dataPtr->timePos = _time;
    if (this->dataPtr->loop)
    {
      if (math::equal(this->dataPtr->length, 0.0))
      {
        this->dataPtr->timePos = 0;
      }
      else
      {
        this->dataPtr->timePos = fmod(this->dataPtr->timePos,
                                    this->dataPtr->length);
        if (this->dataPtr->timePos < 0)
          this->dataPtr->timePos += this->dataPtr->length;
      }
    }
    else
    {
      if (this->dataPtr->timePos < 0)
        this->dataPtr->timePos = 0;
      else if (this->dataPtr->timePos > this->dataPtr->length)
        this->dataPtr->timePos = this->dataPtr->length;
    }
  }
}

/////////////////////////////////////////////////
void Animation::AddTime(const double _time)
{
  this->Time(this->dataPtr->timePos + _time);
}

/////////////////////////////////////////////////
double Animation::Time() const
{
  return this->dataPtr->timePos;
}

/////////////////////////////////////////////////
bool Animation::InterpolateX() const
{
  return this->dataPtr->interpolateX;
}

/////////////////////////////////////////////////
void Animation::SetInterpolateX(const bool _interpolateX)
{
  this->dataPtr->interpolateX = _interpolateX;
}

/////////////////////////////////////////////////
unsigned int Animation::KeyFrameCount() const
{
  return this->dataPtr->keyFrames.size();
}

/////////////////////////////////////////////////
common::KeyFrame *Animation::KeyFrame(const unsigned int _index) const
{
  common::KeyFrame *result = NULL;

  if (_index < this->dataPtr->keyFrames.size())
    result = this->dataPtr->keyFrames[_index].get();
  else
  {
    gzerr << "Key frame index[" << _index
          << "] is larger than key frame array size["
          << this->dataPtr->keyFrames.size() << "]\n";
  }

  return result;
}

/////////////////////////////////////////////////
template<typename KeyFrameType>
KeyFrameType *Animation::CreateKeyFrame(const double _time)
{
  auto frame = std::make_shared<KeyFrameType>(_time);
  auto iter =
    std::upper_bound(this->dataPtr->keyFrames.begin(),
        this->dataPtr->keyFrames.end(),
        ::reinterpret_pointer_cast<common::KeyFrame>(frame),
        KeyFrameTimeLess());

  this->dataPtr->keyFrames.insert(iter, frame);
  return frame.get();
}

/////////////////////////////////////////////////
double Animation::KeyFramesAtTime(double _time, common::KeyFrame **_kf1,
    common::KeyFrame **_kf2, unsigned int &_firstKeyIndex) const
{
  // Parametric time
  // t1 = time of previous keyframe
  // t2 = time of next keyframe
  double t1, t2;

  // Find first key frame after or on current time
  while (_time > this->dataPtr->length && this->dataPtr->length > 0.0)
    _time -= this->dataPtr->length;

  KeyFrame_V::const_iterator iter;
  auto timeKey = std::make_shared<common::KeyFrame>(_time);
  iter = std::lower_bound(this->dataPtr->keyFrames.begin(),
      this->dataPtr->keyFrames.end(), timeKey, KeyFrameTimeLess());

  if (iter == this->dataPtr->keyFrames.end())
  {
    // There is no keyframe after this time, wrap back to first
    *_kf2 = this->dataPtr->keyFrames.front().get();
    t2 = this->dataPtr->length + (*_kf2)->Time();

    // Use the last keyframe as the previous keyframe
    --iter;
  }
  else
  {
    *_kf2 = iter->get();
    t2 = (*_kf2)->Time();

    // Find last keyframe before or on current time
    if (iter != this->dataPtr->keyFrames.begin() && _time < (*iter)->Time())
      --iter;
  }

  _firstKeyIndex = std::distance(this->dataPtr->keyFrames.cbegin(), iter);

  *_kf1 = iter->get();
  t1 = (*_kf1)->Time();

  if (math::equal(t1, t2))
    return 0.0;
  else
    return (_time - t1) / (t2 - t1);
}

/////////////////////////////////////////////////
PoseAnimation::PoseAnimation(const std::string &_name, const double _length,
    const bool _loop, double _tension)
: Animation(_name, _length, _loop),
  dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->tension = math::clamp(_tension, 0.0, 1.0);
}

/////////////////////////////////////////////////
PoseKeyFrame *PoseAnimation::CreateKeyFrame(const double _time)
{
  this->dataPtr->build = true;
  return Animation::CreateKeyFrame<PoseKeyFrame>(_time);
}

/////////////////////////////////////////////////
void PoseAnimation::BuildInterpolationSplines()
{
  if (!this->dataPtr->positionSpline)
    this->dataPtr->positionSpline = std::make_unique<math::Spline>();

  if (!this->dataPtr->rotationSpline)
    this->dataPtr->rotationSpline = std::make_unique<math::RotationSpline>();

  this->dataPtr->positionSpline->AutoCalculate(false);
  this->dataPtr->rotationSpline->AutoCalculate(false);

  this->dataPtr->positionSpline->Tension(this->dataPtr->tension);
  this->dataPtr->positionSpline->Clear();
  this->dataPtr->rotationSpline->Clear();

  for (size_t idx = 0; idx < this->KeyFrameCount(); ++idx)
  {
    PoseKeyFrame *pkey = reinterpret_cast<PoseKeyFrame*>(this->KeyFrame(idx));
    this->dataPtr->positionSpline->AddPoint(pkey->Translation());
    this->dataPtr->rotationSpline->AddPoint(pkey->Rotation());
  }

  this->dataPtr->positionSpline->RecalcTangents();
  this->dataPtr->rotationSpline->RecalcTangents();
  this->dataPtr->build = false;
}

/////////////////////////////////////////////////
void PoseAnimation::InterpolatedKeyFrame(PoseKeyFrame &_kf)
{
  this->InterpolatedKeyFrame(this->Time(), _kf);
}

/////////////////////////////////////////////////
void PoseAnimation::InterpolatedKeyFrame(const double _time,
                                         PoseKeyFrame &_kf)
{
  common::KeyFrame *kBase1, *kBase2;
  PoseKeyFrame *k1;
  unsigned int firstKeyIndex;

  if (this->dataPtr->build)
    this->BuildInterpolationSplines();

  double t = this->KeyFramesAtTime(_time, &kBase1, &kBase2, firstKeyIndex);

  k1 = reinterpret_cast<PoseKeyFrame*>(kBase1);

  if (math::equal(t, 0.0))
  {
    _kf.Translation(k1->Translation());
    _kf.Rotation(k1->Rotation());
  }
  else
  {
    _kf.Translation(
        this->dataPtr->positionSpline->Interpolate(firstKeyIndex, t));
    _kf.Rotation(this->dataPtr->rotationSpline->Interpolate(firstKeyIndex, t));
  }
}

/////////////////////////////////////////////////
NumericAnimation::NumericAnimation(const std::string &_name,
    const double _length, const bool _loop)
: Animation(_name, _length, _loop),
  dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
NumericKeyFrame *NumericAnimation::CreateKeyFrame(const double _time)
{
  return Animation::CreateKeyFrame<NumericKeyFrame>(_time);
}

/////////////////////////////////////////////////
void NumericAnimation::InterpolatedKeyFrame(NumericKeyFrame &_kf) const
{
  // Keyframe pointers
  common::KeyFrame *kBase1, *kBase2;
  NumericKeyFrame *k1, *k2;
  unsigned int firstKeyIndex;

  double t;
  t = this->KeyFramesAtTime(this->Time(), &kBase1, &kBase2, firstKeyIndex);

  k1 = reinterpret_cast<NumericKeyFrame*>(kBase1);
  k2 = reinterpret_cast<NumericKeyFrame*>(kBase2);

  if (math::equal(t, 0.0))
  {
    // Just use k1
    _kf.Value(k1->Value());
  }
  else
  {
    // Interpolate by t
    double diff = k2->Value() - k1->Value();
    _kf.Value(k1->Value() + diff * t);
  }
}

/////////////////////////////////////////////////
TrajectoryInfo::TrajectoryInfo()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
unsigned int TrajectoryInfo::Id() const
{
  return this->dataPtr->id;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetId(unsigned int _id)
{
  this->dataPtr->id = _id;
}

/////////////////////////////////////////////////
unsigned int TrajectoryInfo::AnimIndex() const
{
  return this->dataPtr->animIndex;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetAnimIndex(unsigned int _index)
{
  this->dataPtr->animIndex = _index;
}

/////////////////////////////////////////////////
std::chrono::steady_clock::duration TrajectoryInfo::Duration() const
{
  return this->dataPtr->endTime - this->dataPtr->startTime;
}

/////////////////////////////////////////////////
double TrajectoryInfo::DistanceSoFar(
    const std::chrono::steady_clock::duration &_time) const
{
  double distance = 0.0;

  // Sum all completed segments
  auto segment = this->dataPtr->segDistance.begin();
  auto prevSegment = segment;
  for (; segment != this->dataPtr->segDistance.end() && segment->first <= _time;
      prevSegment = segment, ++segment)
  {
    distance += segment->second;
  }

  if (segment == this->dataPtr->segDistance.end())
    return distance;

  // Add difference
  if (_time - prevSegment->first > std::chrono::steady_clock::duration())
  {
    distance += static_cast<double>((_time - prevSegment->first).count()) /
        static_cast<double>((segment->first - prevSegment->first).count()) *
        segment->second;
  }
  return distance;
}

/////////////////////////////////////////////////
std::chrono::steady_clock::time_point TrajectoryInfo::StartTime() const
{
  return this->dataPtr->startTime;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetStartTime(
    const std::chrono::steady_clock::time_point &_startTime)
{
  this->dataPtr->startTime = _startTime;
}

/////////////////////////////////////////////////
std::chrono::steady_clock::time_point TrajectoryInfo::EndTime() const
{
  return this->dataPtr->endTime;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetEndTime(
    const std::chrono::steady_clock::time_point &_endTime)
{
  this->dataPtr->endTime = _endTime;
}

/////////////////////////////////////////////////
bool TrajectoryInfo::Translated() const
{
  return this->dataPtr->translated;
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetTranslated(bool _translated)
{
  this->dataPtr->translated = _translated;
}

/////////////////////////////////////////////////
PoseAnimation *TrajectoryInfo::Waypoints() const
{
  return this->dataPtr->waypoints.get();
}

/////////////////////////////////////////////////
void TrajectoryInfo::SetWaypoints(
    std::map<std::chrono::steady_clock::time_point, math::Pose3d> _waypoints,
    double _tension)
{
  _tension = math::clamp(_tension, 0.0, 1.0);
  this->dataPtr->segDistance.clear();

  auto first = _waypoints.begin();
  auto last = _waypoints.rbegin();

  this->SetStartTime(first->first);
  this->SetEndTime(last->first);

  std::stringstream animName;
  animName << this->AnimIndex() << "_" << this->Id();
  std::shared_ptr<PoseAnimation> anim =
      std::make_shared<PoseAnimation>(animName.str(),
      std::chrono::duration<double>(this->Duration()).count(), false, _tension);

  auto prevPose = first->second.Pos();
  for (auto pIter = _waypoints.begin(); pIter != _waypoints.end(); ++pIter)
  {
    auto key = anim->CreateKeyFrame(
      std::chrono::duration<double>(pIter->first - this->StartTime()).count());

    math::Vector2d p1(prevPose.X(), prevPose.Y());
    math::Vector2d p2(pIter->second.Pos().X(), pIter->second.Pos().Y());
    this->dataPtr->segDistance[pIter->first - this->StartTime()] =
        p1.Distance(p2);

    key->Translation(pIter->second.Pos());
    key->Rotation(pIter->second.Rot());

    prevPose = pIter->second.Pos();
  }

  this->dataPtr->waypoints = anim;
  this->dataPtr->translated = true;
}
}  // namespace gz
}  // namespace common
