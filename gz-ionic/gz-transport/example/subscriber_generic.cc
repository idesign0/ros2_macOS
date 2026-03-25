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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <iostream>
#include <string>
#include <gz/transport.hh>

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
/// Note that this callback uses the generic signature, hence it may receive
/// messages with different types.
void cb(const google::protobuf::Message &_msg,
        const gz::transport::MessageInfo &_info)
{
  std::cout << "Topic: [" << _info.Topic() << "]" << std::endl;

  if (std::string str; google::protobuf::TextFormat::PrintToString(_msg, &str))
    std::cout << str << std::endl;
  else
    std::cerr << "Error printing message" << std::endl;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  gz::transport::Node node;
  std::string topic = "/foo";

  // Subscribe to a topic by registering a callback.
  if (!node.Subscribe(topic, cb))
  {
    std::cerr << "Error subscribing to topic [" << topic << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();

  return 0;
}
