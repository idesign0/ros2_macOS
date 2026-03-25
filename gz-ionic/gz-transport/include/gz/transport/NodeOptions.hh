/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_NODEOPTIONS_HH_
#define GZ_TRANSPORT_NODEOPTIONS_HH_

#include <memory>
#include <string>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  // Forward declarations.
  class NodeOptionsPrivate;

  /// \class NodeOptions NodeOptions.hh gz/transport/NodeOptions.hh
  /// \brief A class for customizing the behavior of the Node.
  /// E.g.: Set a custom namespace or a partition name.
  class GZ_TRANSPORT_VISIBLE NodeOptions
  {
    /// \brief Constructor.
    public: NodeOptions();

    /// \brief Copy constructor.
    /// \param[in] _other NodeOptions to copy.
    public: NodeOptions(const NodeOptions &_other);

    /// \brief Destructor.
    public: virtual ~NodeOptions();

    /// \brief Assignment operator.
    /// \param[in] _other The new NodeOptions.
    /// \return A reference to this instance.
    public: NodeOptions &operator=(const NodeOptions &_other);

    /// \brief Get the namespace used in this node.
    /// \return The namespace.
    /// \sa SetNameSpace.
    public: const std::string &NameSpace() const;

    /// \brief Set the node's namespace. A namespace is considered a prefix
    /// that might be potentially applied to some of the topic/services
    /// advertised in a node.
    /// E.g.: Node1 sets a namespace 'ns1' and advertises the topics
    /// 't1', 't2' and '/t3'. '/t3' is considered an absolute topic (starts
    /// with '/') and it won't be affected by a namespace. However, 't1' and
    /// 't2' will be advertised as '/ns1/t1' and '/ns1/t2'.
    /// A namespace is any alphanumeric string with a few exceptions.
    /// The symbol '/' is allowed as part of a namespace but just '/' is not
    /// allowed. The symbols '@', '~' and ' ' are not allowed as part of a
    /// namespace. Two or more consecutive slashes ('//') are not allowed.
    /// \param[in] _ns The namespace.
    /// \return True when operation succeed or false if the namespace was
    /// invalid.
    /// \sa NameSpace.
    public: bool SetNameSpace(const std::string &_ns);

    /// \brief Get the partition used in this node.
    /// \return The partition name.
    /// \sa SetPartition.
    public: const std::string &Partition() const;

    /// \brief Set the node's partition name. A partition is used to
    /// isolate a set of topics or services from other nodes that use the same
    /// names. E.g.: Node1 advertises topics '/foo' and Node2 advertises
    /// '/foo' too. If we don't use a partition, a node subscribed to '/foo'
    /// will receive the messages published from Node1 and Node2.
    /// Alternatively, we could specify 'p1' as a partition for Node1 and 'p2'
    /// as a partition for Node2. When we create the node for our subscriber,
    /// if we specify 'p1' as a partition name, we'll receive the messages
    /// published only by Node1. If we use 'p2', we'll only receive the
    /// messages published by Node2. If we don't set a partition name, we
    /// won't receive any messages from Node1 or Node2.
    /// A partition name is any alphanumeric string with a few exceptions.
    /// The symbol '/' is allowed as part of a partition name but just '/' is
    /// not allowed. The symbols '@', '~' and ' ' are not allowed as part of a
    /// partition name. Two or more consecutive slashes ('//') are not allowed
    /// \param[in] _partition The partition's name.
    /// The default partition value is created using a combination of your
    /// hostname, followed by ':' and your username. E.g.: "bb9:caguero" .
    /// It's also possible to use the environment variable GZ_PARTITION for
    /// setting a partition name.
    /// \return True when operation succeed or false if the partition name was
    /// invalid.
    /// \sa Partition
    public: bool SetPartition(const std::string &_partition);

    /// \brief Add a new topic remapping. Any [Un]Advertise(),
    /// [Un]Subscribe() or Request() operation will check for topic
    /// remappings. If a topic is remapped, the '_fromTopic' topic will be
    /// renamed to '_toTopic' in any of the previous functions.
    /// Is not possible to add two remaps over the same '_fromTopic'.
    /// \param[in] _fromTopic Original topic to be renamed.
    /// \param[in] _toTopic New topic to be used.
    /// \return True if the topic remap is possible or false otherwise.
    public: bool AddTopicRemap(const std::string &_fromTopic,
                               const std::string &_toTopic);

    /// \brief Get a topic remapping.
    /// \param[in] _fromTopic The original topic.
    /// \param[out] _toTopic The new topic name.
    /// _toTopic is unchanged if there is no remapping.
    /// \return True if a topic remap was found for '_fromTopic' or
    /// false otherwise.
    public: bool TopicRemap(const std::string &_fromTopic,
                            std::string &_toTopic) const;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Smart pointer to private data.
    protected: std::unique_ptr<transport::NodeOptionsPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}
#endif
