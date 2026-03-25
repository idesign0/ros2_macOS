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
#include <gz/msgs/int32.pb.h>

#include "gtest/gtest.h"
#include "gz/transport/Node.hh"
#include "gz/transport/TransportTypes.hh"

#include <gz/utils/Environment.hh>
#include <gz/utils/Subprocess.hh>

#include "test_config.hh"
#include "test_utils.hh"

using namespace gz;

static std::string partition; // NOLINT(*)
static std::string g_topic = "/foo"; // NOLINT(*)

//////////////////////////////////////////////////
TEST(authPubSub, InvalidAuth)
{
  // Setup the username and password for this test
  ASSERT_TRUE(gz::utils::setenv("GZ_TRANSPORT_USERNAME", "admin"));
  ASSERT_TRUE(gz::utils::setenv("GZ_TRANSPORT_PASSWORD", "test"));

  transport::Node node;
  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  // No subscribers yet.
  EXPECT_FALSE(pub.HasConnections());

  auto pi = gz::utils::Subprocess(
      {test_executables::kAuthPubSubSubscriberInvalid,
      partition, "bad", "invalid"});

  msgs::Int32 msg;
  msg.set_data(1);

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // \todo: Fix this when Node::Publisher::HasConnections takes into account
  // authentication.
  // We should still have no subscribers.
  // EXPECT_FALSE(pub.HasConnections());

  // Publish messages for a few seconds
  for (auto i = 0; i < 5; ++i)
  {
    EXPECT_TRUE(pub.Publish(msg));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Get a random partition name.
  partition = testing::getRandomNumber();

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", partition);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
