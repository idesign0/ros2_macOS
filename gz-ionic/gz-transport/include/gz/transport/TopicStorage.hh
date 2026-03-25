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

#ifndef GZ_TRANSPORT_TOPICSTORAGE_HH_
#define GZ_TRANSPORT_TOPICSTORAGE_HH_

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/TransportTypes.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \class TopicStorage TopicStorage.hh gz/transport/TopicStorage.hh
  /// \brief Store address information about topics and provide convenient
  /// methods for adding new topics, removing them, etc.
  template<typename T> class TopicStorage
  {
    /// \brief Constructor.
    public: TopicStorage() = default;

    /// \brief Destructor.
    public: virtual ~TopicStorage() = default;

    /// \brief Add a new address associated to a given topic and node UUID.
    /// \param[in] _publisher New publisher.
    /// \return true if the new entry is added or false if not (because it
    /// was already stored).
    public: bool AddPublisher(const T &_publisher)
    {
      // The topic does not exist.
      if (this->data.find(_publisher.Topic()) == this->data.end())
      {
        // VS2013 is buggy with initializer list here {}
        this->data[_publisher.Topic()] =
          std::map<std::string, std::vector<T>>();
      }

      // Check if the process uuid exists.
      auto &m = this->data[_publisher.Topic()];
      if (m.find(_publisher.PUuid()) != m.end())
      {
        // Check that the Publisher does not exist.
        auto &v = m[_publisher.PUuid()];
        auto found = std::find_if(v.begin(), v.end(),
          [&](const T &_pub)
          {
            return _pub.Addr()  == _publisher.Addr() &&
                   _pub.NUuid() == _publisher.NUuid();
          });

        // The publisher was already existing, just exit.
        if (found != v.end())
          return false;
      }

      // Add a new Publisher entry.
      m[_publisher.PUuid()].push_back(T(_publisher));
      return true;
    }

    /// \brief Return if there is any publisher stored for the given topic.
    /// \param[in] _topic Topic name.
    /// \return True if there is at least one entry stored for the topic.
    public: bool HasTopic(const std::string &_topic) const
    {
      return this->data.find(_topic) != this->data.end();
    }

    /// \brief Return if there is any publisher stored for the given topic and
    /// type.
    /// \param[in] _topic Topic name.
    /// \param[in] _type Topic type.
    /// \return True if there is at least one entry stored for the topic and
    /// type.
    public: bool HasTopic(const std::string &_topic,
                          const std::string &_type) const
    {
      if (!this->HasTopic(_topic))
        return false;

      // m is {pUUID=>std::vector<Publisher>}.
      auto &m = this->data.at(_topic);

      for (auto const &procs : m)
      {
        // Vector of publishers for a given topic and pUuid.
        auto &v = procs.second;
        auto found = std::find_if(v.begin(), v.end(),
          [&](const T &_pub)
          {
            return _pub.MsgTypeName() == _type ||
                   _pub.MsgTypeName() == kGenericMessageType;
          });

        // Type found!
        if (found != v.end())
          return true;
      }

      return false;
    }

    /// \brief Return if there is any publisher stored for the given topic and
    /// process UUID.
    /// \param[in] _topic Topic name.
    /// \param[in] _pUuid Process UUID of the publisher.
    /// \return True if there is at least one address stored for the topic and
    /// process UUID.
    public: bool HasAnyPublishers(const std::string &_topic,
                                  const std::string &_pUuid) const
    {
      if (!this->HasTopic(_topic))
        return false;

      return this->data.at(_topic).find(_pUuid) !=
             this->data.at(_topic).end();
    }

    /// \brief Return if the requested publisher's address is stored.
    /// \param[in] _addr Publisher's address requested
    /// \return true if the publisher's address is stored.
    public: bool HasPublisher(const std::string &_addr) const
    {
      for (auto const &topic : this->data)
      {
        for (auto const &proc : topic.second)
        {
          for (auto const &pub : proc.second)
          {
            if (pub.Addr() == _addr)
              return true;
          }
        }
      }
      return false;
    }

    /// \brief Get the address information for a given topic and node UUID.
    /// \param[in] _topic Topic name.
    /// \param[in] _pUuid Process UUID of the publisher.
    /// \param[in] _nUuid Node UUID of the publisher.
    /// \param[out] _publisher Publisher's information requested.
    /// \return true if a publisher is found for the given topic and UUID pair
    public: bool Publisher(const std::string &_topic,
                           const std::string &_pUuid,
                           const std::string &_nUuid,
                           T &_publisher) const
    {
      // Topic not found.
      if (this->data.find(_topic) == this->data.end())
        return false;

      // m is {pUUID=>Publisher}.
      auto &m = this->data.at(_topic);

      // pUuid not found.
      if (m.find(_pUuid) == m.end())
        return false;

      // Vector of 0MQ known addresses for a given topic and pUuid.
      auto &v = m.at(_pUuid);
      auto found = std::find_if(v.begin(), v.end(),
        [&](const T &_pub)
        {
          return _pub.NUuid() == _nUuid;
        });
      // Address found!
      if (found != v.end())
      {
        _publisher = *found;
        return true;
      }

      // nUuid not found.
      return false;
    }

    /// \brief Get the map of publishers stored for a given topic.
    /// \param[in] _topic Topic name.
    /// \param[out] _info Map of publishers requested.
    /// \return true if at least there is one publisher stored.
    public: bool Publishers(const std::string &_topic,
                           std::map<std::string, std::vector<T>> &_info) const
    {
      if (!this->HasTopic(_topic))
        return false;

      _info = this->data.at(_topic);
      return true;
    }

    /// \brief Remove a publisher associated to a given topic and UUID pair.
    /// \param[in] _topic Topic name
    /// \param[in] _pUuid Process UUID of the publisher.
    /// \param[in] _nUuid Node UUID of the publisher.
    /// \return True when the publisher was removed or false otherwise.
    public: bool DelPublisherByNode(const std::string &_topic,
                                    const std::string &_pUuid,
                                    const std::string &_nUuid)
    {
      size_t counter = 0;

      // Iterate over all the topics.
      if (this->data.find(_topic) != this->data.end())
      {
        // m is {pUUID=>Publisher}.
        auto &m = this->data[_topic];

        // The pUuid exists.
        if (m.find(_pUuid) != m.end())
        {
          // Vector of 0MQ known addresses for a given topic and pUuid.
          auto &v = m[_pUuid];
          auto priorSize = v.size();
          v.erase(std::remove_if(v.begin(), v.end(),
            [&](const T &_pub)
            {
              return _pub.NUuid() == _nUuid;
            }),
            v.end());
          counter = priorSize - v.size();

          if (v.empty())
            m.erase(_pUuid);

          if (m.empty())
            this->data.erase(_topic);
        }
      }

      return counter > 0;
    }

    /// \brief Remove all the publishers associated to a given process.
    /// \param[in] _pUuid Process' UUID of the publisher.
    /// \return True when at least one address was removed or false otherwise.
    public: bool DelPublishersByProc(const std::string &_pUuid)
    {
      size_t counter = 0;

      // Iterate over all the topics.
      for (auto it = this->data.begin(); it != this->data.end();)
      {
        // m is {pUUID=>Publisher}.
        auto &m = it->second;
        counter += m.erase(_pUuid);
        if (m.empty())
          this->data.erase(it++);
        else
          ++it;
      }

      return counter > 0;
    }

    /// \brief Given a process UUID, the function returns the list of
    /// publishers contained in this process UUID with its address information
    /// \param[in] _pUuid Process UUID.
    /// \param[out] _pubs Map of publishers where the keys are the node UUIDs
    /// and the value is its address information.
    public: void PublishersByProc(const std::string &_pUuid,
                           std::map<std::string, std::vector<T>> &_pubs) const
    {
      _pubs.clear();

      // Iterate over all the topics.
      for (auto const &topic : this->data)
      {
        // m is {pUUID=>Publisher}.
        auto &m = topic.second;
        if (m.find(_pUuid) != m.end())
        {
          auto &v = m.at(_pUuid);
          for (auto const &pub : v)
          {
            _pubs[pub.NUuid()].push_back(T(pub));
          }
        }
      }
    }

    /// \brief Given a process UUID and the node UUID, the function returns
    /// the list of publishers contained in the node.
    /// \param[in] _pUuid Process UUID.
    /// \param[in] _nUuid Node UUID.
    /// \param[out] _pubs Vector of publishers.
    public: void PublishersByNode(const std::string &_pUuid,
                                  const std::string &_nUuid,
                                  std::vector<T> &_pubs) const
    {
      _pubs.clear();

      // Iterate over all the topics.
      for (auto const &topic : this->data)
      {
        // m is {pUUID=>Publisher}.
        auto const &m = topic.second;
        if (m.find(_pUuid) != m.end())
        {
          auto const &v = m.at(_pUuid);
          for (auto const &pub : v)
          {
            if (pub.NUuid() == _nUuid)
            {
              _pubs.push_back(T(pub));
            }
          }
        }
      }
    }

    /// \brief Get the list of topics currently stored.
    /// \param[out] _topics List of stored topics.
    public: void TopicList(std::vector<std::string> &_topics) const
    {
      for (auto const &topic : this->data)
        _topics.push_back(topic.first);
    }

    /// \brief Print all the information for debugging purposes.
    public: void Print() const
    {
      std::cout << "---" << std::endl;
      for (auto const &topic : this->data)
      {
        std::cout << "[" << topic.first << "]" << std::endl;
        auto &m = topic.second;
        for (auto const &proc : m)
        {
          std::cout << "\tProc. UUID: " << proc.first << std::endl;
          auto &v = proc.second;
          for (auto const &publisher : v)
          {
            std::cout << publisher;
          }
        }
      }
    }

    /// \brief Clear the content.
    public: void Clear()
    {
      this->data.clear();
    }

    /// \brief The keys are topics. The values are another map, where the key
    /// is the process UUID and the value a vector of publishers.
    private: std::map<std::string,
                      std::map<std::string, std::vector<T>>> data;
  };
  }
}

#endif
