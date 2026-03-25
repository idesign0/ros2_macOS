/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "Descriptor.hh"

#include <string>
#include <utility>

using namespace gz::transport::log;

//////////////////////////////////////////////////
void Descriptor::Implementation::Reset(const TopicKeyMap &_columns)
{
  topicsToMsgTypesToId.clear();
  msgTypesToTopicsToId.clear();

  for (const auto &entry : _columns)
  {
    const TopicKey &key = entry.first;
    int64_t id = entry.second;

    this->topicsToMsgTypesToId[key.topic][key.type] = id;
    this->msgTypesToTopicsToId[key.type][key.topic] = id;
  }
}

//////////////////////////////////////////////////
auto Descriptor::TopicsToMsgTypesToId() const -> const NameToMap &
{
  return this->dataPtr->topicsToMsgTypesToId;
}

//////////////////////////////////////////////////
auto Descriptor::MsgTypesToTopicsToId() const -> const NameToMap &
{
  return this->dataPtr->msgTypesToTopicsToId;
}

//////////////////////////////////////////////////
int64_t Descriptor::TopicId(const std::string &_topicName,
    const std::string &_msgType) const
{
  auto iter = this->dataPtr->topicsToMsgTypesToId.find(_topicName);
  if (iter == this->dataPtr->topicsToMsgTypesToId.end())
  {
    return -1;
  }

  auto typeIter = iter->second.find(_msgType);
  if (typeIter == iter->second.end())
  {
    return -1;
  }
  return typeIter->second;
}

//////////////////////////////////////////////////
Descriptor::~Descriptor()
{
  // Destruct pimpl
}

//////////////////////////////////////////////////
Descriptor::Descriptor()
  : dataPtr(new Implementation)
{
  // Do nothing
}

//////////////////////////////////////////////////
Descriptor::Descriptor(Descriptor &&_orig)  // NOLINT(build/c++11)
  : dataPtr(std::move(_orig.dataPtr))
{
  // Do nothing
}
