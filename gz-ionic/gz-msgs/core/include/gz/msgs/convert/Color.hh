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
#ifndef GZ_MSGS_CONVERT_COLOR_HH_
#define GZ_MSGS_CONVERT_COLOR_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/color.pb.h"

// Data Headers
#include <gz/math/Color.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////
inline void Set(gz::msgs::Color *_msg, const gz::math::Color &_data)
{
  _msg->set_r(_data.R());
  _msg->set_g(_data.G());
  _msg->set_b(_data.B());
  _msg->set_a(_data.A());
}

inline void Set(gz::math::Color *_data, const gz::msgs::Color &_msg)
{
  _data->Set(_msg.r(), _msg.g(), _msg.b(), _msg.a());
}

inline gz::msgs::Color Convert(const gz::math::Color &_data)
{
  gz::msgs::Color ret;
  Set(&ret, _data);
  return ret;
}

inline gz::math::Color Convert(const gz::msgs::Color &_msg)
{
  gz::math::Color ret;
  Set(&ret, _msg);
  return ret;
}
}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
