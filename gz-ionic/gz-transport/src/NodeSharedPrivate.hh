/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_NODESHAREDPRIVATE_HH_
#define GZ_TRANSPORT_NODESHAREDPRIVATE_HH_

#include <zmq.hpp>

#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "gz/transport/Discovery.hh"
#include "gz/transport/Node.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief Metadata for a publication. This is sent as part of the ZMQ
  /// message for topic statistics.
  class PublicationMetadata
  {
    /// \brief Publication timestamp.
    public: uint64_t stamp = 0;

    /// \brief Sequence number, used to detect dropped messages.
    public: uint64_t seq = 0;
  };

  //
  // Private data class for NodeShared.
  class NodeSharedPrivate
  {
    // Constructor
    public: NodeSharedPrivate() :
              context(new zmq::context_t(1)),
              publisher(new zmq::socket_t(*context, ZMQ_PUB)),
              subscriber(new zmq::socket_t(*context, ZMQ_SUB)),
              requester(new zmq::socket_t(*context, ZMQ_ROUTER)),
              responseReceiver(new zmq::socket_t(*context, ZMQ_ROUTER)),
              replier(new zmq::socket_t(*context, ZMQ_ROUTER))
    {
    }

    /// \brief Initialize security
    public: void SecurityInit();

    /// \brief Handle new secure connections
    public: void SecurityOnNewConnection();

    /// \brief Access control handler for plain security.
    /// This function is designed to be run in a thread.
    public: void AccessControlHandler();

    /// \brief Get and validate a non-negative environment variable.
    /// \param[in] _envVar The name of the environment variable to get.
    /// \param[in] _defaultValue The default value returned in case the
    /// environment variable is invalid (e.g.: invalid number,
    /// negative number).
    /// \return The value read from the environment variable or the default
    /// value if the validation wasn't succeed.
    public: int NonNegativeEnvVar(const std::string &_envVar,
                                  int _defaultValue) const;

    //////////////////////////////////////////////////
    ///////    Declare here the ZMQ Context    ///////
    //////////////////////////////////////////////////

    /// \brief 0MQ context. Always declare this object before any ZMQ socket
    /// to make sure that the context is destroyed after all sockets.
    public: std::unique_ptr<zmq::context_t> context;

    //////////////////////////////////////////////////
    ///////     Declare here all ZMQ sockets   ///////
    //////////////////////////////////////////////////

    /// \brief ZMQ socket to send topic updates.
    public: std::unique_ptr<zmq::socket_t> publisher;

    /// \brief ZMQ socket to receive topic updates.
    public: std::unique_ptr<zmq::socket_t> subscriber;

    /// \brief ZMQ socket for sending service call requests.
    public: std::unique_ptr<zmq::socket_t> requester;

    /// \brief ZMQ socket for receiving service call responses.
    public: std::unique_ptr<zmq::socket_t> responseReceiver;

    /// \brief ZMQ socket to receive service call requests.
    public: std::unique_ptr<zmq::socket_t> replier;

    /// \brief Thread the handle access control
    public: std::thread accessControlThread;

    //////////////////////////////////////////////////
    /////// Declare here the discovery object  ///////
    //////////////////////////////////////////////////

    /// \brief Discovery service (messages).
    public: std::unique_ptr<MsgDiscovery> msgDiscovery;

    /// \brief Discovery service (services).
    public: std::unique_ptr<SrvDiscovery> srvDiscovery;

    //////////////////////////////////////////////////
    /////// Other private member variables     ///////
    //////////////////////////////////////////////////

    /// \brief When true, the reception thread will finish.
    public: std::atomic<bool> exit = false;

    /// \brief Timeout used for receiving messages (ms.).
    public: inline static const int Timeout = 250;

    ////////////////////////////////////////////////////////////////
    /////// The following is for asynchronous publication of ///////
    /////// messages to local subscribers.                    ///////
    ////////////////////////////////////////////////////////////////

    /// \brief Encapsulates information needed to publish a message. An
    /// instance of this class is pushed onto a publish queue, pubQueue, when
    /// a message is published through Node::Publisher::Publish.
    /// The pubThread processes the pubQueue in the
    /// NodeSharedPrivate::PublishThread function.
    ///
    /// A producer-consumer mechanism is used to send messages so that
    /// Node::Publisher::Publish function does not block while executing
    /// local subscriber callbacks.
    public: struct PublishMsgDetails
            {
              /// \brief All the local subscription handlers.
              public: std::vector<ISubscriptionHandlerPtr> localHandlers;

              /// \brief All the raw handlers.
              public: std::vector<RawSubscriptionHandlerPtr> rawHandlers;

              /// \brief Buffer for the raw handlers.
              public: std::unique_ptr<char[]> sharedBuffer = nullptr;

              /// \brief Msg copy for the local handlers.
              public: std::unique_ptr<ProtoMsg> msgCopy = nullptr;

              /// \brief Message size.
              // cppcheck-suppress unusedStructMember
              public: std::size_t msgSize = 0;

              /// \brief Information about the topic and type.
              public: MessageInfo info;

              /// \brief Publisher's node UUID.
              public: std::string publisherNodeUUID;
            };

    /// \brief Publish thread used to process the pubQueue.
    public: std::thread pubThread;

    /// \brief Mutex to protect the pubThread and pubQueue.
    public: std::mutex pubThreadMutex;

    /// \brief List onto which new messages are pushed. The pubThread
    /// will pop off the messages and send them to local subscribers.
    public: std::list<std::unique_ptr<PublishMsgDetails>> pubQueue;

    /// \brief used to signal when new work is available
    public: std::condition_variable signalNewPub;

    /// \brief Handles local publication of messages on the pubQueue.
    public: void PublishThread();

    /// \brief Topic publication sequence numbers.
    public: std::map<std::string, uint64_t> topicPubSeq;

    /// \brief True if topic statistics have been enabled.
    public: bool topicStatsEnabled = false;

    /// \brief Statistics for a topic. The key in the map is the topic
    /// name and the value contains the topic statistics.
    public: std::map<std::string, TopicStatistics> topicStats;

    /// \brief Set of topics that have statistics enabled.
    public: std::map<std::string,
            std::function<void(const TopicStatistics &_stats)>>
              enabledTopicStatistics;

    /// \brief A map of node UUID and its subscribed topics
    public: std::unordered_map<std::string, std::unordered_set<std::string>>
            topicsSubscribed;
  };
  }
}
#endif
