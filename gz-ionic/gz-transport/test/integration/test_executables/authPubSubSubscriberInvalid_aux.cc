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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
#include <gz/msgs/int32.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef _WIN32
  #include <filesystem>
#endif

#include "gz/transport/Node.hh"

#include <gz/utils/Environment.hh>

#include "gtest/gtest.h"
#include "test_config.hh"

using namespace gz;

static bool cbExecuted;
static std::string g_topic = "/foo"; // NOLINT(*)

//////////////////////////////////////////////////
/// \brief Function is called every time a topic update is received.
void cb(const msgs::Int32 &/*_msg*/)
{
  std::cerr << "CALLBACK\n";
  cbExecuted = true;
}

//////////////////////////////////////////////////
TEST(authProcPubSub, PubSubTwoProcsTwoNodesSubscriber)
{
  cbExecuted = false;

  transport::Node node;

  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  int interval = 100;

  while (!cbExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    interval--;

    if (interval == 0)
      break;
  }

  // Check that no messages were received.
  EXPECT_FALSE(cbExecuted);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  if (argc != 4)
  {
    std::cerr << "Partition name, username, and password have not be passed as "
      << "arguments" << std::endl;
    return -1;
  }

  // Set the partition name for this test.
  gz::utils::setenv("GZ_PARTITION", argv[1]);

  // Set the username for this test.
  gz::utils::setenv("GZ_TRANSPORT_USERNAME", argv[2]);

  // Set the password for this test.
  gz::utils::setenv("GZ_TRANSPORT_PASSWORD", argv[3]);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
