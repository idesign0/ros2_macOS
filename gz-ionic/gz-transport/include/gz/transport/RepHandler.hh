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

#ifndef GZ_TRANSPORT_REPHANDLER_HH_
#define GZ_TRANSPORT_REPHANDLER_HH_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <google/protobuf/message.h>
#include <google/protobuf/stubs/common.h>
#if GOOGLE_PROTOBUF_VERSION > 2999999 && GOOGLE_PROTOBUF_VERSION < 4022000
#include <google/protobuf/stubs/casts.h>
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <functional>
#include <iostream>
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
  /// \class IRepHandler RepHandler.hh gz/transport/RepHandler.hh
  /// \brief Interface class used to manage a replier handler.
  class GZ_TRANSPORT_VISIBLE IRepHandler
  {
    /// \brief Constructor.
    public: IRepHandler()
      : hUuid(Uuid().ToString())
    {
    }

    /// \brief Destructor.
    public: virtual ~IRepHandler() = default;

    /// \brief Executes the local callback registered for this handler.
    /// \param[in] _msgReq Input parameter (Protobuf message).
    /// \param[out] _msgRep Output parameter (Protobuf message).
    /// \return Service call result.
    public: virtual bool RunLocalCallback(const transport::ProtoMsg &_msgReq,
                                          transport::ProtoMsg &_msgRep) = 0;

    /// \brief Executes the callback registered for this handler.
    /// \param[in] _req Serialized data received. The data will be used
    /// to compose a specific protobuf message and will be passed to the
    /// callback function.
    /// \param[out] _rep Out parameter with the data serialized.
    /// \return Service call result.
    public: virtual bool RunCallback(const std::string &_req,
                                     std::string &_rep) = 0;

    /// \brief Get the unique UUID of this handler.
    /// \return a string representation of the handler UUID.
    public: std::string HandlerUuid() const
    {
      return this->hUuid;
    }

    /// \brief Get the message type name used in the service request.
    /// \return Message type name.
    public: virtual std::string ReqTypeName() const = 0;

    /// \brief Get the message type name used in the service response.
    /// \return Message type name.
    public: virtual std::string RepTypeName() const = 0;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::string
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Unique handler's UUID.
    protected: std::string hUuid;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  /// \class RepHandler RepHandler.hh
  /// \brief It creates a service reply handler for a pair of protobuf
  /// messages containing the request parameters and the response.
  /// 'Req' is the protobuf message type containing the input parameters of
  // the service call. 'Rep' is the protobuf message type that will be filled
  /// with the service response.
  template <typename Req, typename Rep> class RepHandler
    : public IRepHandler
  {
    // Documentation inherited.
    public: RepHandler() = default;

    /// \brief Set the callback for this handler.
    /// \param[in] _cb The callback with the following parameters:
    /// * _req Protobuf message containing the service request params
    /// * _rep Protobuf message containing the service response.
    /// * Returns true when the service response is considered
    /// successful or false otherwise.
    public: void SetCallback(
      const std::function<bool(const Req &, Rep &)> &_cb)
    {
      this->cb = _cb;
    }

    // Documentation inherited.
    public: bool RunLocalCallback(const transport::ProtoMsg &_msgReq,
                                  transport::ProtoMsg &_msgRep)
    {
      // Execute the callback (if existing)
      if (!this->cb)
      {
        std::cerr << "RepHandler::RunLocalCallback() error: "
                  << "Callback is NULL" << std::endl;
        return false;
      }

#if GOOGLE_PROTOBUF_VERSION >= 5028000
      const auto msgReq =
        google::protobuf::DynamicCastMessage<Req>(&_msgReq);
      auto msgRep =
        google::protobuf::DynamicCastMessage<Rep>(&_msgRep);
#elif GOOGLE_PROTOBUF_VERSION >= 4022000
      auto msgReq =
        google::protobuf::internal::DownCast<const Req*>(&_msgReq);
      auto msgRep = google::protobuf::internal::DownCast<Rep*>(&_msgRep);
#elif GOOGLE_PROTOBUF_VERSION > 2999999
      auto msgReq = google::protobuf::down_cast<const Req*>(&_msgReq);
      auto msgRep = google::protobuf::down_cast<Rep*>(&_msgRep);
#else
      auto msgReq =
        google::protobuf::internal::down_cast<const Req*>(&_msgReq);
      auto msgRep = google::protobuf::internal::down_cast<Rep*>(&_msgRep);
#endif

      // Verify the dynamically casted messages are valid
      if (msgReq == nullptr || msgRep == nullptr)
      {
        if (msgReq == nullptr)
        {
          if (_msgReq.GetDescriptor() != nullptr)
          {
            std::cerr << "RepHandler::RunLocalCallback() error: "
                      << "Failed to cast the request of the type "
                      << _msgReq.GetDescriptor()->full_name()
                      << " to the specified type" << '\n';
          }
          else
          {
            std::cerr << "RepHandler::RunLocalCallback() error: "
                      << "Failed to cast the request of an unknown type"
                      << " to the specified type" << '\n';
          }
        }
        if (msgRep == nullptr)
        {
          if (_msgRep.GetDescriptor() != nullptr)
          {
            std::cerr << "RepHandler::RunLocalCallback() error: "
                      << "Failed to cast the response of the type "
                      << _msgRep.GetDescriptor()->full_name()
                      << " to the specified type" << '\n';
          }
          else
          {
            std::cerr << "RepHandler::RunLocalCallback() error: "
                      << "Failed to cast the response of an unknown type"
                      << " to the specified type" << '\n';
          }
        }
        std::cerr.flush();
        return false;
      }

      return this->cb(*msgReq, *msgRep);
    }

    // Documentation inherited.
    public: bool RunCallback(const std::string &_req,
                             std::string &_rep)
    {
      // Check if we have a callback registered.
      if (!this->cb)
      {
        std::cerr << "RepHandler::RunCallback() error: "
                  << "Callback is NULL" << std::endl;
        return false;
      }

      // Instantiate the specific protobuf message associated to this topic.
      auto msgReq = this->CreateMsg(_req);
      if (!msgReq)
      {
        return false;
      }

      Rep msgRep;
      if (!this->cb(*msgReq, msgRep))
        return false;

      if (!msgRep.SerializeToString(&_rep))
      {
        std::cerr << "RepHandler::RunCallback(): Error serializing the "
                  << "response" << std::endl;
        return false;
      }

      return true;
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

    /// \brief Create a specific protobuf message given its serialized data.
    /// \param[in] _data The serialized data.
    /// \return Pointer to the specific protobuf message.
    private: std::shared_ptr<Req> CreateMsg(const std::string &_data) const
    {
      // Instantiate a specific protobuf message
      auto msgPtr = std::make_shared<Req>();

      // Create the message using some serialized data
      if (!msgPtr->ParseFromString(_data))
      {
        std::cerr << "RepHandler::CreateMsg() error: ParseFromString failed"
                  << std::endl;
      }

      return msgPtr;
    }

    /// \brief Callback to the function registered for this handler.
    private: std::function<bool(const Req &, Rep &)> cb;
  };
  }
}

#endif
