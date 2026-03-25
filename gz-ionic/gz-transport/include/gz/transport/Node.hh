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
#ifndef GZ_TRANSPORT_NODE_HH_
#define GZ_TRANSPORT_NODE_HH_

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"
#include "gz/transport/NodeOptions.hh"
#include "gz/transport/NodeShared.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/RepHandler.hh"
#include "gz/transport/ReqHandler.hh"
#include "gz/transport/SubscribeOptions.hh"
#include "gz/transport/SubscriptionHandler.hh"
#include "gz/transport/TopicStatistics.hh"
#include "gz/transport/TopicUtils.hh"
#include "gz/transport/TransportTypes.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  // Forward declarations.
  class NodePrivate;

  /// \brief Get the capacity of the buffer (High Water Mark)
  /// that stores incoming Gazebo Transport messages. Note that this is a
  /// global queue shared by all subscribers within the same process.
  /// \return The capacity of the buffer storing incoming messages (units are
  /// messages). A value of 0 indicates an unlimited buffer and -1
  /// that the socket cannot be queried. The default buffer size is
  /// contained in the #kDefaultRcvHwm variable.
  /// If the buffer is set to unlimited, then your buffer will grow until
  /// you run out of memory (and probably crash).
  /// If your buffer reaches the maximum capacity data will be dropped.
  int GZ_TRANSPORT_VISIBLE rcvHwm();

  /// \brief Get the capacity of the buffer (High Water Mark)
  /// that stores outgoing Gazebo Transport messages. Note that this is a
  /// global queue shared by all publishers within the same process.
  /// \return The capacity of the buffer storing outgoing messages (units are
  /// messages). A value of 0 indicates an unlimited buffer and -1
  /// that the socket cannot be queried. The default buffer size is
  /// contained in the #kDefaultSndHwm variable.
  /// If the buffer is set to unlimited, then your buffer will grow until
  /// you run out of memory (and probably crash).
  /// If your buffer reaches the maximum capacity data will be dropped.
  int GZ_TRANSPORT_VISIBLE sndHwm();

  /// \brief Block the current thread until a SIGINT or SIGTERM is received.
  /// Note that this function registers a signal handler. Do not use this
  /// function if you want to manage yourself SIGINT/SIGTERM.
  void GZ_TRANSPORT_VISIBLE waitForShutdown();

  /// \class Node Node.hh gz/transport/Node.hh
  /// \brief A class that allows a client to communicate with other peers.
  /// There are two main communication modes: pub/sub messages and service
  /// calls.
  class GZ_TRANSPORT_VISIBLE Node
  {
    class PublisherPrivate;
    class SubscriberPrivate;

    /// \brief A class that is used to store information about an
    /// advertised publisher. An instance of this class is returned
    /// from Node::Advertise, and should be used in subsequent
    /// Node::Publisher::Publish calls.
    ///
    /// ## Pseudo code example ##
    ///
    ///    auto pub = myNode.Advertise<MsgType>("topic_name");
    ///    if (pub)
    ///    {
    ///      MsgType msg;
    ///
    ///      // Note that this version of Publish will copy the message
    ///      // when publishing to interprocess subscribers.
    ///      pub.Publish(msg);
    ///    }
    public: class GZ_TRANSPORT_VISIBLE Publisher
    {
      /// \brief Default constructor.
      public: Publisher();

      /// \brief Constructor.
      /// \param[in] _publisher A message publisher.
      public: explicit Publisher(const MessagePublisher &_publisher);

      /// \brief Destructor.
      public: virtual ~Publisher();

      /// \brief Allows this class to be evaluated as a boolean.
      /// \return True if valid
      /// \sa Valid
      public: operator bool();

      /// \brief Allows this class to be evaluated as a boolean (const).
      /// \return True if valid
      /// \sa Valid
      public: operator bool() const;

      /// \brief Return true if valid information, such as a non-empty
      /// topic name, is present.
      /// \return True if this object can be used in Publish() calls.
      public: bool Valid() const;

      /// \brief Publish a message. This function will copy the message
      /// when publishing to interprocess subscribers. This copy is
      /// necessary to facilitate asynchronous publication.
      /// \param[in] _msg A google::protobuf message.
      /// \return true when success.
      public: bool Publish(const ProtoMsg &_msg);

      /// \brief Publish a raw pre-serialized message.
      ///
      /// \warning This function is only intended for advanced users. The
      /// standard publishing function, Publish(const ProtoMsg &_msg), will
      /// ensure that your message is correctly serialized. It is strongly
      /// recommended that you use the standard publishing function unless
      /// there is a specific reason for using this one (e.g. you are
      /// forwarding or playing back data instead of serializing/deserializing
      /// it). We currently only support the serialization scheme of protobuf.
      ///
      /// \warning This function will copy the message data when
      /// publishing to remote subscribers (interprocess communication).
      ///
      /// \note This function will deserialize the message when sending it to
      /// local (intraprocess) subscribers.
      ///
      /// \param[in] _msgData A std::string that represents a
      /// serialized google::protobuf message.
      /// \param[in] _msgType A std::string that contains the message type
      /// name.
      /// \return true when success.
      public: bool PublishRaw(
        const std::string &_msgData,
        const std::string &_msgType);

      /// \brief Check if message publication is throttled. If so, verify
      /// whether the next message should be published or not.
      ///
      /// This may be used to skip resource or time-intensive operations
      /// in the case that the message won't be published.
      ///
      /// \return true if the message should be published or false otherwise.
      /// Additionally always returns true if the topic is not throttled.
      public: bool ThrottledUpdateReady() const;

      /// \brief Check if message publication is throttled. If so, verify
      /// whether the next message should be published or not.
      /// \return true if the message should be published or false otherwise.
      private: bool UpdateThrottling();

      /// \brief Return true if this publisher has subscribers.
      /// \return True if subscribers have connected to this publisher.
      public: bool HasConnections() const;

      /// \internal
      /// \brief Smart pointer to private data.
      /// This is std::shared_ptr because we want to trigger the destructor
      /// only once: when all references to PublisherPrivate are out of scope.
      /// The destructor of PublisherPrivate unadvertise the topic.
#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::shared_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
      private: std::shared_ptr<PublisherPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };

    /// \brief A class that is used to store information about an
    /// subscriber. An instance of this class is returned
    /// from Node::CreateSubscribe. When the object is destroyed,
    /// the corresponding subscription handler is removed from the node.
    ///
    /// ## Pseudo code example ##
    ///
    ///    std::function<void(const msgs::Int32 &)> cb =
    ///      [](const msgs::Int32 &)
    ///    {
    ///      // Do something
    ///    };
    ///    Node::Subscriber sub = myNode.CreateSubscriber("topic_name", cb);
    public: class GZ_TRANSPORT_VISIBLE Subscriber
    {
      /// \brief Default constructor.
      public: Subscriber();

      /// \brief Constructor
      /// \param[in] _topic Subscribed topic name
      /// \param[in] _nUuid Node to which this subscriber belongs
      /// \param[in] _nOpts Node options for the node
      /// \param[in] _hUuid Subscriber's handler UUID
      public: Subscriber(const std::string &_topic,
                         const std::string &_nUuid,
                         const NodeOptions &_nOpts,
                         const std::string &_hUuid);

      /// \brief Destructor.
      /// Unsubscribe to the topic and remove the subcription handler
      public: virtual ~Subscriber();

      /// \brief Unsubscribe from the topic.
      /// \return True if the topic was successfully unsubscribed
      public: bool Unsubscribe();

      /// \brief Allows this class to be evaluated as a boolean.
      /// \return True if valid
      /// \sa Valid
      public: operator bool();

      /// \brief Allows this class to be evaluated as a boolean (const).
      /// \return True if valid
      /// \sa Valid
      public: operator bool() const;

      /// \brief Move constructor
      /// \param[in] _other The other Node::Subscriber
      public: Subscriber(Subscriber &&_other);

      /// \brief Move assignment operator
      /// \param[in] _other The other Node::Subscriber
      /// \return Reference to this
      public: Node::Subscriber &operator=(Subscriber &&_other);

      /// \brief Return true if valid information, such as a non-empty
      /// topic name, node and handler UUIDs.
      /// \return True if this object has a valid subscription.
      public: bool Valid() const;

      /// \internal
      /// \brief Smart pointer to private data.
#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::shared_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
      private: std::shared_ptr<SubscriberPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };

    public: Node();

    /// \brief Constructor.
    /// \param[in] _options Node options.
    public: explicit Node(const NodeOptions &_options);

    /// \brief Destructor.
    public: virtual ~Node();

    /// \brief Advertise a new topic. If a topic is currently advertised,
    /// you cannot advertise it a second time (regardless of its type).
    /// \param[in] _topic Topic name to be advertised.
    /// \param[in] _options Advertise options.
    /// \return A PublisherId, which can be used in Node::Publish calls.
    /// The PublisherId also acts as boolean, where true occurs if the topic
    /// was successfully advertised.
    /// \sa AdvertiseOptions.
    public: template<typename MessageT>
    Node::Publisher Advertise(
        const std::string &_topic,
        const AdvertiseMessageOptions &_options = AdvertiseMessageOptions());

    /// \brief Advertise a new topic. If a topic is currently advertised,
    /// you cannot advertise it a second time (regardless of its type).
    /// \param[in] _topic Topic name to be advertised.
    /// \param[in] _msgTypeName Name of the message type that will be
    /// published on the topic. The message type name can be retrieved
    /// from a protobuf message using the GetTypeName() function.
    /// \param[in] _options Advertise options.
    /// \return A PublisherId, which can be used in Node::Publish calls.
    /// The PublisherId also acts as boolean, where true occurs if the topic
    /// was successfully advertised.
    /// \sa AdvertiseOptions.
    public: Node::Publisher Advertise(
        const std::string &_topic,
        const std::string &_msgTypeName,
        const AdvertiseMessageOptions &_options = AdvertiseMessageOptions());

    /// \brief Get the list of topics advertised by this node.
    /// \return A vector containing all the topics advertised by this node.
    public: std::vector<std::string> AdvertisedTopics() const;

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback does not include any message information.
    /// In this version the callback is a free function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    public: template<typename MessageT>
    bool Subscribe(
        const std::string &_topic,
        void(*_callback)(const MessageT &_msg),
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback does not include any message information.
    /// In this version the callback is a lambda function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Lambda function with the following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    public: template<typename MessageT>
    bool Subscribe(
        const std::string &_topic,
        std::function<void(const MessageT &_msg)> _callback,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback does not include any message information.
    /// In this version the callback is a member function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    public: template<typename ClassT, typename MessageT>
    bool Subscribe(
        const std::string &_topic,
        void(ClassT::*_callback)(const MessageT &_msg),
        ClassT *_obj,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback includes message information.
    /// In this version the callback is a free function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    public: template<typename MessageT>
    bool Subscribe(
        const std::string &_topic,
        void(*_callback)(const MessageT &_msg, const MessageInfo &_info),
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback includes message information.
    /// In this version the callback is a lambda function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Lambda function with the following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    public: template<typename MessageT>
    bool Subscribe(
        const std::string &_topic,
        std::function<void(const MessageT &_msg,
                           const MessageInfo &_info)> _callback,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback includes message information.
    /// In this version the callback is a member function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    public: template<typename ClassT, typename MessageT>
    bool Subscribe(
        const std::string &_topic,
        void(ClassT::*_callback)(const MessageT &_msg,
                                 const MessageInfo &_info),
        ClassT *_obj,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Create a subscriber to a topic registering a callback
    /// This is function is overloaded with different variants of callback
    /// functions. It returns a Node::Subscriber object that maintains the
    /// subscription while the object is alive. When the subscriber object
    /// goes out of scope, it automatically unsubscribes to the topic,
    /// removing just one single subscription handler from the node that it
    /// belongs to.
    /// Supported function callbacks are:
    ///   * free function
    ///   * member function
    ///   * lambda function
    /// The callback function can contain one (_msg) or both (_msg, _info) of
    /// the following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] args Arguments to be forwarded to SubscribeImpl
    /// \return true when successfully subscribed or false otherwise.
    /// \sa SubscribeImpl
    public: template <typename ...Args>
    Node::Subscriber CreateSubscriber(const std::string &_topic,
                                      Args && ...args);

    /// \brief Get the list of topics subscribed by this node. Note that
    /// we might be interested in one topic but we still don't know the
    /// address of a publisher.
    /// \return A vector containing the subscribed topics (even if we do not
    /// have an address for a particular topic yet).
    public: std::vector<std::string> SubscribedTopics() const;

    /// \brief Unsubscribe from a topic.
    /// \param[in] _topic Topic name to be unsubscribed.
    /// \return true when successfully unsubscribed or false otherwise.
    public: bool Unsubscribe(const std::string &_topic);

    /// \brief Advertise a new service.
    /// In this version the callback is a plain function pointer.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _request Protobuf message containing the request.
    ///   * _reply Protobuf message containing the response.
    ///   * Returns Service call result.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename RequestT, typename ReplyT>
    bool Advertise(
        const std::string &_topic,
        bool(*_callback)(const RequestT &_request, ReplyT &_reply),
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service without input parameter.
    /// In this version the callback is a free function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * Returns Service call result.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename ReplyT>
    bool Advertise(
        const std::string &_topic,
        bool(*_callback)(ReplyT &_reply),
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service without any output parameter.
    /// In this version the callback is a free function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _request Protobuf message containing the request.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename RequestT>
    bool Advertise(
        const std::string &_topic,
        void(*_callback)(const RequestT &_request),
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service.
    /// In this version the callback is a lambda function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _request Protobuf message containing the request.
    ///   * _reply Protobuf message containing the response.
    ///   * Returns Service call result.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename RequestT, typename ReplyT>
    bool Advertise(
        const std::string &_topic,
        std::function<bool(const RequestT &_request,
                           ReplyT &_reply)> _callback,
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service without input parameter.
    /// In this version the callback is a lambda function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * Returns Service call result.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename ReplyT>
    bool Advertise(
        const std::string &_topic,
        std::function<bool(ReplyT &_reply)> &_callback,
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service without any output parameter.
    /// In this version the callback is a lambda function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _request Protobuf message containing the request.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename RequestT>
    bool Advertise(
        const std::string &_topic,
        std::function<void(const RequestT &_request)> &_callback,
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service.
    /// In this version the callback is a member function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _request Protobuf message containing the request.
    ///   * _reply Protobuf message containing the response.
    ///   * Returns Service call result.
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename ClassT, typename RequestT, typename ReplyT>
    bool Advertise(
        const std::string &_topic,
        bool(ClassT::*_callback)(const RequestT &_request, ReplyT &_reply),
        ClassT *_obj,
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service without input parameter.
    /// In this version the callback is a member function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * Returns Service call result.
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions.
    public: template<typename ClassT, typename ReplyT>
    bool Advertise(
        const std::string &_topic,
        bool(ClassT::*_callback)(ReplyT &_reply),
        ClassT *_obj,
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Advertise a new service without any output parameter.
    /// In this version the callback is a member function.
    ///
    /// Multiple nodes can advertise the same service. When a request is
    /// made, only one of the available service providers will handle it.
    /// There is no guarantee of which provider will be selected. If one
    /// provider becomes unavailable, the remaining providers can still
    /// handle subsequent requests.
    /// \param[in] _topic Topic name associated to the service.
    /// \param[in] _callback Callback to handle the service request with the
    /// following parameters:
    ///   * _request Protobuf message containing the request.
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _options Advertise options.
    /// \return true when the topic has been successfully advertised or
    /// false otherwise.
    /// \sa AdvertiseOptions
    public: template<typename ClassT, typename RequestT>
    bool Advertise(
        const std::string &_topic,
        void(ClassT::*_callback)(const RequestT &_request),
        ClassT *_obj,
        const AdvertiseServiceOptions &_options = AdvertiseServiceOptions());

    /// \brief Get the list of services advertised by this node.
    /// \return A vector containing all services advertised by this node.
    public: std::vector<std::string> AdvertisedServices() const;

    /// \brief Request a new service using a non-blocking call.
    /// In this version the callback is a free function.
    /// \param[in] _topic Service name requested.
    /// \param[in] _request Protobuf message containing the request's
    /// parameters.
    /// \param[in] _callback Pointer to the callback function executed when
    /// the response arrives. The callback has the following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * _result Result of the service call. If false, there was
    ///   a problem executing your request.
    /// \return true when the service call was successfully requested.
    public: template<typename RequestT, typename ReplyT>
    bool Request(
        const std::string &_topic,
        const RequestT &_request,
        void(*_callback)(const ReplyT &_reply, const bool _result));

    /// \brief Request a new service without input parameter using a
    /// non-blocking call.
    /// In this version the callback is a free function.
    /// \param[in] _topic Service name requested.
    /// \param[in] _callback Pointer to the callback function executed when
    /// the response arrives. The callback has the following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * _result Result of the service call. If false, there was
    ///   a problem executing your request.
    /// \return true when the service call was successfully requested.
    public: template<typename ReplyT>
    bool Request(
        const std::string &_topic,
        void(*_callback)(const ReplyT &_reply, const bool _result));

    /// \brief Request a new service using a non-blocking call.
    /// In this version the callback is a lambda function.
    /// \param[in] _topic Service name requested.
    /// \param[in] _request Protobuf message containing the request's
    /// parameters.
    /// \param[in] _callback Lambda function executed when the response
    /// arrives. The callback has the following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * _result Result of the service call. If false, there was
    ///   a problem executing your request.
    /// \return true when the service call was successfully requested.
    public: template<typename RequestT, typename ReplyT>
    bool Request(
        const std::string &_topic,
        const RequestT &_request,
        std::function<void(const ReplyT &_reply,
                           const bool _result)> &_callback);

    /// \brief Request a new service without input parameter using a
    /// non-blocking call.
    /// In this version the callback is a lambda function.
    /// \param[in] _topic Service name requested.
    /// \param[in] _callback Lambda function executed when the response
    /// arrives. The callback has the following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * _result Result of the service call. If false, there was
    ///   a problem executing your request.
    /// \return true when the service call was successfully requested.
    public: template<typename ReplyT>
    bool Request(
        const std::string &_topic,
        std::function<void(const ReplyT &_reply,
                           const bool _result)> &_callback);

    /// \brief Request a new service using a non-blocking call.
    /// In this version the callback is a member function.
    /// \param[in] _topic Service name requested.
    /// \param[in] _request Protobuf message containing the request's
    /// parameters.
    /// \param[in] _callback Pointer to the callback function executed when
    /// the response arrives. The callback has the following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * _result Result of the service call. If false, there was
    ///   a problem executing your request.
    /// \param[in] _obj Instance containing the member function.
    /// \return true when the service call was successfully requested.
    public: template<typename ClassT, typename RequestT, typename ReplyT>
    bool Request(
        const std::string &_topic,
        const RequestT &_request,
        void(ClassT::*_callback)(const ReplyT &_reply, const bool _result),
        ClassT *_obj);

    /// \brief Request a new service without input parameter using a
    /// non-blocking call.
    /// In this version the callback is a member function.
    /// \param[in] _topic Service name requested.
    /// \param[in] _callback Pointer to the callback function executed when
    /// the response arrives. The callback has the following parameters:
    ///   * _reply Protobuf message containing the response.
    ///   * _result Result of the service call. If false, there was
    ///   a problem executing your request.
    /// \param[in] _obj Instance containing the member function.
    /// \return true when the service call was successfully requested.
    public: template<typename ClassT, typename ReplyT>
    bool Request(
        const std::string &_topic,
        void(ClassT::*_callback)(const ReplyT &_reply, const bool _result),
        ClassT *_obj);

    /// \brief Request a new service using a blocking call.
    /// \param[in] _topic Service name requested.
    /// \param[in] _request Protobuf message containing the request's
    /// parameters.
    /// \param[in] _timeout The request will timeout after '_timeout' ms.
    /// \param[out] _reply Protobuf message containing the response.
    /// \param[out] _result Result of the service call.
    /// \return true when the request was executed or false if the timeout
    /// expired.
    public: template<typename RequestT, typename ReplyT>
    bool Request(
        const std::string &_topic,
        const RequestT &_request,
        const unsigned int &_timeout,
        ReplyT &_reply,
        bool &_result);

    /// \brief Request a new service without input parameter using a blocking
    /// call.
    /// \param[in] _topic Service name requested.
    /// \param[in] _timeout The request will timeout after '_timeout' ms.
    /// \param[out] _reply Protobuf message containing the response.
    /// \param[out] _result Result of the service call.
    /// \return true when the request was executed or false if the timeout
    /// expired.
    public: template<typename ReplyT>
    bool Request(
        const std::string &_topic,
        const unsigned int &_timeout,
        ReplyT &_reply,
        bool &_result);

    /// \brief Request a new service without waiting for response.
    /// \param[in] _topic Topic requested.
    /// \param[in] _request Protobuf message containing the request's
    /// parameters.
    /// \return true when the service call was successfully requested.
    public: template<typename RequestT>
    bool Request(const std::string &_topic, const RequestT &_request);

    /// \brief Request a new service using a blocking call. This request
    /// function expects a serialized protobuf message as the request and
    /// returns a serialized protobuf message as the response.
    /// \param[in] _topic Service name requested.
    /// \param[in] _request Protobuf message serialized into a string
    /// containing the request's parameters.
    /// \param[in] _requestType Message type of the request.
    /// \param[in] _responseType Message type of the response.
    /// \param[in] _timeout The request will timeout after '_timeout' ms.
    /// \param[out] _response Serialized protobuf message containing the
    ///  response.
    /// \param[out] _result Result of the service call.
    /// \return true when the request was executed or false if the timeout
    /// expired.
    public: bool RequestRaw(const std::string &_topic,
                const std::string &_request, const std::string &_requestType,
                const std::string &_responseType, unsigned int _timeout,
                std::string &_response,
                bool &_result);

    /// \brief Unadvertise a service.
    /// \param[in] _topic Service name to be unadvertised.
    /// \return true if the service was successfully unadvertised.
    public: bool UnadvertiseSrv(const std::string &_topic);

    /// \brief Get the list of topics currently advertised in the network.
    /// Note that this function can block for some time if the
    /// discovery is in its initialization phase.
    /// The value of the "heartbeatInterval" constant, with a default
    /// value of 1000 ms, sets the maximum blocking time period.
    /// \param[out] _topics List of advertised topics.
    public: void TopicList(std::vector<std::string> &_topics) const;

    /// \brief Get the information about a topic.
    /// \param[in] _topic Name of the topic.
    /// \param[out] _publishers List of publishers on the topic
    /// \return False if unable to get topic info
    public: bool GZ_DEPRECATED(13) TopicInfo(const std::string &_topic,
                           std::vector<MessagePublisher> &_publishers) const;

    /// \brief Get the information about a topic.
    /// \param[in] _topic Name of the topic.
    /// \param[out] _publishers List of publishers on the topic.
    /// \param[out] _subscribers List of subscribers on the topic.
    /// \return False if unable to get topic info.
    public: bool TopicInfo(const std::string &_topic,
                           std::vector<MessagePublisher> &_publishers,
                           std::vector<MessagePublisher> &_subscribers) const;

    /// \brief Get the list of topics currently advertised in the network.
    /// Note that this function can block for some time if the
    /// discovery is in its initialization phase.
    /// The value of the "heartbeatInterval" constant, with a default
    /// value of 1000ms, sets the maximum blocking time period.
    /// \param[out] _services List of advertised services.
    public: void ServiceList(std::vector<std::string> &_services) const;

    /// \brief Get the information about a service.
    /// \param[in] _service Name of the service.
    /// \param[out] _publishers List of publishers on the service.
    /// \return False if unable to get service info.
    public: bool ServiceInfo(
        const std::string &_service,
        std::vector<ServicePublisher> &_publishers) const;

    /// \brief Subscribe to a topic registering a callback. The callback must
    /// accept a std::string to represent the message data, and a MessageInfo
    /// which provides metadata about the message.
    /// \param[in] _topic Name of the topic to subscribe to
    /// \param[in] _callback A function pointer or std::function object that
    /// has a void return value and accepts two arguments:
    /// (const std::string &_msgData, const MessageInfo &_info).
    /// \param[in] _msgType The type of message to subscribe to. Using
    /// kGenericMessageType (the default) will allow this subscriber to listen
    /// to all message types. The callback function can identify the type for
    /// each message by inspecting its const MessageInfo& input argument.
    /// \param[in] _opts Options for subscribing.
    /// \return True if subscribing was successful.
    public: bool SubscribeRaw(
      const std::string &_topic,
      const RawCallback &_callback,
      const std::string &_msgType = kGenericMessageType,
      const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Get the reference to the current node options.
    /// \return Reference to the current node options.
    public: const NodeOptions &Options() const;

    /// \brief Turn topic statistics on or off.
    /// \param[in] _topic The name of the topic on which to enable or disable
    /// statistics.
    /// \param[in] _enable True to enable statistics, false to disable.
    /// \param[in] _publicationTopic Topic on which to publish statistics.
    /// \param[in] _publicationRate Rate at which to publish statistics.
    public: bool EnableStats(const std::string &_topic, bool _enable,
                const std::string &_publicationTopic = "/statistics",
                uint64_t _publicationRate = 1);

    /// \brief Get the current statistics for a topic. Statistics must
    /// have been enabled using the EnableStatistics function, otherwise
    /// the return value will be std::nullopt.
    /// \param[in] _topic The name of the topic to get statistics for.
    /// return A TopicStatistics class, or std::nullopt if statistics were
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

    /// \brief Get a pointer to the shared node (singleton shared by all the
    /// nodes).
    /// \return The pointer to the shared node.
    private: NodeShared *Shared() const;

    /// \brief Get the UUID of this node.
    /// \return The node UUID.
    private: const std::string &NodeUuid() const;

    /// \brief Get the set of topics subscribed by this node.
    /// \return The set of subscribed topics.
    private: std::unordered_set<std::string> &TopicsSubscribed() const;

    /// \brief Get the set of services advertised by this node.
    /// \return The set of advertised services.
    private: std::unordered_set<std::string> &SrvsAdvertised() const;

    /// \brief Helper function for Subscribe.
    /// \param[in] _fullyQualifiedTopic Fully qualified topic name
    /// \return True on success.
    private: bool SubscribeHelper(const std::string &_fullyQualifiedTopic);

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback does not include any message information.
    /// In this version the callback is a free function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    private: template<typename MessageT>
    std::shared_ptr<SubscriptionHandler<MessageT>> SubscribeImpl(
        const std::string &_topic,
        void(*_callback)(const MessageT &_msg),
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback does not include any message information.
    /// In this version the callback is a lambda function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Lambda function with the following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    private: template<typename MessageT>
    std::shared_ptr<SubscriptionHandler<MessageT>> SubscribeImpl(
        const std::string &_topic,
        std::function<void(const MessageT &_msg)> _callback,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback does not include any message information.
    /// In this version the callback is a member function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    private: template<typename ClassT, typename MessageT>
    std::shared_ptr<SubscriptionHandler<MessageT>> SubscribeImpl(
        const std::string &_topic,
        void(ClassT::*_callback)(const MessageT &_msg),
        ClassT *_obj,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback includes message information.
    /// In this version the callback is a free function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    private: template<typename MessageT>
    std::shared_ptr<SubscriptionHandler<MessageT>> SubscribeImpl(
        const std::string &_topic,
        void(*_callback)(const MessageT &_msg, const MessageInfo &_info),
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback includes message information.
    /// In this version the callback is a lambda function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Lambda function with the following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    private: template<typename MessageT>
    std::shared_ptr<SubscriptionHandler<MessageT>> SubscribeImpl(
        const std::string &_topic,
        std::function<void(const MessageT &_msg,
                           const MessageInfo &_info)> _callback,
        const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Subscribe to a topic registering a callback.
    /// Note that this callback includes message information.
    /// In this version the callback is a member function.
    /// \param[in] _topic Topic to be subscribed.
    /// \param[in] _callback Pointer to the callback function with the
    /// following parameters:
    ///   * _msg Protobuf message containing a new topic update.
    ///   * _info Message information (e.g.: topic name).
    /// \param[in] _obj Instance containing the member function.
    /// \param[in] _opts Subscription options.
    /// \return true when successfully subscribed or false otherwise.
    private: template<typename ClassT, typename MessageT>
    std::shared_ptr<SubscriptionHandler<MessageT>> SubscribeImpl(
        const std::string &_topic,
        void(ClassT::*_callback)(const MessageT &_msg,
                                 const MessageInfo &_info),
        ClassT *_obj,
        const SubscribeOptions &_opts = SubscribeOptions());

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Smart pointer to private data.
    private: std::unique_ptr<transport::NodePrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}

#include "gz/transport/detail/Node.hh"

#endif
