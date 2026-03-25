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

#ifndef GZ_TRANSPORT_PUBLISHER_HH_
#define GZ_TRANSPORT_PUBLISHER_HH_

#include <gz/msgs/discovery.pb.h>

#include <iostream>
#include <string>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {

  /// \class Publisher Publisher.hh
  /// gz/transport/Publisher.hh
  /// \brief This class stores all the information about a publisher.
  /// It stores the topic name that publishes, addresses, UUIDs, scope, etc.
  class GZ_TRANSPORT_VISIBLE Publisher
  {
    /// \brief Default constructor.
    public: Publisher() = default;

    /// \brief Constructor.
    /// \param[in] _topic Topic name.
    /// \param[in] _addr ZeroMQ address.
    /// \param[in] _pUuid Process UUID.
    /// \param[in] _nUuid node UUID.
    /// \param[in] _opts The advertise options.
    public: Publisher(const std::string &_topic,
                      const std::string &_addr,
                      const std::string &_pUuid,
                      const std::string &_nUuid,
                      const AdvertiseOptions &_opts);

    /// \brief Destructor.
    public: virtual ~Publisher() = default;

    /// \brief Get the topic published by this publisher.
    /// \return Topic name.
    /// \sa SetTopic.
    public: std::string Topic() const;

    /// \brief Get the ZeroMQ address of the publisher.
    /// \return ZeroMQ address.
    /// \sa SetAddr.
    public: std::string Addr() const;

    /// \brief Get the process UUID of the publisher.
    /// return Process UUID.
    /// \sa SetPUuid.
    public: std::string PUuid() const;

    /// \brief Get the node UUID of the publisher.
    /// \return Node UUID.
    /// \sa SetNUuid.
    public: std::string NUuid() const;

    /// \brief Get the advertised options.
    /// \return The advertised options.
    /// \sa SetOptions.
    public: virtual const AdvertiseOptions &Options() const;

    /// \brief Set the topic name published by this publisher.
    /// \param[in] _topic New topic name.
    /// \sa Topic.
    public: void SetTopic(const std::string &_topic);

    /// \brief Set ZeroMQ address of the publisher.
    /// \param[in] _addr New address.
    /// \sa Addr.
    public: void SetAddr(const std::string &_addr);

    /// \brief Set the process UUID of the publisher.
    /// \param[in] _pUuid New process UUID.
    /// \sa PUuid.
    public: void SetPUuid(const std::string &_pUuid);

    /// \brief Set the node UUID of the publisher.
    /// \param[in] _nUuid New node UUID.
    /// \sa NUuid.
    public: void SetNUuid(const std::string &_nUuid);

    /// \brief Set the advertised options.
    /// \param[in] _opts New advertised options.
    /// \sa Options.
    public: void SetOptions(const AdvertiseOptions &_opts);

    /// \brief Populate a discovery message.
    /// \param[in] _msg Message to fill.
    public: virtual void FillDiscovery(msgs::Discovery &_msg) const;

    /// \brief Set data from a discovery message.
    /// \param[in] _msg Discovery message.
    public: virtual void SetFromDiscovery(const msgs::Discovery &_msg);

    /// \brief Equality operator. This function checks if the given
    /// publisher has identical Topic, Addr, PUuid, NUuid, and Scope
    /// strings to this object.
    /// \param[in] _pub The publisher to compare against.
    /// \return True if this object matches the provided object.
    public: bool operator==(const Publisher &_pub) const;

    /// \brief Inequality operator. This function checks if the given
    /// publisher does not have identical Topic, Addr, PUuid, NUuid, and Scope
    /// strings to this object.
    /// \param[in] _pub The publisher to compare against.
    /// \return True if this object does not match the provided object.
    public: bool operator!=(const Publisher &_pub) const;

    /// \brief Stream insertion operator.
    /// \param[out] _out The output stream.
    /// \param[in] _msg Publisher to write to the stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                            const Publisher &_msg)
    {
      _out << "Publisher:"                              << std::endl
           << "\tTopic: ["       << _msg.Topic() << "]" << std::endl
           << "\tAddress: "      << _msg.Addr()         << std::endl
           << "\tProcess UUID: " << _msg.PUuid()        << std::endl
           << "\tNode UUID: "    << _msg.NUuid()        << std::endl
           << _msg.Options();

      return _out;
    }

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::string
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Topic name.
    protected: std::string topic;

    /// \brief ZeroMQ address of the publisher.
    protected: std::string addr;

    /// \brief Process UUID of the publisher.
    protected: std::string pUuid;

    /// \brief Node UUID of the publisher.
    protected: std::string nUuid;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief Advertised options.
    /// This member is not used when we have a derived publisher.
    private: AdvertiseOptions opts;
  };

  /// \class MessagePublisher Publisher.hh
  /// gz/transport/Publisher.hh
  /// \brief This class stores all the information about a message publisher.
  class GZ_TRANSPORT_VISIBLE MessagePublisher : public Publisher
  {
    /// \brief Default constructor.
    public: MessagePublisher() = default;

    /// \brief Constructor.
    /// \param[in] _topic Topic name.
    /// \param[in] _addr ZeroMQ address.
    /// \param[in] _ctrl ZeroMQ control address.
    /// \param[in] _pUuid Process UUID.
    /// \param[in] _nUuid node UUID.
    /// \param[in] _msgTypeName Message type advertised by this publisher.
    /// \param[in] _opts Advertise options.
    public: explicit MessagePublisher(const std::string &_topic,
                                      const std::string &_addr,
                                      const std::string &_ctrl,
                                      const std::string &_pUuid,
                                      const std::string &_nUuid,
                                      const std::string &_msgTypeName,
                                      const AdvertiseMessageOptions &_opts);

    /// \brief Destructor.
    public: virtual ~MessagePublisher() = default;

    /// \brief Get the ZeroMQ control address. This address is used by the
    /// subscribers to notify the publisher about the new subscription.
    /// \return ZeroMQ control address of the publisher.
    /// \sa SetCtrl.
    public: std::string Ctrl() const;

    /// \brief Set the ZeroMQ control address of the publisher.
    /// \param[in] _ctrl New control address.
    /// \sa Ctrl.
    public: void SetCtrl(const std::string &_ctrl);

    /// \brief Get the message type advertised by this publisher.
    /// \return Message type.
    public: std::string MsgTypeName() const;

    /// \brief Set the message type advertised by this publisher.
    /// \param[in] _msgTypeName New message type.
    /// \sa MsgTypeName.
    public: void SetMsgTypeName(const std::string &_msgTypeName);

    /// \brief Get the advertised options.
    /// \return The advertised options.
    /// \sa SetOptions.
    public: virtual const AdvertiseMessageOptions &Options() const;

    /// \brief Set the advertised options.
    /// \param[in] _opts New advertised options.
    /// \sa Options.
    public: void SetOptions(const AdvertiseMessageOptions &_opts);

    /// \brief Populate a discovery message.
    /// \param[in] _msg Message to fill.
    public: virtual void FillDiscovery(msgs::Discovery &_msg) const final;

    /// \brief Set data from a discovery message.
    /// \param[in] _msg Discovery message.
    public: virtual void SetFromDiscovery(const msgs::Discovery &_msg);

    /// \brief Stream insertion operator.
    /// \param[out] _out The output stream.
    /// \param[in] _msg MessagePublisher to write to the stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                            const MessagePublisher &_msg)
    {
      _out << "Publisher:"                                  << std::endl
           << "\tTopic: ["           << _msg.Topic() << "]" << std::endl
           << "\tAddress: "          << _msg.Addr()         << std::endl
           << "\tProcess UUID: "     << _msg.PUuid()        << std::endl
           << "\tNode UUID: "        << _msg.NUuid()        << std::endl
           << "\tControl address: "  << _msg.Ctrl()         << std::endl
           << "\tMessage type: "     << _msg.MsgTypeName()  << std::endl
           << _msg.Options();
      return _out;
    }

    /// \brief Equality operator. This function checks if the given
    /// message publisher has identical Topic, Addr, PUuid, NUuid, Scope,
    /// Ctrl, and MsgTypeName strings to this object.
    /// \param[in] _pub The message publisher to compare against.
    /// \return True if this object matches the provided object.
    public: bool operator==(const MessagePublisher &_pub) const;

    /// \brief Inequality operator. This function checks if the given
    /// message publisher does not have identical Topic, Addr, PUuid, NUuid,
    /// Scope, Ctrl, and MsgTypeName strings to this object.
    /// \param[in] _pub The message publisher to compare against.
    /// \return True if this object does not match the provided object.
    public: bool operator!=(const MessagePublisher &_pub) const;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief ZeroMQ control address of the publisher.
    private: std::string ctrl;

    /// \brief Message type advertised by this publisher.
    private: std::string msgTypeName;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief Advertise options (e.g.: msgsPerSec).
    private: AdvertiseMessageOptions msgOpts;
  };

  /// \class ServicePublisher Publisher.hh
  /// gz/transport/Publisher.hh
  /// \brief This class stores all the information about a service publisher.
  class GZ_TRANSPORT_VISIBLE ServicePublisher : public Publisher
  {
    /// \brief Default constructor.
    public: ServicePublisher() = default;

    /// \brief Constructor.
    /// \param[in] _topic Topic name.
    /// \param[in] _addr ZeroMQ address.
    /// \param[in] _id ZeroMQ socket ID.
    /// \param[in] _pUuid Process UUID.
    /// \param[in] _nUuid node UUID.
    /// \param[in] _reqType Message type used in the service request.
    /// \param[in] _repType Message type used in the service response.
    /// \param[in] _opts Advertise options.
    public: ServicePublisher(const std::string &_topic,
                             const std::string &_addr,
                             const std::string &_id,
                             const std::string &_pUuid,
                             const std::string &_nUuid,
                             const std::string &_reqType,
                             const std::string &_repType,
                             const AdvertiseServiceOptions &_opts);

    /// \brief Destructor.
    public: virtual ~ServicePublisher() = default;

    /// \brief Get the ZeroMQ socket ID used by this publisher.
    /// \return The socket ID.
    /// \sa SetSocketId.
    public: std::string SocketId() const;

    /// \brief Set the ZeroMQ socket ID for this publisher.
    /// \param[in] _socketId New socket ID.
    /// \sa SocketId.
    public: void SetSocketId(const std::string &_socketId);

    /// \brief Get the name of the request's protobuf message advertised.
    /// \return The protobuf message type.
    /// \sa SetReqTypeName.
    public: std::string ReqTypeName() const;

    /// \brief Get the name of the response's protobuf message advertised.
    /// \return The protobuf message type.
    /// \sa SetRepTypeName.
    public: std::string RepTypeName() const;

    /// \brief Set the name of the request's protobuf message advertised.
    /// \param[in] _reqTypeName The protobuf message type.
    /// \sa ReqTypeName.
    public: void SetReqTypeName(const std::string &_reqTypeName);

    /// \brief Set the name of the response's protobuf message advertised.
    /// \param[in] _repTypeName The protobuf message type.
    /// \sa RepTypeName.
    public: void SetRepTypeName(const std::string &_repTypeName);

    /// \brief Get the advertised options.
    /// \return The advertised options.
    /// \sa SetOptions.
    public: virtual const AdvertiseServiceOptions& Options() const;

    /// \brief Set the advertised options.
    /// \param[in] _opts New advertised options.
    /// \sa Options.
    public: void SetOptions(const AdvertiseServiceOptions &_opts);

    /// \brief Populate a discovery message.
    /// \param[in] _msg Message to fill.
    public: virtual void FillDiscovery(msgs::Discovery &_msg) const final;

    /// \brief Populate a discovery message.
    /// \brief Set data from a discovery message.
    /// \param[in] _msg Discovery message.
    public: virtual void SetFromDiscovery(const msgs::Discovery &_msg);

    /// \brief Stream insertion operator.
    /// \param[out] _out The output stream.
    /// \param[in] _msg ServicePublisher to write to the stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                            const ServicePublisher &_msg)
    {
      _out << "Publisher:"                                  << std::endl
           << "\tTopic: ["           << _msg.Topic() << "]" << std::endl
           << "\tAddress: "          << _msg.Addr()         << std::endl
           << "\tProcess UUID: "     << _msg.PUuid()        << std::endl
           << "\tNode UUID: "        << _msg.NUuid()        << std::endl
           << "\tSocket ID: "        << _msg.SocketId()     << std::endl
           << "\tRequest type: "     << _msg.ReqTypeName()  << std::endl
           << "\tResponse type: "    << _msg.RepTypeName()  << std::endl
           << _msg.Options();

      return _out;
    }

    /// \brief Equality operator. This function checks if the given
    /// service has identical Topic, Addr, PUuid, NUuid, Scope,
    /// SocketId, ReqTypeName, RepTypeName strings to this object.
    /// \param[in] _srv The service publisher to compare against.
    /// \return True if this object matches the provided object.
    public: bool operator==(const ServicePublisher &_srv) const;

    /// \brief Inequality operator. This function checks if the given
    /// service does not have identical Topic, Addr, PUuid, NUuid, Scope,
    /// SocketId, ReqTypeName, RepTypeName strings to this object.
    /// \param[in] _srv The service publisher to compare against.
    /// \return True if this object does not match the provided object.
    public: bool operator!=(const ServicePublisher &_srv) const;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::string
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief ZeroMQ socket ID used by this publisher.
    private: std::string socketId;

    /// \brief The name of the request's protobuf message advertised.
    private: std::string reqTypeName;

    /// \brief The name of the response's protobuf message advertised.
    private: std::string repTypeName;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief Advertise options.
    private: AdvertiseServiceOptions srvOpts;
  };
  }
}

#endif
