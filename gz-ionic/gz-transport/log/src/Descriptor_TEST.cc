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

#include "gz/transport/log/Descriptor.hh"
#include "Descriptor.hh"
#include "gtest/gtest.h"

using namespace gz;
using namespace gz::transport;
using namespace gz::transport::log;

/// \brief test hook for Descriptor
class gz::transport::log::Log
{
  /// \brief Construct a descriptor
  public: static Descriptor Construct()
  {
    return Descriptor();
  }

  /// \brief call descriptor api Reset()
  /// \sa Descriptor::Implementation::Reset(const TopicKeyMap &)
  public: static void Reset(
      Descriptor &descriptor, const TopicKeyMap &_topics)
  {
    descriptor.dataPtr->Reset(_topics);
  }
};

//////////////////////////////////////////////////
TEST(Descriptor, ConstructHasNothing)
{
  Descriptor desc = Log::Construct();
  EXPECT_TRUE(desc.TopicsToMsgTypesToId().empty());
  EXPECT_TRUE(desc.MsgTypesToTopicsToId().empty());
}

//////////////////////////////////////////////////
TEST(Descriptor, ResetNoTopics)
{
  Descriptor desc = Log::Construct();
  Log::Reset(desc, TopicKeyMap());
  EXPECT_TRUE(desc.TopicsToMsgTypesToId().empty());
  EXPECT_TRUE(desc.MsgTypesToTopicsToId().empty());
}

//////////////////////////////////////////////////
TEST(Descriptor, TopicIdOneTopic)
{
  Descriptor desc = Log::Construct();
  TopicKeyMap topics;
  TopicKey key = {"/foo/bar", "gz.msgs.DNE"};
  topics[key] = 5;
  Log::Reset(desc, topics);
  EXPECT_EQ(5, desc.TopicId("/foo/bar", "gz.msgs.DNE"));
  EXPECT_GT(0, desc.TopicId("/fooo/bar", "gz.msgs.DNE"));
  EXPECT_GT(0, desc.TopicId("/foo/bar", "gz.msgs.DNEE"));
}

//////////////////////////////////////////////////
TEST(Descriptor, TopicIdMultipleTopicsSameName)
{
  Descriptor desc = Log::Construct();
  TopicKeyMap topics;
  TopicKey key1 = {"/foo/bar", "gz.msgs.DNE"};
  TopicKey key2 = {"/foo/bar", "gz.msgs.DNE2"};
  TopicKey key3 = {"/foo/bar", "gz.msgs.DNE3"};
  topics[key1] = 5;
  topics[key2] = 6;
  topics[key3] = 7;
  Log::Reset(desc, topics);
  EXPECT_EQ(5, desc.TopicId("/foo/bar", "gz.msgs.DNE"));
  EXPECT_EQ(6, desc.TopicId("/foo/bar", "gz.msgs.DNE2"));
  EXPECT_EQ(7, desc.TopicId("/foo/bar", "gz.msgs.DNE3"));
  EXPECT_GT(0, desc.TopicId("/fooo/bar", "gz.msgs.DNE"));
  EXPECT_GT(0, desc.TopicId("/foo/bar", "gz.msgs.DNEE"));
}

//////////////////////////////////////////////////
TEST(Descriptor, TopicIdMultipleTopicsSameType)
{
  Descriptor desc = Log::Construct();
  TopicKeyMap topics;
  TopicKey key1 = {"/foo/bar", "gz.msgs.DNE"};
  TopicKey key2 = {"/fiz/buz", "gz.msgs.DNE"};
  TopicKey key3 = {"/fiz/bar", "gz.msgs.DNE"};
  topics[key1] = 5;
  topics[key2] = 6;
  topics[key3] = 7;
  Log::Reset(desc, topics);
  EXPECT_EQ(5, desc.TopicId("/foo/bar", "gz.msgs.DNE"));
  EXPECT_EQ(6, desc.TopicId("/fiz/buz", "gz.msgs.DNE"));
  EXPECT_EQ(7, desc.TopicId("/fiz/bar", "gz.msgs.DNE"));
  EXPECT_GT(0, desc.TopicId("/fooo/bar", "gz.msgs.DNE"));
  EXPECT_GT(0, desc.TopicId("/foo/bar", "gz.msgs.DNEE"));
}

//////////////////////////////////////////////////
TEST(Descriptor, TopicsMapOneTopic)
{
  Descriptor desc = Log::Construct();
  TopicKeyMap topics;
  TopicKey key = {"/foo/bar", "gz.msgs.DNE"};
  topics[key] = 5;
  Log::Reset(desc, topics);
  auto topicsMap = desc.TopicsToMsgTypesToId();
  ASSERT_EQ(1u, topicsMap.size());
  EXPECT_EQ("/foo/bar", topicsMap.begin()->first);
  auto msgsMap = topicsMap.begin()->second;
  ASSERT_EQ(1u, msgsMap.size());
  EXPECT_EQ("gz.msgs.DNE", msgsMap.begin()->first);
  EXPECT_EQ(5, msgsMap.begin()->second);
}

//////////////////////////////////////////////////
TEST(Descriptor, MsgTypesMapOneTopic)
{
  Descriptor desc = Log::Construct();
  TopicKeyMap topics;
  TopicKey key = {"/foo/bar", "gz.msgs.DNE"};
  topics[key] = 5;
  Log::Reset(desc, topics);
  auto msgsMap = desc.MsgTypesToTopicsToId();
  ASSERT_EQ(1u, msgsMap.size());
  EXPECT_EQ("gz.msgs.DNE", msgsMap.begin()->first);
  auto topicsMap = msgsMap.begin()->second;
  ASSERT_EQ(1u, topicsMap.size());
  EXPECT_EQ("/foo/bar", topicsMap.begin()->first);
  EXPECT_EQ(5, topicsMap.begin()->second);
}

//////////////////////////////////////////////////
TEST(Descriptor, TopicKeyEquality)
{
  TopicKey key1 = {"/foo/bar", "gz.msgs.DNE"};
  TopicKey key2 = {"/foo/bar", "gz.msgs.DNE2"};
  TopicKey key3 = {"/foo/bar3", "gz.msgs.DNE"};
  TopicKey key4 = {"/foo/bar", "gz.msgs.DNE"};
  EXPECT_FALSE(key1 == key2);
  EXPECT_FALSE(key1 == key3);
  EXPECT_EQ(key1, key4);
}
