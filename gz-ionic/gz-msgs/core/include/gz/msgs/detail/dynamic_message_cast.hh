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

#ifndef GZ_MSGS_DETAIL_DYNAMIC_POINTER_CAST_HH_
#define GZ_MSGS_DETAIL_DYNAMIC_POINTER_CAST_HH_

#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4512 4127 4068 4244 4267 4251 4146)
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace gz::msgs::detail
{

/// Cast a base unique pointer to protobuf message type to child type
template<typename MsgT>
std::unique_ptr<MsgT>
dynamic_message_cast(std::unique_ptr<google::protobuf::Message> &&_baseMsg)
{
  auto converted = std::unique_ptr<MsgT>{dynamic_cast<MsgT*>(_baseMsg.get())};
  if (converted) {
    // transfer ownership to a new unique_ptr object by releasing from old one
    (void) _baseMsg.release();
  }
  return converted;
}

}  // namespace gz::msgs::detail

#endif  // GZ_MSGS_DETAIL_DYNAMIC_POINTER_CAST_HH_
