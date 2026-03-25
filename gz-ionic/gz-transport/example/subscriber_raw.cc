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

#include <iostream>
#include <string>
#include <gz/msgs.hh>
#include <gz/transport.hh>

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cb(const char *_data, const size_t _size,
        const gz::transport::MessageInfo &_info)
{
  gz::msgs::StringMsg msg;
  msg.ParseFromArray(_data, _size);

  std::cout << "Msg length: " << _size << " bytes" << std::endl;
  std::cout << "Msg contents: " << msg.data() << std::endl;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  gz::transport::Node node;
  std::string topic = "/foo";

  // Subscribe to a topic by registering a callback.
  if (!node.SubscribeRaw(topic, cb))
  {
    std::cerr << "Error subscribing to topic [" << topic << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();

  return 0;
}
