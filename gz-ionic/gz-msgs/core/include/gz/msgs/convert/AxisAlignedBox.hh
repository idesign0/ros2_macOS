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
#ifndef GZ_MSGS_CONVERT_AXISALIGNEDBOX_HH_
#define GZ_MSGS_CONVERT_AXISALIGNEDBOX_HH_

#include <gz/msgs/config.hh>
#include <gz/msgs/convert/Vector3.hh>

// Message Headers
#include "gz/msgs/axis_aligned_box.pb.h"

// Data Headers
#include <gz/math/AxisAlignedBox.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////
inline void Set(gz::msgs::AxisAlignedBox *_msg,
                const gz::math::AxisAlignedBox &_data)
{
  Set(_msg->mutable_min_corner(), _data.Min());
  Set(_msg->mutable_max_corner(), _data.Max());
}

inline void Set(gz::math::AxisAlignedBox *_data,
                const gz::msgs::AxisAlignedBox &_msg)
{
  Set(&_data->Min(), _msg.min_corner());
  Set(&_data->Max(), _msg.max_corner());
}

inline gz::msgs::AxisAlignedBox Convert(const gz::math::AxisAlignedBox &_data)
{
  gz::msgs::AxisAlignedBox ret;
  Set(&ret, _data);
  return ret;
}

inline gz::math::AxisAlignedBox Convert(const gz::msgs::AxisAlignedBox &_msg)
{
  gz::math::AxisAlignedBox ret;
  Set(&ret, _msg);
  return ret;
}
}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
