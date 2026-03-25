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

#ifndef GZ_TRANSPORT_REQHANDLER_HH_
#define GZ_TRANSPORT_REQHANDLER_HH_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <condition_variable>
#include <functional>
#include <memory>
#include <string>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"
#include "gz/transport/TransportTypes.hh"
#include "gz/transport/Uuid.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \class IReqHandler ReqHandler.hh gz/transport/ReqHandler.hh
  /// \brief Interface class used to manage a request handler.
  class GZ_TRANSPORT_VISIBLE IReqHandler
  {
    /// \brief Constructor.
    /// \param[in] _nUuid UUID of the node registering the request handler.
    public: explicit IReqHandler(const std::string &_nUuid)
      : rep(""),
        hUuid(Uuid().ToString()),
        nUuid(_nUuid),
        result(false),
        requested(false),
        repAvailable(false)
    {
    }

    /// \brief Destructor.
    public: virtual ~IReqHandler() = default;

    /// \brief Executes the callback registered for this handler and notify
    /// a potential requester waiting on a blocking call.
    /// \param[in] _rep Serialized data containing the response coming from
    /// the service call responser.
    /// \param[in] _result Contains the result of the service call coming from
    /// the service call responser.
    public: virtual void NotifyResult(const std::string &_rep,
                                      const bool _result) = 0;

    /// \brief Get the node UUID.
    /// \return The string representation of the node UUID.
    public: std::string NodeUuid() const
    {
      return this->nUuid;
    }

    /// \brief Get the service response as raw bytes.
    /// \return The string containing the service response.
    public: std::string Response() const
    {
      return this->rep;
    }

    /// \brief Get the result of the service response.
    /// \return The boolean result.
    public: bool Result() const
    {
      return this->result;
    }

    /// \brief Returns if this service call request has already been requested
    /// \return True when the service call has been requested.
    public: bool Requested() const
    {
      return this->requested;
    }

    /// \brief Mark the service call as requested (or not).
    /// \param[in] _value true when you want to flag this REQ as requested.
    public: void Requested(const bool _value)
    {
      this->requested = _value;
    }

    /// \brief Serialize the Req protobuf message stored.
    /// \param[out] _buffer The serialized data.
    /// \return True if the serialization succeed or false otherwise.
    public: virtual bool Serialize(std::string &_buffer) const = 0;

    /// \brief Returns the unique handler UUID.
    /// \return The handler's UUID.
    public: std::string HandlerUuid() const
    {
      return this->hUuid;
    }

    /// \brief Block the current thread until the response to the
    /// service request is available or until the timeout expires.
    /// This method uses a condition variable to notify when the response is
    /// available.
    /// \param[in] _lock Lock used to protect the condition variable.
    /// \param[in] _timeout Maximum waiting time in milliseconds.
    /// \return True if the service call was executed or false otherwise.
    public: template<typename Lock> bool WaitUntil(Lock &_lock,
                                                  const unsigned int _timeout)
    {
      auto now = std::chrono::steady_clock::now();
      return this->condition.wait_until(_lock,
        now + std::chrono::milliseconds(_timeout),
        [this]
        {
          return this->repAvailable;
        });
    }

    /// \brief Get the message type name used in the service request.
    /// \return Message type name.
    public: virtual std::string ReqTypeName() const = 0;

    /// \brief Get the message type name used in the service response.
    /// \return Message type name.
    public: virtual std::string RepTypeName() const = 0;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Condition variable used to wait until a service call REP is
    /// available.
    protected: std::condition_variable_any condition;

    /// \brief Stores the service response as raw bytes.
    protected: std::string rep;

    /// \brief Unique handler's UUID.
    protected: std::string hUuid;

    /// \brief Node UUID.
    private: std::string nUuid;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief Stores the result of the service call.
    protected: bool result;

    /// \brief When true, the REQ was already sent and the REP should be on
    /// its way. Used to not resend the same REQ more than one time.
    private: bool requested;

    /// \brief When there is a blocking service call request, the call can
    /// be unlocked when a service call REP is available. This variable
    /// captures if we have found a node that can satisfy our request.
    public: bool repAvailable;
  };

  /// \class ReqHandler ReqHandler.hh
  /// \brief It creates a reply handler for the specific protobuf
  /// messages used. 'Req' is a protobuf message type containing the input
  /// parameters of the service request. 'Rep' is a protobuf message type
  /// that will be filled with the service response.
  template <typename Req, typename Rep> class ReqHandler
    : public IReqHandler
  {
    // Documentation inherited.
    public: explicit ReqHandler(const std::string &_nUuid)
      : IReqHandler(_nUuid)
    {
    }

    /// \brief Create a specific protobuf message given its serialized data.
    /// \param[in] _data The serialized data.
    /// \return Pointer to the specific protobuf message.
    public: std::shared_ptr<Rep> CreateMsg(const std::string &_data) const
    {
      // Instantiate a specific protobuf message
      auto msgPtr = std::make_shared<Rep>();

      // Create the message using some serialized data
      if (!msgPtr->ParseFromString(_data))
      {
        std::cerr << "ReqHandler::CreateMsg() error: ParseFromString failed"
                  << std::endl;
      }

      return msgPtr;
    }

    /// \brief Set the callback for this handler.
    /// \param[in] _cb The callback with the following parameters:
    /// * _rep Protobuf message containing the service response.
    /// * _result True when the service request was successful or
    /// false otherwise.
    public: void SetCallback(const std::function <void(
      const Rep &_rep, const bool _result)> &_cb)
    {
      this->cb = _cb;
    }

    /// \brief Set the REQ protobuf message for this handler.
    /// \param[in] _reqMsg Protobuf message containing the input parameters of
    /// of the service request.
    public: void SetMessage(const Req *_reqMsg)
    {
      if (!_reqMsg)
      {
        std::cerr << "ReqHandler::SetMessage() _reqMsg is null" << std::endl;
        return;
      }

      this->reqMsg.CopyFrom(*_reqMsg);
    }

    /// \brief This function is only used for compatibility with
    /// SetResponse() when [REP = google::protobuf::Message].
    /// It shouldn't do anything.
    /// \param[in] _repMsg Protobuf message containing the variable where
    /// the result will be stored.
    public: void SetResponse(const Rep *_repMsg)
    {
      (void)_repMsg;
    }

    // Documentation inherited
    public: bool Serialize(std::string &_buffer) const
    {
      if (!this->reqMsg.SerializeToString(&_buffer))
      {
        std::cerr << "ReqHandler::Serialize(): Error serializing the request"
                  << std::endl;
        return false;
      }

      return true;
    }

    // Documentation inherited.
    public: void NotifyResult(const std::string &_rep, const bool _result)
    {
      // Execute the callback (if existing).
      if (this->cb)
      {
        // Instantiate the specific protobuf message associated to this topic.
        auto msg = this->CreateMsg(_rep);

        this->cb(*msg, _result);
      }
      else
      {
        this->rep = _rep;
        this->result = _result;
      }

      this->repAvailable = true;
      this->condition.notify_one();
    }

    // Documentation inherited.
    public: virtual std::string ReqTypeName() const
    {
      return std::string(Req().GetTypeName());
    }

    // Documentation inherited.
    public: virtual std::string RepTypeName() const
    {
      return std::string(Rep().GetTypeName());
    }

    /// \brief Protobuf message containing the request's parameters.
    private: Req reqMsg;

    /// \brief Callback to the function registered for this handler with the
    /// following parameters:
    /// \param[in] _rep Protobuf message containing the service response.
    /// \param[in] _result True when the service request was successful or
    /// false otherwise.
    private: std::function<void(const Rep &_rep, const bool _result)> cb;
  };

  /// \class ReqHandler<google::protobuf::Message> ReqHandler.hh
  /// \brief Template specialization for google::protobuf::Message.
  /// This is only used by some gz command line tools.
  template <> class ReqHandler<google::protobuf::Message,
                               google::protobuf::Message>
    : public IReqHandler
  {
    // Documentation inherited.
    public: explicit ReqHandler(const std::string &_nUuid)
      : IReqHandler(_nUuid)
    {
    }

    /// \brief Set the REQ protobuf message for this handler.
    /// \param[in] _reqMsg Protobuf message containing the input parameters of
    /// of the service request.
    public: void SetMessage(const google::protobuf::Message *_reqMsg)
    {
      if (!_reqMsg)
      {
        std::cerr << "ReqHandler::SetMessage() _reqMsg is null" << std::endl;
        return;
      }

      this->reqMsg = _reqMsg->New();
      this->reqMsg->CopyFrom(*_reqMsg);
    }

    /// \brief Set the REP protobuf message for this handler.
    /// \param[in] _repMsg Protobuf message containing the variable where
    /// the result will be stored. The only purpose of this function is to
    /// store the type information of _repMsg.
    public: void SetResponse(const google::protobuf::Message *_repMsg)
    {
      if (!_repMsg)
      {
        std::cerr << "ReqHandler::SetResponse() _repMsg is null" << std::endl;
        return;
      }

      this->repMsg = _repMsg->New();
      this->repMsg->CopyFrom(*_repMsg);
    }

    // Documentation inherited
    public: bool Serialize(std::string &_buffer) const
    {
      if (!this->reqMsg)
      {
        std::cerr << "ReqHandler::Serialize() reqMsg is null" << std::endl;
        return false;
      }

      if (!this->reqMsg->SerializeToString(&_buffer))
      {
        std::cerr << "ReqHandler::Serialize(): Error serializing the request"
                  << std::endl;
        return false;
      }

      return true;
    }

    // Documentation inherited.
    public: void NotifyResult(const std::string &_rep, const bool _result)
    {
      this->rep = _rep;
      this->result = _result;

      this->repAvailable = true;
      this->condition.notify_one();
    }

    // Documentation inherited.
    public: virtual std::string ReqTypeName() const
    {
      if (this->reqMsg)
        return std::string(this->reqMsg->GetTypeName());
      else
      {
        std::cerr << "ReqHandler::ReqTypeName() Warning: Using ReqTypeName() "
                  << "without type information" << std::endl;
        return "";
      }
    }

    //// Documentation inherited.
    public: virtual std::string RepTypeName() const
    {
      if (this->repMsg)
        return std::string(this->repMsg->GetTypeName());
      else
      {
        std::cerr << "ReqHandler::RepTypeName() Warning: Using RepTypeName() "
                  << "without type information" << std::endl;
        return "";
      }
    }

    /// \brief Protobuf message containing the request's parameters.
    private: google::protobuf::Message *reqMsg = nullptr;

    /// \brief Protobuf message containing the response.
    private: google::protobuf::Message *repMsg = nullptr;
  };
  }
}

#endif
