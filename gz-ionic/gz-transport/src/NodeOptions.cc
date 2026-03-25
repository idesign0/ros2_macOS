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

#include <iostream>
#include <string>

#include "gz/transport/Helpers.hh"
#include "gz/transport/NodeOptions.hh"
#include "gz/transport/TopicUtils.hh"

#include "NodeOptionsPrivate.hh"

namespace gz::transport
{
//////////////////////////////////////////////////
NodeOptions::NodeOptions()
  : dataPtr(new NodeOptionsPrivate())
{
  // Check if the environment variable GZ_PARTITION is present.
  std::string gzPartition;
  if (env("GZ_PARTITION", gzPartition))
  {
    this->SetPartition(gzPartition);
  }
}

//////////////////////////////////////////////////
NodeOptions::NodeOptions(const NodeOptions &_other)
  : dataPtr(new NodeOptionsPrivate())
{
  (*this) = _other;
}

//////////////////////////////////////////////////
NodeOptions::~NodeOptions()
{
}

//////////////////////////////////////////////////
NodeOptions &NodeOptions::operator=(const NodeOptions &_other)
{
  this->SetNameSpace(_other.NameSpace());
  this->SetPartition(_other.Partition());
  this->dataPtr->topicsRemap = _other.dataPtr->topicsRemap;
  return *this;
}

//////////////////////////////////////////////////
const std::string &NodeOptions::NameSpace() const
{
  return this->dataPtr->ns;
}

//////////////////////////////////////////////////
bool NodeOptions::SetNameSpace(const std::string &_ns)
{
  if (!TopicUtils::IsValidNamespace(_ns))
  {
    std::cerr << "Invalid namespace [" << _ns << "]" << std::endl;
    return false;
  }
  this->dataPtr->ns = _ns;
  return true;
}

//////////////////////////////////////////////////
const std::string &NodeOptions::Partition() const
{
  return this->dataPtr->partition;
}

//////////////////////////////////////////////////
bool NodeOptions::SetPartition(const std::string &_partition)
{
  if (!TopicUtils::IsValidPartition(_partition))
  {
    std::cerr << "Invalid partition name [" << _partition << "]" << std::endl;
    return false;
  }
  this->dataPtr->partition = _partition;
  return true;
}

//////////////////////////////////////////////////
bool NodeOptions::AddTopicRemap(const std::string &_fromTopic,
                                const std::string &_toTopic)
{
  // Sanity check: Make sure that both topics are valid.
  for (auto topic : {_fromTopic, _toTopic})
  {
    if (!TopicUtils::IsValidTopic(topic))
    {
      std::cerr << "Invalid topic name [" << topic << "]" << std::endl;
      return false;
    }
  }

  // Sanity check: Make sure that the original topic hasn't been remapped
  // already
  if (this->dataPtr->topicsRemap.find(_fromTopic) !=
      this->dataPtr->topicsRemap.end())
  {
    std::cerr << "Topic name [" << _fromTopic << "] has already been remapped"
              << "to [" << this->dataPtr->topicsRemap.at(_fromTopic) << "]"
              << std::endl;
    return false;
  }

  this->dataPtr->topicsRemap[_fromTopic] = _toTopic;

  return true;
}

//////////////////////////////////////////////////
bool NodeOptions::TopicRemap(const std::string &_fromTopic,
  std::string &_toTopic) const
{
  // Is there any remap for this topic?
  auto topicIt = this->dataPtr->topicsRemap.find(_fromTopic);
  if (topicIt != this->dataPtr->topicsRemap.end())
    _toTopic = topicIt->second;

  return topicIt != this->dataPtr->topicsRemap.end();
}
}  // namespace gz::transport
