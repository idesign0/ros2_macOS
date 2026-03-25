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

#ifndef GZ_TRANSPORT_MESSAGEINFO_HH_
#define GZ_TRANSPORT_MESSAGEINFO_HH_

#include <memory>
#include <string>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  class MessageInfoPrivate;

  /// \brief A class that provides information about the message received.
  class GZ_TRANSPORT_VISIBLE MessageInfo
  {
    /// \brief Default constructor.
    public: MessageInfo();

    /// \brief Explicit copy constructor (The copy constructor is deleted by
    /// default due to the use of std::unique_ptr member).
    /// \param[in] _other an instance to copy data from
    public: MessageInfo(const MessageInfo &_other);

    /// \brief Move constructor
    /// \param[in] _other an instance data is moved from
    public: MessageInfo(MessageInfo &&_other);  // NOLINT

    /// \brief Destructor.
    public: ~MessageInfo();

    /// \brief Get the topic name associated to the message.
    /// \return The topic name.
    public: const std::string &Topic() const;

    /// \brief Set the topic name associated to the message.
    public: void SetTopic(const std::string &_topic);

    /// \brief Get the name of the message type.
    /// \return The message type name.
    public: const std::string &Type() const;

    /// \brief Set the name of the message type.
    /// \param[in] _type the type name to set.
    public: void SetType(const std::string &_type);

    /// \brief Get the name of the partition.
    /// \return The partition name.
    public: const std::string &Partition() const;

    /// \brief Set the partition of the topic the message was on.
    /// \param[in] _partition of the topic.
    public: void SetPartition(const std::string &_partition);

    /// \brief Set both the topic and the partition from a single string.
    /// \param[in] _fullyQualifiedName The topic string with the partition
    /// information included.
    /// \return true if the topic and partition were set
    /// \sa TopicUtils::FullyQualifiedName
    public: bool SetTopicAndPartition(const std::string &_fullyQualifiedName);

    /// \brief Whether the message is coming from a node within this process.
    /// \return True when intra-process, false otherwise.
    public: bool IntraProcess() const;

    /// \brief Set whether this message is intra-process or not.
    /// \param[in] _value The intra-process value.
    public: void SetIntraProcess(bool _value);

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Pointer to private data.
    protected: std::unique_ptr<MessageInfoPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}
#endif
