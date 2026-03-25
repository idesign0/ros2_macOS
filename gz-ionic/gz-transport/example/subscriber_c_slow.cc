/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#include <stdio.h>
#include <gz/msgs/stringmsg.pb.h>
#include <gz/transport/CIface.h>

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cb(const char *_data, const size_t _size, const char *_msgType,
        void *_userData)
{
  gz::msgs::StringMsg msg;
  msg.ParseFromArray(_data, _size);
  const char *userData;

  if (_userData)
    userData = static_cast<const char *>(_userData);
  else
    userData = "null";

  printf("UseData[%s] Msg length: %zu bytes\n", userData, _size);
  printf("UseData[%s] Msg type: %s\n", userData, _msgType);
  printf("UseData[%s] Msg contents: %s\n", userData, msg.data().c_str());
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  GzTransportNode *node = gzTransportNodeCreate(nullptr);

  const char *topic = "/foo";

  SubscribeOpts opts;
  opts.msgsPerSec = 1;

  // Subscribe to a topic by registering a callback.
  if (gzTransportSubscribeOptions(node, topic, opts, cb, nullptr) != 0)
  {
    printf("Error subscribing to topic %s.\n", topic);
    return -1;
  }

  // Zzzzzz.
  gzTransportWaitForShutdown();
  gzTransportNodeDestroy(&node);

  return 0;
}
