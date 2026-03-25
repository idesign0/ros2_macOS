/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include <unordered_set>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146 4251)
#endif
#include <google/protobuf/text_format.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "DynamicFactory.hh"
#include "gz/msgs/MessageFactory.hh"
#include <gz/utils/ImplPtr.hh>

static constexpr const char * kGzMsgsPrefix = "gz.msgs.";

namespace gz::msgs
{

/////////////////////////////////////////////////
MessageFactory::MessageFactory():
  dynamicFactory(gz::utils::MakeUniqueImpl<gz::msgs::DynamicFactory>())
{
}

/////////////////////////////////////////////////
MessageFactory::~MessageFactory() = default;

/////////////////////////////////////////////////
void MessageFactory::Register(const std::string &_msgType,
                              FactoryFn _factoryfn)
{
  msgMap[_msgType] = _factoryfn;
}

/////////////////////////////////////////////////
MessageFactory::MessagePtr MessageFactory::New(
    const std::string &_msgType)
{
  std::string type;

  // Convert "gz_msgs." prefix
  if (_msgType.find("gz_msgs.") == 0)
  {
    type = kGzMsgsPrefix + _msgType.substr(8);
  }
  // Convert ".gz.msgs." prefix
  else if (_msgType.find(".gz.msgs.") == 0)
  {
    type = kGzMsgsPrefix + _msgType.substr(9);
  }
  // Convert ".gz_msgs." prefix
  else if (_msgType.find(".gz_msgs.") == 0)
  {
    type = kGzMsgsPrefix +  _msgType.substr(9);
  }
  else
  {
    type = _msgType;
  }

  auto getMessagePtr = [this](const std::string &_type)
  {
    MessageFactory::MessagePtr ret;
    if (auto it = msgMap.find(_type); it != msgMap.end())
    {
      // Create a new message via FactoryFn
      ret = it->second();
    }
    else
    {
      // Create a new message via dynamic descriptors
      ret = dynamicFactory->New(_type);
    }
    return ret;
  };

  auto ret = getMessagePtr(type);

  // Message was not found in either static or dynamic message types,
  // try again adding the gz.msgs prefix
  if (nullptr == ret)
  {
    ret = getMessagePtr(kGzMsgsPrefix + type);
    if (nullptr != ret)
    {
      std::cerr << "Message (" << kGzMsgsPrefix + type
          << ") was retrieved with non-fully qualified name. "
          << "This behavior is deprecated in msgs11" << std::endl;
    }
  }
  return ret;
}

/////////////////////////////////////////////////
MessageFactory::MessagePtr MessageFactory::New(
    const std::string &_msgType, const std::string &_args)
{
  std::unique_ptr<google::protobuf::Message> msg = New(_msgType);
  if (msg)
  {
    if (!google::protobuf::TextFormat::ParseFromString(_args, msg.get()))
    {
      // The user-provided string was invalid,
      // return nullptr rather than an empty message.
      msg.reset();
    }
  }
  return msg;
}

/////////////////////////////////////////////////
void MessageFactory::Types(std::vector<std::string> &_types)
{
  _types.clear();

  // Add the types loaded from descriptor files
  std::vector<std::string> dynTypes;
  this->dynamicFactory->Types(dynTypes);

  // Use set to remove duplicates
  std::unordered_set<std::string> typesSet(dynTypes.begin(), dynTypes.end());

  // Return the list of all known message types.
  for (auto iter = msgMap.begin(); iter != msgMap.end(); ++iter)
  {
    typesSet.insert(iter->first);
  }

  std::copy(typesSet.begin(), typesSet.end(), std::back_inserter(_types));
}

/////////////////////////////////////////////////
void MessageFactory::LoadDescriptors(const std::string &_paths)
{
  dynamicFactory->LoadDescriptors(_paths);
}

}  // namespace gz::msgs
