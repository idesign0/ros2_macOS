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

#ifndef GZ_TRANSPORT_NODESHARED_HH_
#define GZ_TRANSPORT_NODESHARED_HH_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <unordered_set>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"
#include "gz/transport/HandlerStorage.hh"
#include "gz/transport/NodeOptions.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/RepHandler.hh"
#include "gz/transport/ReqHandler.hh"
#include "gz/transport/SubscriptionHandler.hh"
#include "gz/transport/TopicStorage.hh"
#include "gz/transport/TopicStatistics.hh"
#include "gz/transport/TransportTypes.hh"
#include "gz/transport/Uuid.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  // Forward declarations.
  class Node;
  class NodePrivate;

  /// \brief Private data pointer
  class NodeSharedPrivate;

  /// \class NodeShared NodeShared.hh gz/transport/NodeShared.hh
  /// \brief Private data for the Node class. This class should not be
  /// directly used. You should use the Node class.
  class GZ_TRANSPORT_VISIBLE NodeShared
  {
    /// \brief NodeShared is a singleton. This method gets the
    /// NodeShared instance shared between all the nodes.
    /// \return Pointer to the current NodeShared instance.
    public: static NodeShared *Instance();

    /// \brief Receive data and control messages.
    public: void RunReceptionTask();

    /// \brief Publish data.
    /// \param[in] _topic Topic to be published.
    /// \param[in, out] _data Serialized data. Note that this buffer will be
    /// automatically deallocated by ZMQ when all data has been published.
    /// \param[in] _dataSize Data size (bytes).
    /// \param[in, out] _ffn Deallocation function. This function is
    /// executed by ZeroMQ when the data is published. This function
    /// deallocates the buffer containing the published data.
    /// \sa http://zeromq.org/blog:zero-copy
    /// \param[in] _msgType Message type in string format.
    /// \return true when success or false otherwise.
    public: bool Publish(const std::string &_topic,
                         char *_data,
                         const size_t _dataSize,
                         DeallocFunc *_ffn,
                         const std::string &_msgType);

    /// \brief Method in charge of receiving the topic updates.
    public: void RecvMsgUpdate();

    /// \brief HandlerInfo contains information about callback handlers which
    /// is useful for local publishers and message receivers. You should only
    /// retrieve a HandlerInfo by calling
    /// CheckHandlerInfo(const std::string &_topic) const
    public: struct HandlerInfo
    {
      /// \brief This is a map of the standard local callback handlers. The
      /// key is the topic name, and the value is another map whose key is
      /// the node UUID and whose value is a smart pointer to the handler.
      public: std::map<std::string, ISubscriptionHandler_M> localHandlers;

      /// \brief This is a map of the raw local callback handlers. The key is
      /// the topic name, and the value is another map whose key is the node
      /// UUID and whose value is a smart pointer to the handler.
      public: std::map<std::string, RawSubscriptionHandler_M> rawHandlers;

      /// \brief True iff there are any standard local subscribers.
      public: bool haveLocal;

      /// \brief True iff there are any raw local subscribers
      public: bool haveRaw;

      // Friendship. This allows HandlerInfo to be created by
      // CheckHandlerInfo()
      friend class NodeShared;

      // TODO(sloretz) private default constructor (visual studio 2017?)
    };

    /// \brief Get information about the local and raw subscribers that are
    /// attached to this NodeShared.
    /// \param[in] _topic Information will only be returned for handlers that
    /// are subscribed to the given topic name.
    /// \return Information about local subscription handlers that are held by
    /// this NodeShared.
    HandlerInfo CheckHandlerInfo(const std::string &_topic) const;

    /// \brief This struct provides information about the Subscribers of a
    /// Publisher. It should only be retrieved using
    /// CheckSubscriberInfo(const std::string&, const std::string&) const.
    /// The relevant subscriber info is a superset of the relevant HandlerInfo
    /// so we extend that struct.
    ///
    /// This struct is used internally by publishers to determine what kind of
    /// subscribers they have.
    public: struct SubscriberInfo : public HandlerInfo
    {
      /// \brief True if this Publisher has any remote subscribers
      // cppcheck-suppress unusedStructMember
      public: bool haveRemote;

      // Friendship declaration
      friend class NodeShared;

      // TODO(sloretz) private default constructor (visual studio 2017?)
    };

    /// \brief Get information about the nodes that are subscribed to the
    /// publishers of this NodeShared.
    /// \param[in] _topic Only information about subscribers to this topic
    /// will be returned.
    /// \param[in] _msgType If there are no remote subscribers listening for
    /// this message type, then SubscriberInfo::haveRemote will be false in
    /// the return value of this function.
    /// \return Information about subscribers.
    SubscriberInfo CheckSubscriberInfo(
        const std::string &_topic,
        const std::string &_msgType) const;

    /// \brief Call the SubscriptionHandler callbacks (local and raw) for this
    /// NodeShared.
    /// \param[in] _info Message information.
    /// \param[in] _msgData The raw serialized data for the message
    /// \param[in] _handlerInfo Information for the handlers of this node,
    /// as generated by CheckHandlerInfo(const std::string&) const
    public: void TriggerCallbacks(
      const MessageInfo &_info,
      const std::string &_msgData,
      const HandlerInfo &_handlerInfo);

    /// \brief Method in charge of receiving the service call requests.
    public: void RecvSrvRequest();

    /// \brief Method in charge of receiving the service call responses.
    public: void RecvSrvResponse();

    /// \brief Try to send all the requests for a given service call and a
    /// pair of request/response types.
    /// \param[in] _topic Topic name.
    /// \param[in] _reqType Type of the request in string format.
    /// \param[in] _repType Type of the response in string format.
    public: void SendPendingRemoteReqs(const std::string &_topic,
                                       const std::string &_reqType,
                                       const std::string &_repType);

    /// \brief Callback executed when the discovery detects new topics.
    /// \param[in] _pub Information of the publisher in charge of the topic.
    public: void OnNewConnection(const MessagePublisher &_pub);

    /// \brief Callback executed when the discovery detects disconnections.
    /// \param[in] _pub Information of the publisher in charge of the topic.
    public: void OnNewDisconnection(const MessagePublisher &_pub);

    /// \brief Callback executed when the discovery detects a new service call
    /// \param[in] _pub Information of the publisher in charge of the service.
    public: void OnNewSrvConnection(const ServicePublisher &_pub);

    /// \brief Callback executed when a service call is no longer available.
    /// \param[in] _pub Information of the publisher in charge of the service.
    public: void OnNewSrvDisconnection(const ServicePublisher &_pub);

    /// \brief Callback executed when a remote subscriber connects.
    /// \param[in] _pub Information of the remote subscriber.
    public: void OnNewRegistration(const MessagePublisher &_pub);

    /// \brief Callback executed when a remote subscriber unregisters.
    /// \param[in] _pub Information of the remote subscriber.
    public: void OnEndRegistration(const MessagePublisher &_pub);

    /// \brief Callback executed when a SUBSCRIBERS request is received.
    public: void OnSubscribers();

    /// \brief Pass through to bool Publishers(const std::string &_topic,
    /// Addresses_M<Pub> &_publishers) const
    /// \param[in] _topic Service name.
    /// \param[out] _publishers Collection of service publishers.
    /// \return True if the service is found and
    //  there is at least one publisher.
    /// \sa bool Publishers(const std::string &_topic,
    /// Addresses_M<Pub> &_publishers) const
    public: bool TopicPublishers(const std::string &_topic,
                                 SrvAddresses_M &_publishers) const;

    /// \brief Pass through to bool Discovery::Discover(const std::string
    /// &_topic) const
    /// \param[in] _topic Service name.
    /// \return True if the method succeeded or false otherwise
    /// (e.g. if the discovery has not been started).
    /// \sa bool Discovery::Discover(const std::string &_topic) const
    public: bool DiscoverService(const std::string &_topic) const;

    /// \brief Pass through to bool Advertise(const Pub &_publisher)
    /// \param[in] _publisher Publisher's information to advertise.
    /// \return True if the method succeed or false otherwise
    /// (e.g. if the discovery has not been started).
    /// \sa Pass through to bool Advertise(const Pub &_publisher)
    public: bool AdvertisePublisher(const ServicePublisher &_publisher);

    /// \brief Get the capacity of the buffer (High Water Mark)
    /// that stores incoming Gazebo Transport messages. Note that this is a
    /// global queue shared by all subscribers within the same process.
    /// \return The capacity of the buffer storing incoming messages (units
    /// are messages). A value of 0 indicates an unlimited buffer and -1
    /// that the socket cannot be queried. The default buffer size is
    /// contained in the #kDefaultRcvHwm variable.
    /// If the buffer is set to unlimited, then your buffer will grow until
    /// you run out of memory (and probably crash).
    /// If your buffer reaches the maximum capacity data will be dropped.
    public: int RcvHwm();

    /// \brief Get the capacity of the buffer (High Water Mark)
    /// that stores outgoing Gazebo Transport messages. Note that this is a
    /// global queue shared by all publishers within the same process.
    /// \return The capacity of the buffer storing outgoing messages (units
    /// are messages). A value of 0 indicates an unlimited buffer and -1
    /// that the socket cannot be queried. The default buffer size is
    /// contained in the #kDefaultSndHwm variable.
    /// If the buffer is set to unlimited, then your buffer will grow until
    /// you run out of memory (and probably crash).
    /// If your buffer reaches the maximum capacity data will be dropped.
    public: int SndHwm();

    /// \brief Turn topic statistics on or off.
    /// \param[in] _topic The name of the topic on which to enable or disable
    /// statistics.
    /// \param[in] _enable True to enable statistics, false to disable.
    /// \param[in] _cb Callback that is triggered whenever statistics are
    /// updated.
    public: void EnableStats(const std::string &_topic, bool _enable,
                std::function<void(const TopicStatistics &_stats)> _cb);

    /// \brief Get the current statistics for a topic. Statistics must
    /// have been enabled using the EnableStatistics function, otherwise
    /// the return value will be std::nullopt.
    /// \param[in] _topic The name of the topic to get statistics for.
    /// \return A TopicStatistics class, or std::nullopt if statistics were
    /// not enabled.
    public: std::optional<TopicStatistics> TopicStats(
                const std::string &_topic) const;

    /// \brief Adds a unicast relay IP. All nodes in this process will send
    /// UDP unicast traffic to the address to connect networks when UDP
    /// multicast traffic is not forwarded.
    /// It's also possible to use the environment variable GZ_RELAY to add
    /// relays.
    /// \param[in] _relayAddress IPv4 address of the relay to add.
    public: void AddGlobalRelay(const std::string& _relayAddress);

    /// \brief Gets the relay addresses configured for all nodes in this
    /// process.
    /// \return The relay addresses.
    public: std::vector<std::string> GlobalRelays() const;

    /// \brief Unsubscribe a node from a topic.
    /// If the handler UUID argument is empty, all subscription handlers in
    /// the node for the specified topic are removed
    /// \param[in] _topic Topic name to be unsubscribed.
    /// \param[in] _nUuid Node UUID.
    /// \param[in] _nOpt Node options.
    /// \param[in] _hUuid Hander UUID.
    /// \return True when successfully unsubscribed or false otherwise.
    public: bool Unsubscribe(const std::string &_topic,
                             const std::string &_nUuid,
                             const NodeOptions &_nOpt,
                             const std::string &_hUuid = "");

    /// \brief Get the set of topics subscribed by a node.
    /// \param[in] _nUuid Node UUID.
    /// \return The set of subscribed topics.
    private: std::unordered_set<std::string> &TopicsSubscribed(
             const std::string &_nUuid) const;

    /// \brief Remove a subscribed topic for a node
    /// \param[in] _topic Topic to remove.
    /// \param[in] _nUuid Node UUID.
    /// \return True if the topic is successfully removed, false otherwise.
    private: bool RemoveSubscribedTopic(const std::string &_topic,
                                        const std::string &_nUuid);

    /// \brief Helper function to remove handlers from the shared publish
    /// queue for a node. This is called when the node unsubscribes to a topic
    /// \param[in] _topic Topic that the node unsubscribed to.
    /// \param[in] _nUuid Node UUID.
    /// \return True on success.
    public: bool RemoveHandlersFromPubQueue(const std::string &_topic,
                                            const std::string &_nUuid);

    /// \brief Helper function to remove one handler from the shared publish
    /// queue for a node. This is called when the node unsubscribes to a
    /// topic for a single handler.
    /// \param[in] _topic Topic that the node unsubscribed to.
    /// \param[in] _nUuid Node UUID.
    /// \param[in] _hUuid Handler UUID.
    /// \return True on success.
    public: bool RemoveHandlerFromPubQueue(const std::string &_topic,
                                           const std::string &_nUuid,
                                           const std::string &_hUuid);

    /// \brief Helper function for Subscribe. This adds the fully qualified
    /// topic name to a map of node to topic names, which helps track
    /// a list of topics subscribed by a node.
    /// \param[in] _fullyQualifiedTopic Fully qualified topic name
    /// \param[in] _nUuid Node UUID.
    /// \return True on success.
    /// \sa TopicUtils::FullyQualifiedName
    public: bool SubscribeHelper(const std::string &_fullyQualifiedTopic,
                                 const std::string &_nUuid);

    /// \brief Constructor.
    protected: NodeShared();

    /// \brief Destructor.
    protected: virtual ~NodeShared();

    /// \brief Initialize all sockets.
    /// \return True when success or false otherwise. This function might
    /// return false if any operation on a ZMQ socket triggered an exception.
    private: bool InitializeSockets();

    //////////////////////////////////////////////////
    /////// Declare here other member variables //////
    //////////////////////////////////////////////////

    /// \brief Response receiver socket identity.
    public: Uuid responseReceiverId;

    /// \brief Replier socket identity.
    public: Uuid replierId;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Process UUID.
    public: std::string pUuid;

    /// \brief thread in charge of receiving and handling incoming messages.
    public: std::thread threadReception;

    /// \brief Mutex to guarantee exclusive access between all threads.
    public: mutable std::recursive_mutex mutex;

    /// \brief Default IP address used by the message discovery layer.
    public: std::string discoveryIP = "239.255.0.7";

    /// \brief Default port used by the message discovery layer.
    public: static const int kDefaultMsgDiscPort = 10317;

    /// \brief Default port used by the service discovery layer.
    public: static const int kDefaultSrvDiscPort = 10318;

    /// \brief Port used by the message discovery layer.
    public: int msgDiscPort = kDefaultMsgDiscPort;

    /// \brief Port used by the service discovery layer.
    public: int srvDiscPort = kDefaultSrvDiscPort;

    /// \brief Remote connections for pub/sub messages.
    private: TopicStorage<MessagePublisher> connections;

    /// \brief List of connected zmq end points for request/response.
    private: std::vector<std::string> srvConnections;

    /// \brief Remote subscribers.
    public: TopicStorage<MessagePublisher> remoteSubscribers;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief This struct wraps up the two different types of subscription
    /// handlers: normal (deserialized) and raw (serialized). This wrapper
    /// keeps the two sets of subscription handlers coordinated while allowing
    /// them to act independently when necessary.
    struct HandlerWrapper
    {
      /// \brief Returns true if this wrapper contains any subscriber that
      /// matches the given topic name and message type name.
      /// \param[in] _fullyQualifiedTopic Fully-qualified topic name
      /// \param[in] _msgType Name of message type
      /// \return True if this contains a matching subscriber, otherwise false
      /// \sa TopicUtils::FullyQualifiedName
      public: bool HasSubscriber(
          const std::string &_fullyQualifiedTopic,
          const std::string &_msgType) const;

      /// \brief Returns true if this wrapper contains any subscriber that
      /// matches the given fully-qualified topic name. The message type name
      /// of the subscriber is irrelevant.
      /// \param[in] _fullyQualifiedTopic Fully-qualified topic name
      /// \return True if this contains a matching subscriber, otherwise false
      public: bool HasSubscriber(
          const std::string &_fullyQualifiedTopic) const;

      /// \brief Returns true if this wrapper contains any subscriber that
      /// matches the given fully-qualified topic name and node UUID.
      /// The message type name of the subscriber is irrelevant.
      /// \param[in] _fullyQualifiedTopic Fully-qualified topic name
      /// \param[in] _nUuid Node UUID
      /// \return True if this contains a matching subscriber, otherwise false
      public: bool HasSubscriberForNode(
          const std::string &_fullyQualifiedTopic, const std::string &_nUuid)
          const;

      /// \brief Get a set of node UUIDs for subscribers in this wrapper that
      /// match the topic and message type criteria.
      /// \param[in] _fullyQualifiedTopic Fully-qualified topic name that the
      /// subscribers must be listening to.
      /// \param[in] _msgTypeName Name of the message type that the
      /// subscribers must be listening for.
      /// \return The node UUIDs of all subscribers that match the criteria
      public: std::vector<std::string> NodeUuids(
          const std::string &_fullyQualifiedTopic,
          const std::string &_msgTypeName) const;

      /// \brief Remove the handlers for the given topic name that belong to
      /// a specific node.
      /// \param[in] _fullyQualifiedTopic The fully-qualified name of the
      /// topic whose subscribers should be removed.
      /// \param[in] _nUuid The UUID of the node whose subscribers should be
      /// removed.
      /// \return True if at least one subscriber was removed.
      public: bool RemoveHandlersForNode(
          const std::string &_fullyQualifiedTopic,
          const std::string &_nUuid);

      /// \brief Remove one handler for the given topic name that belong to
      /// a specific node.
      /// \param[in] _fullyQualifiedTopic The fully-qualified name of the
      /// topic whose subscribers should be removed.
      /// \param[in] _nUuid The UUID of the node whose subscribers should be
      /// removed.
      /// \param[in] _hUuid The UUID of the handler to remove.
      /// \return True if the subscriber was removed.
      public: bool RemoveHandler(
          const std::string &_fullyQualifiedTopic,
          const std::string &_nUuid,
          const std::string &_hUuid);

      /// \brief Convert all the HandlerStorages into a vector of publishers.
      /// \param[in] _addr The pub/sub address.
      /// \param[in] _pUuid The process UUID.
      /// \return The vector of message publishers.
      public: std::vector<MessagePublisher> Convert(const std::string &_addr,
          const std::string &_pUuid);

      /// \brief Normal local subscriptions.
      public: HandlerStorage<ISubscriptionHandler> normal;

      /// \brief Raw local subscriptions. Keeping these separate from
      /// localSubscriptions allows us to avoid an unnecessary deserialization
      /// followed by an immediate reserialization.
      public: HandlerStorage<RawSubscriptionHandler> raw;
    };

    public: HandlerWrapper localSubscribers;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Service call repliers.
    public: HandlerStorage<IRepHandler> repliers;

    /// \brief Pending service call requests.
    public: HandlerStorage<IReqHandler> requests;

    /// \brief Print activity to stdout.
    public: int verbose;

    /// \brief My pub/sub address.
    public: std::string myAddress;

    /// \brief My pub/sub control address.
    public: std::string myControlAddress;

    /// \brief My requester service call address.
    public: std::string myRequesterAddress;

    /// \brief My replier service call address.
    public: std::string myReplierAddress;

    /// \brief IP address of this host.
    public: std::string hostAddr;

    /// \brief Internal data pointer.
    private: std::unique_ptr<NodeSharedPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
    private: friend Node;
    private: friend NodePrivate;
  };
  }
}
#endif
