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
#include <gz/msgs/int32.pb.h>

#include <chrono>
#include <string>
#include <thread>

#include "gtest/gtest.h"
#include "gz/transport/Node.hh"

#include <gz/utils/Environment.hh>

#include "test_config.hh"

using namespace gz;

static std::string g_topic = "/foo"; // NOLINT(*)

//////////////////////////////////////////////////
/// \brief A publisher node.
void advertiseAndPublish()
{
  msgs::Int32 msg;
  msg.set_data(1);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  for (auto i = 0; i < 15; ++i)
  {
    EXPECT_TRUE(pub.Publish(msg));

    // Rate: 10 msgs/sec.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Partition name has not be passed as argument" << std::endl;
    return -1;
  }

  // Set the partition name for this test.
  gz::utils::setenv("GZ_PARTITION", argv[1]);

  advertiseAndPublish();
}
