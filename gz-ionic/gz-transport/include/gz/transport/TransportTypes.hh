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
#ifndef GZ_TRANSPORT_TRANSPORTTYPES_HH_
#define GZ_TRANSPORT_TRANSPORTTYPES_HH_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gz/transport/config.hh"
#include "gz/transport/Publisher.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief Forward declarations.
  class IRepHandler;
  class IReqHandler;
  class ISubscriptionHandler;
  class RawSubscriptionHandler;
  class MessageInfo;

  /// \def Addresses_M
  /// \brief Map that stores all generic publishers.
  /// The keys are the process uuids of the nodes. For each uuid key, the
  /// value contains the list of publishers advertising the topic within the
  /// same process uuid.
  template<typename T>
  using Addresses_M = std::map<std::string, std::vector<T>>;

  /// \def MsgAddresses_M
  /// \brief Specialized Addresses_M map for message publishers.
  using MsgAddresses_M = Addresses_M<MessagePublisher>;

  /// \def SrvAddresses_M
  /// \brief Specialized Addresses_M map for service publishers.
  using SrvAddresses_M = Addresses_M<ServicePublisher>;

  /// \def ProtoMsg
  /// \brief An abbreviated protobuf message type.
  using ProtoMsg = google::protobuf::Message;

  /// \def ProtoMsgPtr
  /// \brief Shared pointer to any protobuf message.
  using ProtoMsgPtr = std::shared_ptr<ProtoMsg>;

  /// \def ISubscriptionHandlerPtr
  /// \brief Shared pointer to ISubscriptionHandler.
  using ISubscriptionHandlerPtr = std::shared_ptr<ISubscriptionHandler>;

  /// \def RawSubscriptionHandlerPtr
  /// \brief Shared pointer to RawSubscriptionHandler
  using RawSubscriptionHandlerPtr = std::shared_ptr<RawSubscriptionHandler>;

  /// \def ISubscriptionHandler_M
  /// \brief Map to store the different subscription handlers for a topic.
  /// Each node can have its own subscription handler. The node id
  /// is used as key and a pointer to a generic subscription handler is the
  /// value.
  using ISubscriptionHandler_M =
    std::map<std::string, ISubscriptionHandlerPtr>;

  /// \def RawSubscriptionHandler_M
  /// \brief Map to store the raw subscription handlers for a topic.
  /// Each node can have its own raw subscription handler. The node id is used
  /// as the key and a pointer to a raw subscription handler is the value.
  using RawSubscriptionHandler_M =
    std::map<std::string, RawSubscriptionHandlerPtr>;

  /// \def IRepHandlerPtr
  /// \brief Shared pointer to IRepHandler.
  using IRepHandlerPtr = std::shared_ptr<IRepHandler>;

  /// \def IReqHandlerPtr
  /// \brief Shared pointer to IReqHandler.
  using IReqHandlerPtr = std::shared_ptr<IReqHandler>;

  /// \def IReqHandler_M
  /// \brief Map to store the different service request handlers for a
  /// topic. Each node can have its own request handler. The node id
  /// is used as key. The value is another map, where the key is the request
  /// UUID and the value is pointer to a generic request handler.
  using IReqHandler_M =
    std::map<std::string, std::map<std::string, IReqHandlerPtr>>;

  /// \def DiscoveryCallback
  /// \brief The user can register callbacks of this type when new connections
  /// or disconnections are detected by the discovery. The prototype of the
  /// callback contains the publisher's information advertising a topic.
  /// E.g.: void onDiscoveryResponse(const MessagePublisher &_publisher).
  template <typename T>
  using DiscoveryCallback = std::function<void(const T &_publisher)>;

  /// \def MsgDiscoveryCallback
  /// \brief Specialized DiscoveryCallback function for receiving a message
  /// publisher.
  using MsgDiscoveryCallback =
    std::function<void(const MessagePublisher &_publisher)>;

  /// \def SrvDiscoveryCallback
  /// \brief Specialized DiscoveryCallback function for receiving a service
  /// publisher.
  using SrvDiscoveryCallback =
    std::function<void(const ServicePublisher &_publisher)>;

  /// \def MsgCallback
  /// \brief User callback used for receiving messages:
  ///   \param[in] _msg Protobuf message containing the topic update.
  ///   \param[in] _info Message information (e.g.: topic name).
  template <typename T>
  using MsgCallback =
    std::function<void(const T &_msg, const MessageInfo &_info)>;

  /// \def RawCallback
  /// \brief User callback used for receiving raw message data:
  /// \param[in] _msgData string of a serialized protobuf message
  /// \param[in] _size Number of bytes in the serialized message data
  /// string.
  /// \param[in] _info Message information
  using RawCallback =
      std::function<void(const char *_msgData, const size_t _size,
                         const MessageInfo &_info)>;

  /// \def Timestamp
  /// \brief Used to evaluate the validity of a discovery entry.
  using Timestamp = std::chrono::steady_clock::time_point;

  /// \def DeallocFunc
  /// \brief Used when passing data to be published using ZMQ.
  /// \param[in] _data The buffer containing the message to be published.
  /// \param[in] _hint This parameter can be used if more complex allocation
  /// mechanism is used. Say we allocated the chunk using some "allocator"
  /// object and we have to deallocate it via the same object.
  /// In such case we can pass the pointer to allocator as a hint to
  /// zmq::message_t and modify the deallocation function as follows:
  ///
  ///   void my_free (void *data, void *hint)
  ///   {
  ///     ((allocator_t*) hint)->free (data);
  ///   }
  /// \ref http://zeromq.org/blog:zero-copy
  using DeallocFunc = void(void *_data, void *_hint);

  /// \brief The string type used for generic messages.
  const std::string kGenericMessageType = "google.protobuf.Message";

  /// \brief The high water mark of the receive message buffer.
  /// \sa NodeShared::RcvHwm
  const int kDefaultRcvHwm = 1000;

  /// \brief The high water mark of the send message buffer.
  /// \sa NodeShared::SndHwm
  const int kDefaultSndHwm = 1000;
  }
}
#endif
