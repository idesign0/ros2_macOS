/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include <gz/msgs.hh>
#include <gz/transport.hh>

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;

  // Prepare the input parameters.
  gz::msgs::StringMsg req;
  req.set_data("HELLO");

  bool result;
  unsigned int timeout = 5000;

  std::string reqStr, repStr;
  req.SerializeToString(&reqStr);

  // Request the "/echo" service.
  bool executed = node.RequestRaw("/echo", reqStr, "gz.msgs.StringMsg",
      "gz.msgs.StringMsg", timeout, repStr, result);

  if (executed)
  {
    if (result)
    {
      gz::msgs::StringMsg rep;
      rep.ParseFromString(repStr);
      std::cout << "Response: [" << rep.data() << "]" << std::endl;
    }
    else
      std::cout << "Service call failed" << std::endl;
  }
  else
    std::cerr << "Service call timed out" << std::endl;
}
