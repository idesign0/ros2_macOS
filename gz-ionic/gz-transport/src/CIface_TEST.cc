/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#include "gtest/gtest.h"

#include <gz/msgs/stringmsg.pb.h>

#include "gz/transport/CIface.h"

#include "test_utils.hh"
#include <gz/utils/Environment.hh>

static int count;

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cb(const char *_data, size_t _size, const char *_msgType, void *_userData)
{
  int *userData = static_cast<int*>(_userData);

  ASSERT_NE(nullptr, userData);
  EXPECT_EQ(42, *userData);

  gz::msgs::StringMsg msg;
  EXPECT_TRUE(msg.ParseFromArray(_data, _size));
  EXPECT_STREQ("gz.msgs.StringMsg", _msgType);
  EXPECT_EQ(msg.data(), "HELLO");
  ++count;
}

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cbNonConst(char *_data, size_t _size, char *_msgType, void *_userData)
{
  int *userData = static_cast<int*>(_userData);

  ASSERT_NE(nullptr, userData);
  EXPECT_EQ(42, *userData);

  gz::msgs::StringMsg msg;
  EXPECT_TRUE(msg.ParseFromArray(_data, _size));
  EXPECT_STREQ("gz.msgs.StringMsg", _msgType);
  EXPECT_EQ(msg.data(), "HELLO");
  ++count;
}

//////////////////////////////////////////////////
TEST(CIfaceTest, PubSub)
{
  count = 0;
  GzTransportNode *node = gzTransportNodeCreate(nullptr);
  EXPECT_NE(nullptr, node);

  const char *topic = "/foo";

  int userData = 42;

  // Subscribe
  ASSERT_EQ(0, gzTransportSubscribe(node, topic, cb, &userData));

  // Subscribe
  ASSERT_EQ(0, gzTransportSubscribeNonConst(node,
        const_cast<char *>(topic), cbNonConst, &userData));


  // Prepare the message.
  gz::msgs::StringMsg msg;
  msg.set_data("HELLO");

  // Get the size of the serialized message
#if GOOGLE_PROTOBUF_VERSION >= 3004000
  int size = msg.ByteSizeLong();
#else
  int size = msg.ByteSize();
#endif

  // Allocate space for the serialized message
  void *buffer = malloc(size);

  ASSERT_NE(nullptr, buffer);

  // Serialize the message.
  EXPECT_TRUE(msg.SerializeToArray(buffer, size));

  EXPECT_EQ(0,
    gzTransportPublish(node, topic, buffer, msg.GetTypeName().data()));

  EXPECT_EQ(2, count);

  count = 0;

  // Unsubscribe
  ASSERT_EQ(0, gzTransportUnsubscribe(node, topic));
  EXPECT_EQ(0,
    gzTransportPublish(node, topic, buffer, msg.GetTypeName().data()));
  EXPECT_EQ(0, count);

  free(buffer);
  gzTransportNodeDestroy(&node);
  EXPECT_EQ(nullptr, node);
}

//////////////////////////////////////////////////
TEST(CIfaceTest, PubSubPartitions)
{
  count = 0;
  GzTransportNode *node = gzTransportNodeCreate(nullptr);
  GzTransportNode *nodeBar = gzTransportNodeCreate("bar");
  EXPECT_NE(nullptr, node);

  const char *topic = "/foo";

  int userData = 42;

  // Subscribe on "bar" topic
  ASSERT_EQ(0, gzTransportSubscribe(nodeBar, topic, cb, &userData));

  // Subscribe
  ASSERT_EQ(0, gzTransportSubscribeNonConst(node,
        const_cast<char *>(topic), cbNonConst, &userData));

  // Prepare the message.
  gz::msgs::StringMsg msg;
  msg.set_data("HELLO");

  // Get the size of the serialized message
#if GOOGLE_PROTOBUF_VERSION >= 3004000
  int size = msg.ByteSizeLong();
#else
  int size = msg.ByteSize();
#endif

  // Allocate space for the serialized message
  void *buffer = malloc(size);

  ASSERT_NE(nullptr, buffer);

  // Serialize the message.
  EXPECT_TRUE(msg.SerializeToArray(buffer, size));

  // Publish on "bar" partition
  EXPECT_EQ(0,
    gzTransportPublish(nodeBar, topic, buffer, msg.GetTypeName().data()));
  EXPECT_EQ(1, count);

  // Publish on default partition
  EXPECT_EQ(0,
    gzTransportPublish(nodeBar, topic, buffer, msg.GetTypeName().data()));
  EXPECT_EQ(2, count);

  count = 0;

  // Unsubscribe
  ASSERT_EQ(0, gzTransportUnsubscribe(nodeBar, topic));
  EXPECT_EQ(0,
    gzTransportPublish(nodeBar, topic, buffer, msg.GetTypeName().data()));
  EXPECT_EQ(0, count);

  free(buffer);
  gzTransportNodeDestroy(&node);
  EXPECT_EQ(nullptr, node);
  gzTransportNodeDestroy(&nodeBar);
  EXPECT_EQ(nullptr, nodeBar);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Get a random partition name.
  std::string partition = testing::getRandomNumber();

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", partition);

  // Enable verbose mode.
  // gz::utils::setenv("GZ_VERBOSE", "1");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
