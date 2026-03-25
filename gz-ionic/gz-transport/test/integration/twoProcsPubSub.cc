/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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
#include <gz/msgs/vector3d.pb.h>

#include <chrono>
#include <string>

#include "gz/transport/Node.hh"
#include "gz/transport/TransportTypes.hh"

#include <gz/utils/Environment.hh>
#include <gz/utils/Subprocess.hh>

#include "gtest/gtest.h"
#include "test_config.hh"
#include "test_utils.hh"

using namespace gz;

static std::string partition;  // NOLINT(*)
static std::string g_FQNPartition;  // NOLINT(*)
static const std::string g_topic = "/foo";  // NOLINT(*)
static std::string data = "bar";  // NOLINT(*)
static bool cbExecuted = false;
static bool cbInfoExecuted = false;
static bool genericCbExecuted = false;
static bool cbVectorExecuted = false;
static bool cbRawExecuted = false;
static int counter = 0;

//////////////////////////////////////////////////
/// \brief Initialize some global variables.
void reset()
{
  cbExecuted = false;
  cbInfoExecuted = false;
  genericCbExecuted = false;
  cbVectorExecuted = false;
  cbRawExecuted = false;
  counter = 0;
}

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cb(const msgs::Int32 &/*_msg*/)
{
  cbExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cbInfo(const msgs::Int32 &_msg,
            const transport::MessageInfo &_info)
{
  EXPECT_EQ(_info.Topic(), g_topic);
  EXPECT_EQ(g_FQNPartition, _info.Partition());
  EXPECT_EQ(_msg.GetTypeName(), _info.Type());
  EXPECT_FALSE(_info.IntraProcess());
  cbInfoExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
/// \brief A generic callback.
void genericCb(const transport::ProtoMsg &/*_msg*/)
{
  genericCbExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
/// \brief Callback for receiving Vector3d data.
void cbVector(const msgs::Vector3d &/*_msg*/)
{
  cbVectorExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
void cbRaw(const char * /*_msgData*/, const size_t /*_size*/,
           const transport::MessageInfo &/*_info*/)
{
  cbRawExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
/// \brief Three different nodes running in two different processes. In the
/// subscriber process there are two nodes. Both should receive the message.
/// After some time one of them unsubscribe. After that check that only one
/// node receives the message.
TEST(twoProcPubSub, PubSubTwoProcsThreeNodes)
{
  transport::Node node;
  auto pub = node.Advertise<msgs::Vector3d>(g_topic);
  EXPECT_TRUE(pub);

  // No subscribers yet.
  EXPECT_FALSE(pub.HasConnections());

  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPubSubSubscriber, partition});

  msgs::Vector3d msg;
  msg.set_x(1.0);
  msg.set_y(2.0);
  msg.set_z(3.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Now, we should have subscribers.
  EXPECT_TRUE(pub.HasConnections());

  // Publish messages for a few seconds
  for (auto i = 0; i < 10; ++i)
  {
    EXPECT_TRUE(pub.Publish(msg));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

//////////////////////////////////////////////////
/// \brief This is the same as the last test, but we use PublishRaw(~) instead
/// of Publish(~).
TEST(twoProcPubSub, RawPubSubTwoProcsThreeNodes)
{
  transport::Node node;
  auto pub = node.Advertise<msgs::Vector3d>(g_topic);
  EXPECT_TRUE(pub);

  // No subscribers yet.
  EXPECT_FALSE(pub.HasConnections());

  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPubSubSubscriber, partition});

  msgs::Vector3d msg;
  msg.set_x(1.0);
  msg.set_y(2.0);
  msg.set_z(3.0);

  unsigned int retries = 0u;

  while (!pub.HasConnections() && retries++ < 5u)
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // Now, we should have subscribers.
  EXPECT_LT(retries, 5u);

  // Publish messages for a few seconds
  for (auto i = 0; i < 10; ++i)
  {
    EXPECT_TRUE(pub.PublishRaw(msg.SerializeAsString(),
          std::string(msg.GetTypeName())));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

//////////////////////////////////////////////////
/// \brief Check that a message is not received if the callback does not use
/// the advertised types.
TEST(twoProcPubSub, PubSubWrongTypesOnSubscription)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});

  reset();

  transport::Node node;
  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // Check that the message was not received.
  EXPECT_FALSE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief Same as above, but using a raw subscription.
TEST(twoProcPubSub, PubRawSubWrongTypesOnSubscription)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});

  reset();

  transport::Node node;
  EXPECT_TRUE(node.SubscribeRaw(g_topic, cbRaw,
                                std::string(msgs::Int32().GetTypeName())));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // Check that the message was not received.
  EXPECT_FALSE(cbRawExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns three subscribers on the same topic. The first
/// subscriber has a wrong callback (types in the callback does not match the
/// advertised type). The second subscriber uses the correct callback. The third
/// uses a generic callback. Check that only two of the callbacks are executed
/// (correct and generic).
TEST(twoProcPubSub, PubSubWrongTypesTwoSubscribers)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});

  reset();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  transport::Node node1;
  transport::Node node2;
  transport::Node node3;
  EXPECT_TRUE(node1.Subscribe(g_topic, cb));
  EXPECT_TRUE(node2.Subscribe(g_topic, cbVector));
  EXPECT_TRUE(node3.Subscribe(g_topic, genericCb));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  // Check that the message was not received.
  EXPECT_FALSE(cbExecuted);
  EXPECT_TRUE(cbVectorExecuted);
  EXPECT_TRUE(genericCbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns three raw subscribers on the same topic. The first
/// subscriber has the wrong type (the type specified to SubscribeRaw does not
/// match the advertised type). The second subscriber requests the correct type.
/// The third accepts the generic (default) type. Check that only two of the
/// callbacks are executed (correct and generic).
TEST(twoProcPubSub, PubSubWrongTypesTwoRawSubscribers)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});

  reset();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  bool wrongRawCbExecuted = false;
  bool correctRawCbExecuted = false;
  bool genericRawCbExecuted = false;

  auto wrongCb = [&](const char *, const size_t /*_size*/,
                     const transport::MessageInfo &)
  {
    wrongRawCbExecuted = true;
  };

  auto correctCb = [&](const char *, const size_t /*_size*/,
                       const transport::MessageInfo &)
  {
    correctRawCbExecuted = true;
  };

  auto genericCb = [&](const char *, const size_t /*_size*/,
                       const transport::MessageInfo &)
  {
    genericRawCbExecuted = true;
  };

  transport::Node node1;
  transport::Node node2;
  transport::Node node3;
  EXPECT_TRUE(node1.SubscribeRaw(g_topic, wrongCb, "wrong.msg.type"));
  EXPECT_TRUE(node2.SubscribeRaw(g_topic, correctCb,
                                 std::string(msgs::Vector3d().GetTypeName())));
  EXPECT_TRUE(node3.SubscribeRaw(g_topic, genericCb));


  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));


  // Check that the message was not received.
  EXPECT_FALSE(wrongRawCbExecuted);
  EXPECT_TRUE(correctRawCbExecuted);
  EXPECT_TRUE(genericRawCbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// subscribes to a topic and the other advertises, publishes a message and
/// terminates. This test checks that the subscriber doesn't get affected by
/// the prompt termination of the publisher.
TEST(twoProcPubSub, FastPublisher)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kFastPub, partition});

  reset();

  transport::Node node;

  EXPECT_TRUE(node.Subscribe(g_topic, cbVector));
}

//////////////////////////////////////////////////
/// \brief This test creates one publisher and one subscriber on different
/// processes. The publisher publishes at higher frequency than the rate set
/// by the subscriber.
TEST(twoProcPubSub, SubThrottled)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kPub, partition});

  reset();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  transport::Node node;
  transport::SubscribeOptions opts;
  opts.SetMsgsPerSec(1u);
  EXPECT_TRUE(node.Subscribe(g_topic, cb, opts));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  // Node published 15 messages in ~1.5 sec. We should only receive 2 messages.
  EXPECT_LT(counter, 5);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test creates one publisher and one subscriber on different
/// processes. The publisher publishes at a throttled frequency.
TEST(twoProcPubSub, PubThrottled)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kPubThrottled, partition});

  reset();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  transport::Node node;
  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  // Wait for receive some messages.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  // Node published 25 messages in ~2.5 sec. We should only receive 2 messages.
  EXPECT_LT(counter, 5);

  reset();
}

//////////////////////////////////////////////////
/// \brief Check that a message is received after Advertise->Subscribe->Publish
/// using a callback that accepts message information.
TEST(twoProcPubSub, PubSubMessageInfo)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});
  reset();

  transport::Node node;
  EXPECT_TRUE(node.Subscribe(g_topic, cbInfo));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  // Check that the message was not received.
  EXPECT_FALSE(cbInfoExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// advertises a topic and the other uses TopicList() for getting the list of
/// available topics.
TEST(twoProcPubSub, TopicList)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});

  reset();

  transport::Node node;
  std::vector<std::string> topics;

  // We need some time for discovering the other node.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  auto start1 = std::chrono::steady_clock::now();
  node.TopicList(topics);
  auto end1 = std::chrono::steady_clock::now();
  ASSERT_EQ(topics.size(), 1u);
  EXPECT_EQ(topics.at(0), g_topic);
  topics.clear();

  // Time elapsed to get the first topic list
  auto elapsed1 = std::chrono::duration_cast<std::chrono::milliseconds>
    (end1 - start1).count();

  auto start2 = std::chrono::steady_clock::now();
  node.TopicList(topics);
  auto end2 = std::chrono::steady_clock::now();
  EXPECT_EQ(topics.size(), 1u);
  EXPECT_EQ(topics.at(0), g_topic);

  // The first TopicList() call might block if the discovery is still
  // initializing (it may happen if we run this test alone).
  // However, the second call should never block.
  auto elapsed2 = std::chrono::duration_cast<std::chrono::milliseconds>
    (end2 - start2).count();
  EXPECT_LE(elapsed2, elapsed1);

  EXPECT_LT(elapsed2, 2);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// advertises a topic and the other uses TopicInfo() for getting information
/// about the topic.
TEST(twoProcPubSub, TopicInfo)
{
  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, partition});

  reset();

  transport::Node node;
  std::vector<transport::MessagePublisher> publishers;
  std::vector<transport::MessagePublisher> subscribers;

  // We need some time for discovering the other node.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  EXPECT_FALSE(node.TopicInfo("@", publishers, subscribers));
  EXPECT_EQ(publishers.size(), 0u);

  EXPECT_TRUE(node.TopicInfo("/bogus", publishers, subscribers));
  EXPECT_EQ(publishers.size(), 0u);

  EXPECT_TRUE(node.TopicInfo("/foo", publishers, subscribers));
  EXPECT_EQ(publishers.size(), 1u);
  EXPECT_EQ(publishers.front().MsgTypeName(), "gz.msgs.Vector3d");

  reset();
}

//////////////////////////////////////////////////
/// \brief Two different nodes running in two different processes. The
/// publisher in the main process here publishes a message to the
/// remote subscriber in the other process before immediately going
/// out of scope. The subscriber in the other process then unsubscribes to
/// the topic. The test verifies that the publisher node in the main process
/// is able to correctly remove its remote subscribers in the case that the
/// publisher is destroyed before the subscriber so that HasConnections()
/// check returns the correct result.
TEST(twoProcPubSub, PubSubTwoProcsScopedPub)
{
  transport::Node node;

  for (auto j = 0; j < 2; ++j)
  {
    // Start subscriber process before a publisher is created
    auto pi = gz::utils::Subprocess(
       {test_executables::kTwoProcsPubSubSingleSubscriber, partition});

    // Sleep for subscriber process to fully come up
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Reduce the publisher scope so that it is destroyed before the subscriber
    // process ends.
    {
      auto pub = node.Advertise<msgs::Vector3d>(g_topic);
      EXPECT_TRUE(pub);

      // No subscribers yet right after pub comes up because it takes time for
      // it to discover subscribers on the network
      EXPECT_FALSE(pub.HasConnections());

      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      // Now, we should have subscribers.
      EXPECT_TRUE(pub.HasConnections());

      msgs::Vector3d msg;
      msg.set_x(1.0);
      msg.set_y(2.0);
      msg.set_z(3.0);

      EXPECT_TRUE(pub.Publish(msg));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

//////////////////////////////////////////////////
/// \brief Two different nodes running in two different processes. In the
/// subscriber process there are three subscribers created using different
/// APIs. All should receive the message. After some time twoo them unsubscribe.
/// After that check that only one remaining subscriber receives the message.
TEST(twoProcPubSub, PubSubTwoProcsMixedSubscribers)
{
  transport::Node node;
  auto pub = node.Advertise<msgs::Vector3d>(g_topic);
  EXPECT_TRUE(pub);

  // No subscribers yet.
  EXPECT_FALSE(pub.HasConnections());

  auto pi = gz::utils::Subprocess(
    {test_executables::kTwoProcsPubSubMixedSubscribers, partition});

  msgs::Vector3d msg;
  msg.set_x(1.0);
  msg.set_y(2.0);
  msg.set_z(3.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // Now, we should have subscribers.
  EXPECT_TRUE(pub.HasConnections());

  // Publish messages for a few seconds
  for (auto i = 0; i < 10; ++i)
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
  g_FQNPartition = std::string("/") + partition;

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", partition);
  gz::utils::setenv("GZ_TRANSPORT_TOPIC_STATISTICS", "1");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
