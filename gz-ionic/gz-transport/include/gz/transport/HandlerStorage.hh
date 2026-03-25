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

#ifndef GZ_TRANSPORT_HANDLERSTORAGE_HH_
#define GZ_TRANSPORT_HANDLERSTORAGE_HH_

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "gz/transport/config.hh"
#include "gz/transport/TransportTypes.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \class HandlerStorage HandlerStorage.hh
  /// gz/transport/HandlerStorage.hh
  /// \brief Class to store and manage service call handlers.
  template<typename T> class HandlerStorage
  {
    /// \brief Stores all the service call data for each topic. The key of
    /// _data is the topic name. The value is another map, where the key is
    /// the node UUID and the value is a smart pointer to the handler.
    /// \TODO(Carlos) review this names and fix them
    using UUIDHandler_M = std::map<std::string, std::shared_ptr<T>>;
    using UUIDHandler_Collection_M = std::map<std::string, UUIDHandler_M>;

    /// \brief key is a topic name and value is UUIDHandler_Collection_M
    using TopicServiceCalls_M =
      std::map<std::string, UUIDHandler_Collection_M>;

    /// \brief Constructor.
    public: HandlerStorage() = default;

    /// \brief Destructor.
    public: virtual ~HandlerStorage() = default;

    /// \brief Get the data handlers for a topic. A request
    /// handler stores the callback and types associated to a service call
    /// request.
    /// \param[in] _topic Topic name.
    /// \param[out] _handlers Request handlers. The key of _handlers is the
    /// topic name. The value is another map, where the key is the node
    /// UUID and the value is a smart pointer to the handler.
    /// \return true if the topic contains at least one request.
    public: bool Handlers(const std::string &_topic,
      std::map<std::string,
        std::map<std::string, std::shared_ptr<T> >> &_handlers) const
    {
      if (this->data.find(_topic) == this->data.end())
        return false;

      _handlers = this->data.at(_topic);
      return true;
    }

    /// \brief Get the first handler for a topic that matches a specific pair
    /// of request/response types.
    /// \param[in] _topic Topic name.
    /// \param[in] _reqTypeName Type of the service request.
    /// \param[in] _repTypeName Type of the service response.
    /// \param[out] _handler handler.
    /// \return true if a handler was found.
    public: bool FirstHandler(const std::string &_topic,
                              const std::string &_reqTypeName,
                              const std::string &_repTypeName,
                              std::shared_ptr<T> &_handler) const
    {
      if (this->data.find(_topic) == this->data.end())
        return false;

      const auto &m = this->data.at(_topic);
      for (const auto &node : m)
      {
        for (const auto &handler : node.second)
        {
          if (_reqTypeName == handler.second->ReqTypeName() &&
              _repTypeName == handler.second->RepTypeName())
          {
            _handler = handler.second;
            return true;
          }
        }
      }
      return false;
    }

    /// \brief Get the first handler for a topic that matches a specific
    /// message type.
    /// \param[in] _topic Topic name.
    /// \param[in] _msgTypeName Type of the msg in string format.
    /// \param[out] _handler handler.
    /// \return true if a handler was found.
    public: bool FirstHandler(const std::string &_topic,
                              const std::string &_msgTypeName,
                              std::shared_ptr<T> &_handler) const
    {
      if (this->data.find(_topic) == this->data.end())
        return false;

      const auto &m = this->data.at(_topic);
      for (const auto &node : m)
      {
        for (const auto &handler : node.second)
        {
          if (_msgTypeName == handler.second->TypeName() ||
              handler.second->TypeName() == kGenericMessageType)
          {
            _handler = handler.second;
            return true;
          }
        }
      }
      return false;
    }

    /// \brief Get a specific handler.
    /// \param[in] _topic Topic name.
    /// \param[in] _nUuid Node UUID of the handler.
    /// \param[in] _hUuid Handler UUID.
    /// \param[out] _handler Handler requested.
    /// \return true if the handler was found.
    public: bool Handler(const std::string &_topic,
                         const std::string &_nUuid,
                         const std::string &_hUuid,
                         std::shared_ptr<T> &_handler) const
    {
      if (this->data.find(_topic) == this->data.end())
        return false;

      auto const &m = this->data.at(_topic);
      if (m.find(_nUuid) == m.end())
        return false;

      if (m.at(_nUuid).find(_hUuid) == m.at(_nUuid).end())
        return false;

      _handler = m.at(_nUuid).at(_hUuid);
      return true;
    }

    /// \brief Get a reference to all the handlers.
    /// \return All the handlers.
    public: TopicServiceCalls_M &AllHandlers()
    {
      return this->data;
    }

    /// \brief Add a request handler to a topic. A request handler stores
    /// the callback and types associated to a service call request.
    /// \param[in] _topic Topic name.
    /// \param[in] _nUuid Node's unique identifier.
    /// \param[in] _handler Request handler.
    public: void AddHandler(const std::string &_topic,
                            const std::string &_nUuid,
                            const std::shared_ptr<T> &_handler)
    {
      // Create the topic entry.
      if (this->data.find(_topic) == this->data.end())
        this->data[_topic] = UUIDHandler_Collection_M();

      // Create the Node UUID entry.
      if (this->data[_topic].find(_nUuid) == this->data[_topic].end())
        this->data[_topic][_nUuid] = UUIDHandler_M();

      // Add/Replace the Req handler.
      this->data[_topic][_nUuid].insert(
        std::make_pair(_handler->HandlerUuid(), _handler));
    }

    /// \brief Return true if we have stored at least one request for the
    /// topic.
    /// \param[in] _topic Topic name.
    /// \return true if we have stored at least one request for the topic.
    public: bool HasHandlersForTopic(const std::string &_topic) const
    {
      if (this->data.find(_topic) == this->data.end())
        return false;

      return !this->data.at(_topic).empty();
    }

    /// \brief Check if a node has at least one handler.
    /// \param[in] _topic Topic name.
    /// \param[in] _nUuid Node's unique identifier.
    /// \return true if the node has at least one handler registered.
    public: bool HasHandlersForNode(const std::string &_topic,
                                    const std::string &_nUuid) const
    {
      if (this->data.find(_topic) == this->data.end())
        return false;

      return this->data.at(_topic).find(_nUuid) !=
             this->data.at(_topic).end();
    }

    /// \brief Remove a request handler. The node's uuid is used as a key to
    /// remove the appropriate request handler.
    /// \param[in] _topic Topic name.
    /// \param[in] _nUuid Node's unique identifier.
    /// \param[in] _reqUuid Request's UUID to remove.
    /// \return True when the handler is removed or false otherwise.
    public: bool RemoveHandler(const std::string &_topic,
                               const std::string &_nUuid,
                               const std::string &_reqUuid)
    {
      size_t counter = 0;
      if (this->data.find(_topic) != this->data.end())
      {
        if (this->data[_topic].find(_nUuid) != this->data[_topic].end())
        {
          counter = this->data[_topic][_nUuid].erase(_reqUuid);
          if (this->data[_topic][_nUuid].empty())
            this->data[_topic].erase(_nUuid);
          if (this->data[_topic].empty())
            this->data.erase(_topic);
        }
      }

      return counter > 0;
    }

    /// \brief Remove all the handlers from a given node.
    /// \param[in] _topic Topic name.
    /// \param[in] _nUuid Node's unique identifier.
    /// \return True when at least one handler was removed or false otherwise.
    public: bool RemoveHandlersForNode(const std::string &_topic,
                                       const std::string &_nUuid)
    {
      size_t counter = 0;
      if (this->data.find(_topic) != this->data.end())
      {
        counter = this->data[_topic].erase(_nUuid);
        if (this->data[_topic].empty())
          this->data.erase(_topic);
      }

      return counter > 0;
    }

    /// \brief Stores all the service call data for each topic. The key of
    /// _data is the topic name. The value is another map, where the key is
    /// the node UUID and the value is a smart pointer to the handler.
    private: TopicServiceCalls_M data;
  };
  }
}

#endif
