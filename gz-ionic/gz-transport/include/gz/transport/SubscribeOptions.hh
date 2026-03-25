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

#ifndef GZ_TRANSPORT_SUBSCRIBEOPTIONS_HH_
#define GZ_TRANSPORT_SUBSCRIBEOPTIONS_HH_

#include <cstdint>
#include <memory>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  class SubscribeOptionsPrivate;

  /// \class SubscribeOptions SubscribeOptions.hh
  /// gz/transport/SubscribeOptions.hh
  /// \brief A class to provide different options for a subscription.
  class GZ_TRANSPORT_VISIBLE SubscribeOptions
  {
    /// \brief Constructor.
    public: SubscribeOptions();

    /// \brief Copy constructor.
    /// \param[in] _otherSubscribeOpts SubscribeOptions to copy.
    public: SubscribeOptions(const SubscribeOptions &_otherSubscribeOpts);

    /// \brief Destructor.
    public: ~SubscribeOptions();

    /// \brief Whether the subscription has been throttled.
    /// \return true when the subscription is throttled or false otherwise.
    /// \sa SetMsgsPerSec
    /// \sa MsgsPerSec
    public: bool Throttled() const;

    /// \brief Set the maximum number of messages per second received per
    /// topic. Note that we calculate the minimum period of a message based
    /// on the msgs/sec rate. Any message received since the last subscription
    /// callback and the duration of the period will be discarded.
    /// \param[in] _newMsgsPerSec Maximum number of messages per second.
    public: void SetMsgsPerSec(const uint64_t _newMsgsPerSec);

    /// \brief Get the maximum number of messages per seconds received per
    /// topic.
    /// \return The maximum number of messages per second.
    public: uint64_t MsgsPerSec() const;

    /// \brief Set the value to ignore local messages or not.
    /// \param[in] _ignore True when ignoring local messages
    /// or false otherwise.
    /// \sa IgnoreLocalMessages
    public: void SetIgnoreLocalMessages(bool _ignore);

    /// \brief Whether the local messages should be ignored.
    /// \return true when the local messages should be ignored or
    /// false otherwise.
    /// \sa SetIgnoreLocalMessages
    public: bool IgnoreLocalMessages() const;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Pointer to private data.
    protected: std::unique_ptr<SubscribeOptionsPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}
#endif
