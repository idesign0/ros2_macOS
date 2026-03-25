/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_TOPICUTILS_HH_
#define GZ_TRANSPORT_TOPICUTILS_HH_

#include <cstdint>
#include <string>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \class TopicUtils TopicUtils.hh gz/transport/TopicUtils.hh
  /// \brief This class provides different utilities related with topics.
  class GZ_TRANSPORT_VISIBLE TopicUtils
  {
    /// \brief Determines if a namespace is valid. A namespace's length must
    /// not exceed kMaxNameLength.
    /// The following symbols are not allowed as part of the
    /// namespace:  '@', ':=', '~'.
    /// \param[in] _ns Namespace to be checked.
    /// \return true if the namespace is valid.
    public: static bool IsValidNamespace(const std::string &_ns);

    /// \brief Determines if a partition is valid.
    /// The same rules to validate a topic name applies to a partition with
    /// the addition of the empty string, which is a valid partition (meaning
    /// no partition is used). A partition name's length must not exceed
    /// kMaxNameLength.
    /// \param[in] _partition Partition to be checked.
    /// \return true if the partition is valid.
    public: static bool IsValidPartition(const std::string &_partition);

    /// \brief Determines if a topic name is valid. A topic name is any
    /// non-empty alphanumeric string. The symbol '/' is also allowed as part
    /// of a topic name.
    /// The following symbols are not allowed as part of the
    /// topic name:  '@', ':=', '~'.
    /// A topic name's length must not exceed kMaxNameLength.
    /// Examples of valid topics: abc, /abc, /abc/de, /abc/de/
    /// \param[in] _topic Topic name to be checked.
    /// \return true if the topic name is valid.
    public: static bool IsValidTopic(const std::string &_topic);

    /// \brief Get the full topic path given a namespace and a topic name.
    /// A fully qualified topic name's length must not exceed kMaxNameLength.
    /// The fully qualified name follows the next syntax:
    /// \@\<PARTITION\>\@\<NAMESPACE\>/\<TOPIC\>
    /// where:
    /// \<PARTITION\>: The name of the partition or empty string.
    ///              A "/" will be prefixed to the partition name unless is
    ///              empty or it already starts with slash. A trailing slash
    ///              will always be removed.
    /// \<NAMESPACE\>: The namespace or empty string. A namespace is a prefix
    ///              applied to the topic name. If not empty, it will always
    ///              start with a "/". A trailing slash will always be
    ///              removed
    /// \<TOPIC\>: The topic name. A trailing slash will always be removed.
    ///
    /// Note: Intuitively, you can imagine the fully qualified name as a
    /// UNIX absolute path, where the partition is always surrounded by "@".
    /// A namespace, if present, corresponds with the directories of the
    /// path, and you can imagine the topic as the filename.
    ///
    /// E.g.:
    ///   Only topic:                @@/topic
    ///   No partition:              @@/namespace/topic1
    ///   No namespace:              @/partition@/topic1
    ///   Partition+namespace+topic: @/my_partition@/name/space/topic
    ///
    /// \param[in] _partition Partition name.
    /// \param[in] _ns Namespace.
    /// \param[in] _topic Topic name.
    /// \param[out] _name Fully qualified topic name.
    /// \return True if the fully qualified name is valid
    /// (if partition, namespace and topic are correct).
    /// \sa IsValidPartition
    /// \sa IsValidNamespace
    /// \sa IsValidTopic
    /// \sa DecomposeFullyQualifiedTopic
    public: static bool FullyQualifiedName(const std::string &_partition,
                                           const std::string &_ns,
                                           const std::string &_topic,
                                           std::string &_name);

    /// \brief Decompose a fully qualified topic name into its partition and
    /// topic strings. Note that if the topic is preceded by a namespace, then
    /// the namespace and topic name will remain together as one string.
    ///
    /// Given a fully qualified topic name with the following syntax:
    ///
    /// \@\<PARTITION\>\@\<NAMESPACE\>/\<TOPIC\>
    ///
    /// The _partition output argument will be set to \<PARTITION\>, and the
    /// _namespaceAndTopic output argument will be set to
    /// \<NAMESPACE\>/\<TOPIC\>.
    ///
    /// \param[in] _fullyQualifiedName The fully qualified topic name.
    /// \param[out] _partition The partition component of the fully qualified
    /// topic name.
    /// \param[out] _namespaceAndTopic The namespace and topic name component.
    /// Note that there is no way to distinguish between where a namespace
    /// ends and a topic name begins, since topic names may contain slashes.
    /// \return True if the topic and partition were set.
    /// \sa FullyQualifiedName
    public: static bool DecomposeFullyQualifiedTopic(
      const std::string &_fullyQualifiedName,
      std::string &_partition,
      std::string &_namespaceAndTopic);

    /// \brief Convert a topic name to a valid topic. The input topic is
    /// modified by:
    /// * turning white space into `_`.
    /// * removing special characters and combinations.
    /// \param[in] _topic Input topic, which may be invalid.
    /// \return A valid topic, or empty string if not possible to convert.
    public: static std::string AsValidTopic(const std::string &_topic);

    /// \brief The kMaxNameLength specifies the maximum number of characters
    /// allowed in a namespace, a partition name, a topic name, and a fully
    /// qualified topic name.
    public: static const uint16_t kMaxNameLength = 65535;
  };
  }
}

#endif
