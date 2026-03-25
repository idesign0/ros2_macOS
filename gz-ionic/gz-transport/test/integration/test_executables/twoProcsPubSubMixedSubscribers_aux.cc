/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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
#include <gz/msgs/statistic.pb.h>
#include <gz/msgs/vector3d.pb.h>

#include <atomic>
#include <chrono>
#include <string>

#include "gz/transport/Node.hh"

#include <gz/utils/Environment.hh>

#include "gtest/gtest.h"
#include "test_config.hh"

using namespace gz;

static std::atomic<bool> cbExecuted;
static std::atomic<bool> cbRawExecuted;
static std::atomic<bool> cbCreateSubExecuted;
static std::atomic<bool> cbCreateSub2Executed;
static std::string g_topic = "/foo"; // NOLINT(*)
static std::string data = "bar"; // NOLINT(*)

//////////////////////////////////////////////////
/// \brief Function is called every time a topic update is received.
void cb(const msgs::Vector3d &_msg)
{
  EXPECT_DOUBLE_EQ(_msg.x(), 1.0);
  EXPECT_DOUBLE_EQ(_msg.y(), 2.0);
  EXPECT_DOUBLE_EQ(_msg.z(), 3.0);
  cbExecuted = true;
}

//////////////////////////////////////////////////
void cbRaw(const char *_msgData, const size_t _size,
           const transport::MessageInfo &_info)
{
  msgs::Vector3d v;

  EXPECT_TRUE(v.GetTypeName() == _info.Type());

  EXPECT_TRUE(v.ParseFromArray(_msgData, _size));

  EXPECT_DOUBLE_EQ(v.x(), 1.0);
  EXPECT_DOUBLE_EQ(v.y(), 2.0);
  EXPECT_DOUBLE_EQ(v.z(), 3.0);

  cbRawExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Function is called every time a topic update is received.
void cbCreateSub(const msgs::Vector3d &_msg)
{
  EXPECT_DOUBLE_EQ(_msg.x(), 1.0);
  EXPECT_DOUBLE_EQ(_msg.y(), 2.0);
  EXPECT_DOUBLE_EQ(_msg.z(), 3.0);
  cbCreateSubExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Function is called every time a topic update is received.
void cbCreateSub2(const msgs::Vector3d &_msg)
{
  EXPECT_DOUBLE_EQ(_msg.x(), 1.0);
  EXPECT_DOUBLE_EQ(_msg.y(), 2.0);
  EXPECT_DOUBLE_EQ(_msg.z(), 3.0);
  cbCreateSub2Executed = true;
}

//////////////////////////////////////////////////
void runSubscriber()
{
  cbExecuted = false;
  cbRawExecuted = false;
  cbCreateSubExecuted = false;
  cbCreateSub2Executed = false;

  transport::Node node;

  // Subscribe to topic using a mix of Subscribe / CreateSubscriber APIs
  EXPECT_TRUE(node.Subscribe(g_topic, cb));
  EXPECT_TRUE(node.SubscribeRaw(g_topic, cbRaw,
                                std::string(msgs::Vector3d().GetTypeName())));
  transport::Node::Subscriber sub = node.CreateSubscriber(g_topic, cbCreateSub);
  EXPECT_TRUE(sub);
  transport::Node::Subscriber sub2 = node.CreateSubscriber(g_topic,
      cbCreateSub2);
  EXPECT_TRUE(sub2);

  int interval = 100;

  // Wait until we've received at least one message in each callback.
  while (!cbExecuted || !cbRawExecuted ||
         !cbCreateSubExecuted || !cbCreateSub2Executed)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (--interval == 0)
      break;
  }

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);
  EXPECT_TRUE(cbRawExecuted);
  EXPECT_TRUE(cbCreateSubExecuted);
  EXPECT_TRUE(cbCreateSub2Executed);

  // Reset the test flags
  cbExecuted = false;
  cbRawExecuted = false;
  cbCreateSubExecuted = false;
  cbCreateSub2Executed = false;

  // Only unsubscribe 'sub'
  EXPECT_TRUE(sub.Unsubscribe());

  // Wait a small amount of time so that the master process can send some new
  // messages.
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Check that messages are received by subscribers except sub
  EXPECT_TRUE(cbExecuted);
  EXPECT_TRUE(cbRawExecuted);
  EXPECT_FALSE(cbCreateSubExecuted);
  EXPECT_TRUE(cbCreateSub2Executed);

  // Reset the test flags
  cbExecuted = false;
  cbRawExecuted = false;
  cbCreateSubExecuted = false;
  cbCreateSub2Executed = false;

  // Unsubscribe from all topics
  EXPECT_TRUE(node.Unsubscribe(g_topic));

  // Wait for messages
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  EXPECT_FALSE(cbExecuted);
  EXPECT_FALSE(cbRawExecuted);
  EXPECT_FALSE(cbCreateSubExecuted);
  EXPECT_FALSE(cbCreateSub2Executed);
}

//////////////////////////////////////////////////
TEST(twoProcPubSub, PubSubTwoProcsTwoNodesSubscriber)
{
  runSubscriber();
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cerr << "Partition name has not be passed as argument" << std::endl;
    return -1;
  }

  // Set the partition name for this test.
  gz::utils::setenv("GZ_PARTITION", argv[1]);
  gz::utils::setenv("GZ_TRANSPORT_TOPIC_STATISTICS", "1");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
