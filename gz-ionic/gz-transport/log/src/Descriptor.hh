/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_LOG_SRC_DESCRIPTOR_HH_
#define GZ_TRANSPORT_LOG_SRC_DESCRIPTOR_HH_

#include <functional>
#include <string>
#include <unordered_map>

#include <gz/transport/log/Descriptor.hh>

namespace gz
{
  namespace transport
  {
    namespace log
    {
      // Inline bracket to help doxygen filtering.
      inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
      //
      /// \brief A representation of the information that defines a topic row
      /// \note We export the symbols for this class so it can be used in
      /// UNIT_Descriptor_TEST
      struct GZ_TRANSPORT_LOG_VISIBLE TopicKey
      {
#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
        /// \brief The name of the topic
        public: std::string topic;

        /// \brief The message type name published on the topic
        public: std::string type;
#ifdef _WIN32
#pragma warning(pop)
#endif

        /// \brief Equality operator. Needed for std::unordered_map
        /// \param[in] _other Another TopicKey
        /// \return true if equal
        inline bool operator==(const TopicKey &_other) const
        {
          return (this->topic == _other.topic &&
                  this->type == _other.type);
        }
      };

      /// \brief A map from the (topic, message type) of a topic to its integer
      /// key in the database.
      using TopicKeyMap = std::unordered_map<TopicKey, int64_t>;

      /// \brief Implementation of the Descriptor class
      /// \note We export the symbols for this class so it can be used in
      /// UNIT_Descriptor_TEST
      class GZ_TRANSPORT_LOG_VISIBLE Descriptor::Implementation
      {
        /// \internal Reset this descriptor. This should only be called by the
        /// Log class, when it is generating a new Descriptor after opening a
        /// new file.
        /// \param[in] _topics The map of topics that the log contains.
        public: void Reset(const TopicKeyMap &_topics);

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
        /// \internal \sa Descriptor::TopicsToMsgTypesToId()
        public: NameToMap topicsToMsgTypesToId;

        /// \internal \sa Descriptor::MsgTypesToTopicsToId()
        public: NameToMap msgTypesToTopicsToId;
#ifdef _WIN32
#pragma warning(pop)
#endif
      };
      }
    }
  }
}

//////////////////////////////////////////////////
/// \brief Allow a TopicKey to be used as a key in a std::unordered_map
namespace std {
  template <> struct hash<gz::transport::log::TopicKey>
  {
    size_t operator()(
        const gz::transport::log::TopicKey &_key) const
    {
      // Terrible, but it gets the job done
      return (std::hash<std::string>()(_key.topic) << 16)
        + std::hash<std::string>()(_key.type);
    }
  };
}

#endif
