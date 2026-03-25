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

#include "gz/common/Console.hh"
#include "gz/common/NodeAnimation.hh"
#include "gz/common/SkeletonAnimation.hh"

using namespace gz;
using namespace common;

/// Prvate data class
class gz::common::SkeletonAnimation::Implementation
{
  /// \brief the node name
  public: std::string name;

  /// \brief the duration of the longest animation
  public: double length;

  /// \brief a dictionary of node animations
  public: std::map<std::string, std::shared_ptr<NodeAnimation>> animations;
};

//////////////////////////////////////////////////
SkeletonAnimation::SkeletonAnimation(const std::string &_name)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
SkeletonAnimation::~SkeletonAnimation()
{
}

//////////////////////////////////////////////////
void SkeletonAnimation::SetName(const std::string &_name)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
std::string SkeletonAnimation::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
unsigned int SkeletonAnimation::NodeCount() const
{
  return this->dataPtr->animations.size();
}

//////////////////////////////////////////////////
NodeAnimation *SkeletonAnimation::NodeAnimationByName(
    const std::string &_node) const
{
  auto it = this->dataPtr->animations.find(_node);
  if (it != this->dataPtr->animations.end())
    return it->second.get();
  return nullptr;
}

//////////////////////////////////////////////////
bool SkeletonAnimation::XDisplacement() const
{
  bool xDisplacement = false;
  for (auto iter = this->dataPtr->animations.begin();
      iter != this->dataPtr->animations.end(); ++iter)
  {
    auto node = iter->second;
    math::Matrix4d lastPos = node->KeyFrame(node->FrameCount() - 1).second;
    math::Matrix4d firstPos = node->KeyFrame(0).second;
    xDisplacement = !math::equal(firstPos.Translation().X(),
                                 lastPos.Translation().X());
    if (xDisplacement)
    {
      break;
    }
  }
  return xDisplacement;
}

//////////////////////////////////////////////////
bool SkeletonAnimation::HasNode(const std::string &_node) const
{
  return (this->dataPtr->animations.find(_node) !=
      this->dataPtr->animations.end());
}

//////////////////////////////////////////////////
void SkeletonAnimation::AddKeyFrame(const std::string &_node,
    const double _time, const math::Matrix4d &_mat)
{
  if (this->dataPtr->animations.find(_node) == this->dataPtr->animations.end())
    this->dataPtr->animations[_node] = std::make_shared<NodeAnimation>(_node);

  if (_time > this->dataPtr->length)
    this->dataPtr->length = _time;

  this->dataPtr->animations[_node]->AddKeyFrame(_time, _mat);
}

//////////////////////////////////////////////////
void SkeletonAnimation::AddKeyFrame(const std::string &_node,
      const double _time, const math::Pose3d &_pose)
{
  if (this->dataPtr->animations.find(_node) == this->dataPtr->animations.end())
    this->dataPtr->animations[_node] = std::make_shared<NodeAnimation>(_node);

  if (_time > this->dataPtr->length)
    this->dataPtr->length = _time;

  this->dataPtr->animations[_node]->AddKeyFrame(_time, _pose);
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonAnimation::NodePoseAt(const std::string &_node,
    const double _time, const bool _loop) const
{
  math::Matrix4d mat;

  auto nodeAnim = this->dataPtr->animations.find(_node);
  if (nodeAnim != this->dataPtr->animations.end())
    mat = nodeAnim->second->FrameAt(_time, _loop);

  return mat;
}

//////////////////////////////////////////////////
std::map<std::string, math::Matrix4d> SkeletonAnimation::PoseAt(
                      const double _time, const bool _loop) const
{
  ///  TODO need to make sure that all nodes have keyframes at the same
  ///  points in time and create the missing keyframes. if the animation
  ///  comes from bvh this is guaranteed, but if it's comming from collada
  ///  it's not guaranteed. fixing this will help not having to find the
  ///  prev and next keyframe for each node at each time step, but rather
  ///  doing it only once per time step.
  std::map<std::string, math::Matrix4d> pose;
  for (auto iter = this->dataPtr->animations.begin();
      iter != this->dataPtr->animations.end(); ++iter)
  {
    pose[iter->first] = iter->second->FrameAt(_time, _loop);
  }

  return pose;
}

//////////////////////////////////////////////////
std::map<std::string, math::Matrix4d> SkeletonAnimation::PoseAtX(
             const double _x, const std::string &_node, const bool _loop) const
{
  auto nodeAnim = this->dataPtr->animations.find(_node);
  if (nodeAnim == this->dataPtr->animations.end())
  {
    gzerr << "Can't find animation named [" << _node << "]" << std::endl;
    return {};
  }

  math::Matrix4d lastPos = nodeAnim->second->KeyFrame(
      nodeAnim->second->FrameCount() - 1).second;

  math::Matrix4d firstPos = nodeAnim->second->KeyFrame(0).second;

  double x = _x;
  if (x < firstPos.Translation().X())
    x = firstPos.Translation().X();

  double lastX = lastPos.Translation().X();
  if (x > lastX && !_loop)
    x = lastX;
  while (x > lastX)
    x -= lastX;

  double time = nodeAnim->second->TimeAtX(x);

  return this->PoseAt(time, _loop);
}

//////////////////////////////////////////////////
void SkeletonAnimation::Scale(const double _scale)
{
  for (auto iter = this->dataPtr->animations.begin();
       iter != this->dataPtr->animations.end(); ++iter)
  {
    iter->second->Scale(_scale);
  }
}

//////////////////////////////////////////////////
double SkeletonAnimation::Length() const
{
  return this->dataPtr->length;
}
