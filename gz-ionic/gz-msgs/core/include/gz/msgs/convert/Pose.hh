/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#ifndef GZ_MSGS_CONVERT_POSE_HH_
#define GZ_MSGS_CONVERT_POSE_HH_

#include <gz/msgs/config.hh>
#include <gz/msgs/convert/Vector3.hh>
#include <gz/msgs/convert/Quaternion.hh>

// Message Headers
#include "gz/msgs/quaternion.pb.h"
#include "gz/msgs/pose.pb.h"

// Data Headers
#include <gz/math/Pose3.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////
inline void Set(gz::msgs::Pose *_msg, const gz::math::Pose3d &_data)
{
  Set(_msg->mutable_position(), _data.Pos());
  Set(_msg->mutable_orientation(), _data.Rot());
}

inline void Set(gz::math::Pose3d *_data, const gz::msgs::Pose &_msg)
{
  gz::math::Vector3d pos;
  gz::math::Quaterniond orientation;

  if (_msg.has_position())
    pos = Convert(_msg.position());

  // This bit is critical.  If orientation hasn't been set in the message,
  // then we want the quaternion to default to identity.
  if (_msg.has_orientation())
    orientation = Convert(_msg.orientation());

  _data->Set(pos, orientation);
}

inline gz::msgs::Pose Convert(const gz::math::Pose3d &_data)
{
  gz::msgs::Pose ret;
  Set(&ret, _data);
  return ret;
}

inline gz::math::Pose3d Convert(const gz::msgs::Pose &_msg)
{
  gz::math::Pose3d ret;
  Set(&ret, _msg);
  return ret;
}
}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
