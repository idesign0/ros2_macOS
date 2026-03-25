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

#ifndef GZ_TRANSPORT_SUBSCRIBEOPTIONSPRIVATE_HH_
#define GZ_TRANSPORT_SUBSCRIBEOPTIONSPRIVATE_HH_

#include <cstdint>

#include "gz/transport/Helpers.hh"

namespace gz::transport
{
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE
  {
  /// \class SubscribeOptionsPrivate SubscribeOptionsPrivate.hh
  /// gz/transport/SubscribeOptionsPrivate.hh
  /// \brief Private data for the SubscribeOptions class.
  class SubscribeOptionsPrivate
  {
    /// \brief Constructor.
    public: SubscribeOptionsPrivate() = default;

    /// \brief Destructor.
    public: virtual ~SubscribeOptionsPrivate() = default;

    /// \brief Default message subscription rate.
    public: uint64_t msgsPerSec = kUnthrottled;

    /// \brief Whether local messages should be ignored or not.
    public: bool ignoreLocalMessages = false;
  };
  }  // GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
#endif  // GZ_TRANSPORT_SUBSCRIBEOPTIONSPRIVATE_HH_
