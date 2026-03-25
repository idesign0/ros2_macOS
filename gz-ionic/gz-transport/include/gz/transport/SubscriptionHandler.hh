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

#ifndef GZ_TRANSPORT_SUBSCRIPTIONHANDLER_HH_
#define GZ_TRANSPORT_SUBSCRIPTIONHANDLER_HH_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <google/protobuf/message.h>
#include <google/protobuf/stubs/common.h>
#if GOOGLE_PROTOBUF_VERSION >= 3000000 && GOOGLE_PROTOBUF_VERSION < 4022000
#include <google/protobuf/stubs/casts.h>
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <gz/msgs/Factory.hh>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"
#include "gz/transport/MessageInfo.hh"
#include "gz/transport/SubscribeOptions.hh"
#include "gz/transport/TransportTypes.hh"
#include "gz/transport/Uuid.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief SubscriptionHandlerBase contains functions and data which are
  /// common to all SubscriptionHandler types.
  class GZ_TRANSPORT_VISIBLE SubscriptionHandlerBase
  {
    /// \brief Constructor.
    /// \param[in] _nUuid UUID of the node registering the handler.
    /// \param[in] _opts Subscription options.
    public: explicit SubscriptionHandlerBase(
      const std::string &_nUuid,
      const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Destructor.
    public: virtual ~SubscriptionHandlerBase() = default;

    /// \brief Get the type of the messages from which this subscriber
    /// handler is subscribed.
    /// \return String representation of the message type.
    public: virtual std::string TypeName() = 0;

    /// \brief Get the node UUID.
    /// \return The string representation of the node UUID.
    public: std::string NodeUuid() const;

    /// \brief Get the unique UUID of this handler.
    /// \return A string representation of the handler UUID.
    public: std::string HandlerUuid() const;

    /// \brief Return whether local messages are ignored or not.
    /// \return True when local messages are ignored or false otherwise.
    public: bool IgnoreLocalMessages() const;

    /// \brief Check if message subscription is throttled. If so, verify
    /// whether the callback should be executed or not.
    /// \return true if the callback should be executed or false otherwise.
    protected: bool UpdateThrottling();

    /// \brief Subscribe options.
    protected: SubscribeOptions opts;

    /// \brief If throttling is enabled, the minimum period for receiving a
    /// message in nanoseconds.
    protected: double periodNs;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Unique handler's UUID.
    protected: std::string hUuid;

    /// \brief Timestamp of the last callback executed.
    protected: Timestamp lastCbTimestamp;

    /// \brief Node UUID.
    private: std::string nUuid;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  /// \class ISubscriptionHandler SubscriptionHandler.hh
  /// gz/transport/SubscriptionHandler.hh
  /// \brief Interface class used to manage generic protobuf messages.
  ///
  /// This extends SubscriptionHandlerBase by defining virtual functions for
  /// deserializing protobuf message data, and for receiving deserialized
  /// messages. Those functions are not needed by the RawSubscriptionHandler
  /// class.
  class GZ_TRANSPORT_VISIBLE ISubscriptionHandler
      : public SubscriptionHandlerBase
  {
    /// \brief Constructor.
    /// \param[in] _nUuid UUID of the node registering the handler.
    /// \param[in] _opts Subscription options.
    public: explicit ISubscriptionHandler(
      const std::string &_nUuid,
      const SubscribeOptions &_opts = SubscribeOptions());

    /// \brief Destructor.
    public: virtual ~ISubscriptionHandler() = default;

    /// \brief Executes the local callback registered for this handler.
    /// \param[in] _msg Protobuf message received.
    /// \param[in] _info Message information (e.g.: topic name).
    /// \return True when success, false otherwise.
    public: virtual bool RunLocalCallback(
      const ProtoMsg &_msg,
      const MessageInfo &_info) = 0;

    /// \brief Create a specific protobuf message given its serialized data.
    /// \param[in] _data The serialized data.
    /// \param[in] _type The data type.
    /// \return Pointer to the specific protobuf message.
    public: virtual const std::shared_ptr<ProtoMsg> CreateMsg(
      const std::string &_data,
      const std::string &_type) const = 0;
  };

  /// \class SubscriptionHandler SubscriptionHandler.hh
  /// \brief It creates a subscription handler for a specific protobuf
  /// message. 'T' is the Protobuf message type that will be used for this
  /// particular handler.
  template <typename T> class SubscriptionHandler
    : public ISubscriptionHandler
  {
    // Documentation inherited.
    public: explicit SubscriptionHandler(const std::string &_nUuid,
      const SubscribeOptions &_opts = SubscribeOptions())
      : ISubscriptionHandler(_nUuid, _opts)
    {
    }

    // Documentation inherited.
    public: const std::shared_ptr<ProtoMsg> CreateMsg(
      const std::string &_data,
      const std::string &/*_type*/) const
    {
      // Instantiate a specific protobuf message
      auto msgPtr = std::make_shared<T>();

      // Create the message using some serialized data
      if (!msgPtr->ParseFromString(_data))
      {
        std::cerr << "SubscriptionHandler::CreateMsg() error: ParseFromString"
                  << " failed" << std::endl;
      }

      return msgPtr;
    }

    // Documentation inherited.
    public: std::string TypeName()
    {
      return std::string(T().GetTypeName());
    }

    /// \brief Set the callback for this handler.
    /// \param[in] _cb The callback with the following parameters:
    public: void SetCallback(const MsgCallback<T> &_cb)
    {
      this->cb = _cb;
    }

    // Documentation inherited.
    public: bool RunLocalCallback(const ProtoMsg &_msg,
                                  const MessageInfo &_info)
    {
      // No callback stored.
      if (!this->cb)
      {
        std::cerr << "SubscriptionHandler::RunLocalCallback() error: "
                  << "Callback is NULL" << std::endl;
        return false;
      }

      // Check the subscription throttling option.
      if (!this->UpdateThrottling())
        return true;

#if GOOGLE_PROTOBUF_VERSION >= 5028000
      auto msgPtr = google::protobuf::DynamicCastMessage<T>(&_msg);
#elif GOOGLE_PROTOBUF_VERSION >= 4022000
      auto msgPtr = google::protobuf::internal::DownCast<const T*>(&_msg);
#elif GOOGLE_PROTOBUF_VERSION >= 3000000
      auto msgPtr = google::protobuf::down_cast<const T*>(&_msg);
#else
      auto msgPtr = google::protobuf::internal::down_cast<const T*>(&_msg);
#endif

      // Verify the dynamically casted message is valid
      if (msgPtr == nullptr)
      {
        if (_msg.GetDescriptor() != nullptr)
        {
          std::cerr << "SubscriptionHandler::RunLocalCallback() error: "
                    << "Failed to cast the message of the type "
                    << _msg.GetDescriptor()->full_name()
                    << " to the specified type" << '\n';
        }
        else
        {
          std::cerr << "SubscriptionHandler::RunLocalCallback() error: "
                    << "Failed to cast the message of an unknown type"
                    << " to the specified type" << '\n';
        }
        std::cerr.flush();
        return false;
      }

      this->cb(*msgPtr, _info);
      return true;
    }

    /// \brief Callback to the function registered for this handler.
    private: MsgCallback<T> cb;
  };

  /// \brief Specialized template when the user prefers a callbacks that
  /// accepts a generic google::protobuf::message instead of a specific type.
  template <> class SubscriptionHandler<ProtoMsg>
    : public ISubscriptionHandler
  {
    // Documentation inherited.
    public: explicit SubscriptionHandler(const std::string &_nUuid,
      const SubscribeOptions &_opts = SubscribeOptions())
      : ISubscriptionHandler(_nUuid, _opts)
    {
    }

    // Documentation inherited.
    public: const std::shared_ptr<ProtoMsg> CreateMsg(
      const std::string &_data,
      const std::string &_type) const
    {
      std::shared_ptr<google::protobuf::Message> msgPtr;

      const google::protobuf::Descriptor *desc =
        google::protobuf::DescriptorPool::generated_pool()
          ->FindMessageTypeByName(_type);

      // First, check if we have the descriptor from the generated proto
      // classes.
      if (desc)
      {
        msgPtr.reset(google::protobuf::MessageFactory::generated_factory()
          ->GetPrototype(desc)->New());
      }
      else
      {
        // Fallback on Gazebo Msgs if the message type is not found.
        msgPtr = gz::msgs::Factory::New(_type);
      }

      if (!msgPtr)
        return nullptr;

      // Create the message using some serialized data
      if (!msgPtr->ParseFromString(_data))
      {
        std::cerr << "CreateMsg() error: ParseFromString failed" << std::endl;
        return nullptr;
      }

      return msgPtr;
    }

    // Documentation inherited.
    public: std::string TypeName()
    {
      return kGenericMessageType;
    }

    /// \brief Set the callback for this handler.
    /// \param[in] _cb The callback.
    public: void SetCallback(const MsgCallback<ProtoMsg> &_cb)
    {
      this->cb = _cb;
    }

    // Documentation inherited.
    public: bool RunLocalCallback(const ProtoMsg &_msg,
                                  const MessageInfo &_info)
    {
      // No callback stored.
      if (!this->cb)
      {
        std::cerr << "SubscriptionHandler::RunLocalCallback() "
                  << "error: Callback is NULL" << std::endl;
        return false;
      }

      // Check the subscription throttling option.
      if (!this->UpdateThrottling())
        return true;

      this->cb(_msg, _info);
      return true;
    }

    /// \brief Callback to the function registered for this handler.
    private: MsgCallback<ProtoMsg> cb;
  };

  //////////////////////////////////////////////////
  /// RawSubscriptionHandler is used to manage the callback of a raw
  /// subscription.
  class RawSubscriptionHandler : public SubscriptionHandlerBase
  {
    /// \brief Constructor
    /// \param[in] _nUuid UUID of the node registering the handler
    /// \param[in] _msgType Name of message type that this handler should
    /// listen for. Setting this to kGenericMessageType will tell this handler
    /// to listen for all message types.
    /// \param[in] _opts Subscription options.
    public: explicit RawSubscriptionHandler(
      const std::string &_nUuid,
      const std::string &_msgType = kGenericMessageType,
      const SubscribeOptions &_opts = SubscribeOptions());

    // Documentation inherited
    public: std::string TypeName() override;

    /// \brief Set the callback of this handler.
    /// \param[in] _callback The callback function that will be triggered when
    /// a message is received.
    public: void SetCallback(const RawCallback &_callback);

    /// \brief Executes the raw callback registered for this handler.
    /// \param[in] _msgData Serialized string of message data
    /// \param[in] _size Number of bytes in the serialized message data
    /// \param[in] _info Meta-data for the message
    /// \return True if the callback was triggered, false if the callback was
    /// not set.
    public: bool RunRawCallback(const char *_msgData, const size_t _size,
                                const MessageInfo &_info);

    /// \brief Destructor
    public: ~RawSubscriptionHandler();

    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Pointer to the implementation of the class
    private: std::unique_ptr<Implementation> pimpl;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}

#endif
