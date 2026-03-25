/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <chrono>
#include <string>

#include "gz/transport/log/Log.hh"
#include "gtest/gtest.h"

using namespace gz;
using namespace std::chrono_literals;

//////////////////////////////////////////////////
TEST(Message, DefaultConstructor)
{
  transport::log::Message msg;
  EXPECT_EQ(std::string(""), msg.Data());
  EXPECT_EQ(std::string(""), msg.Topic());
  EXPECT_EQ(std::string(""), msg.Type());
  EXPECT_EQ(0ns, msg.TimeReceived());
}

//////////////////////////////////////////////////
TEST(Message, DataConstructor)
{
  std::chrono::nanoseconds goldenTime = 12345678ns;
  std::string data("SomeData");
  std::string topic("/a/topic");
  std::string msgType("msg.type");

  transport::log::Message msg(goldenTime,
      data.c_str(), data.size(),
      msgType.c_str(), msgType.size(),
      topic.c_str(), topic.size());

  EXPECT_EQ(data, msg.Data());
  EXPECT_EQ(msgType, msg.Type());
  EXPECT_EQ(topic, msg.Topic());
  EXPECT_EQ(goldenTime, msg.TimeReceived());
}
