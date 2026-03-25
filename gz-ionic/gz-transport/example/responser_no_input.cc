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
#include <string>
#include <gz/msgs.hh>
#include <gz/transport.hh>

//////////////////////////////////////////////////
/// \brief Provide a "quote" service.
/// Well OK, it's just single-quote service but do you really need more?
bool srvQuote(gz::msgs::StringMsg &_rep)
{
  std::string awesomeQuote = "This is it! This is the answer. It says here..."
    "that a bolt of lightning is going to strike the clock tower at precisely "
    "10:04pm, next Saturday night! If...If we could somehow...harness this "
    "lightning...channel it...into the flux capacitor...it just might work. "
    "Next Saturday night, we're sending you back to the future!";

  // Set the response's content.
  _rep.set_data(awesomeQuote);

  // The response succeed.
  return true;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;
  std::string service = "/quote";

  // Advertise a service call.
  if (!node.Advertise(service, srvQuote))
  {
    std::cerr << "Error advertising service [" << service << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
