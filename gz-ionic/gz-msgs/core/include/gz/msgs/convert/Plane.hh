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
#ifndef GZ_MSGS_CONVERT_PLANE_HH_
#define GZ_MSGS_CONVERT_PLANE_HH_

#include <gz/msgs/config.hh>
#include <gz/msgs/convert/Vector2.hh>
#include <gz/msgs/convert/Vector3.hh>

// Message Headers
#include "gz/msgs/planegeom.pb.h"

// Data Headers
#include <gz/math/Plane.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////
inline void Set(gz::msgs::PlaneGeom *_msg, const gz::math::Planed &_data)
{
  Set(_msg->mutable_normal(), _data.Normal());
  _msg->mutable_size()->set_x(_data.Size().X());
  _msg->mutable_size()->set_y(_data.Size().Y());
  _msg->set_d(_data.Offset());
}

inline void Set(gz::math::Planed *_data, const gz::msgs::PlaneGeom &_msg)
{
  _data->Set(
    Convert(_msg.normal()),
    Convert(_msg.size()),
    _msg.d()
  );
}

inline gz::msgs::PlaneGeom Convert(const gz::math::Planed &_data)
{
  gz::msgs::PlaneGeom ret;
  Set(&ret, _data);
  return ret;
}

inline gz::math::Planed Convert(const gz::msgs::PlaneGeom &_msg)
{
  gz::math::Planed ret;
  Set(&ret, _msg);
  return ret;
}
}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
