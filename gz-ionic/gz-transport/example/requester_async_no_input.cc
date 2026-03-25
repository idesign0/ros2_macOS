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

#include <iostream>
#include <gz/msgs.hh>
#include <gz/transport.hh>

//////////////////////////////////////////////////
/// \brief Service response callback.
void responseCb(const gz::msgs::StringMsg &_rep, const bool _result)
{
  if (_result)
    std::cout << "Response: [" << _rep.data() << "]" << std::endl;
  else
    std::cerr << "Service call failed" << std::endl;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;

  std::cout << "Press <CTRL-C> to exit" << std::endl;

  // Request the "/quote" service.
  node.Request("/quote", responseCb);

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
