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

#include <functional>
#include <iostream>
#include <gz/common/Event.hh>

using namespace gz;

int g_callback = 0;
common::EventT<void ()> g_event;
common::ConnectionPtr g_conn;

/////////////////////////////////////////////////
void callback()
{
  g_callback++;
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));

  std::cout << "  Before event: g_callback = " << g_callback << std::endl;
  evt();
  std::cout << "     One event: g_callback = " << g_callback << std::endl;

  evt();
  evt();
  evt();

  std::cout << "   Four events: g_callback = " << g_callback << std::endl;

  std::cout << "Reset callback connection" << std::endl;

  conn.reset();

  evt();
  evt();
  evt();

  std::cout << "More events but no more callbacks: g_callback = "
            << g_callback << std::endl;
}
