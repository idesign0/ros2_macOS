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

#include <chrono>
#include <string>

#include "gz/transport/log/Message.hh"

using namespace gz::transport;
using namespace gz::transport::log;
using namespace std::chrono_literals;

class gz::transport::log::MessagePrivate
{
  /// \brief Time received
  public: std::chrono::nanoseconds timeReceived = 0ns;

  /// \brief pointer to data bytes
  public: const void *data = nullptr;

  /// \brief Length of data
  public: std::size_t dataLen = 0;

  /// \brief pointer to topic string
  public: const char *topic = nullptr;

  /// \brief Length of topic
  public: std::size_t topicLen = 0;

  /// \brief pointer to message type string
  public: const char *type = nullptr;

  /// \brief Length of message type
  public: std::size_t typeLen = 0;
};

//////////////////////////////////////////////////
Message::Message()
  : dataPtr(new MessagePrivate)
{
}

//////////////////////////////////////////////////
Message::Message(const std::chrono::nanoseconds &_timeRecv,
            const void *_data, std::size_t _dataLen,
            const char *_type, std::size_t _typeLen,
            const char *_topic, std::size_t _topicLen)
  : dataPtr(new MessagePrivate)
{
  this->dataPtr->timeReceived = _timeRecv;
  this->dataPtr->data = _data;
  this->dataPtr->dataLen = _dataLen;
  this->dataPtr->type = _type;
  this->dataPtr->typeLen = _typeLen;
  this->dataPtr->topic = _topic;
  this->dataPtr->topicLen = _topicLen;
}

//////////////////////////////////////////////////
Message::~Message()
{
}

//////////////////////////////////////////////////
std::string Message::Data() const
{
  return std::string(reinterpret_cast<const char *>(this->dataPtr->data),
      this->dataPtr->dataLen);
}

//////////////////////////////////////////////////
std::string Message::Type() const
{
  return std::string(this->dataPtr->type, this->dataPtr->typeLen);
}

//////////////////////////////////////////////////
std::string Message::Topic() const
{
  return std::string(this->dataPtr->topic, this->dataPtr->topicLen);
}

//////////////////////////////////////////////////
const std::chrono::nanoseconds &Message::TimeReceived() const
{
  return this->dataPtr->timeReceived;
}
