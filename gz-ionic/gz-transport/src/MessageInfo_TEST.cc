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

#include <string>

#include "gz/transport/MessageInfo.hh"
#include "gtest/gtest.h"

using namespace gz;

//////////////////////////////////////////////////
/// \brief Check [Set]Topic().
TEST(MessageInfoTest, topic)
{
  transport::MessageInfo info;
  EXPECT_TRUE(info.Topic().empty());

  std::string aTopic = "/foo";
  info.SetTopic(aTopic);
  EXPECT_EQ(info.Topic(), aTopic);
}

//////////////////////////////////////////////////
/// \brief Check [Set]Type().
TEST(MessageInfoTest, type)
{
  transport::MessageInfo info;
  EXPECT_TRUE(info.Type().empty());

  std::string aType = ".msg.foo";
  info.SetType(aType);
  EXPECT_EQ(aType, info.Type());
}

//////////////////////////////////////////////////
/// \brief Check [Set]Partition().
TEST(MessageInfoTest, partition)
{
  transport::MessageInfo info;
  EXPECT_TRUE(info.Partition().empty());

  std::string aPartition = "some_partition";
  info.SetPartition(aPartition);
  EXPECT_EQ(aPartition, info.Partition());
}

//////////////////////////////////////////////////
/// \brief Check SetTopicAndPartition().
TEST(MessageInfoTest, SetTopicAndPartition)
{
  {
    transport::MessageInfo info;
    EXPECT_TRUE(info.SetTopicAndPartition("@/a_partition@/b_topic"));
    EXPECT_EQ("/a_partition", info.Partition());
    EXPECT_EQ("/b_topic", info.Topic());
  }

  {
    transport::MessageInfo info;
    EXPECT_FALSE(info.SetTopicAndPartition("/a_partition@/b_topic"));
    EXPECT_EQ("", info.Partition());
    EXPECT_EQ("", info.Topic());
  }

  {
    transport::MessageInfo info;
    EXPECT_FALSE(info.SetTopicAndPartition("@/a_partition/b_topic"));
    EXPECT_EQ("", info.Partition());
    EXPECT_EQ("", info.Topic());
  }

  {
    transport::MessageInfo info;
    EXPECT_FALSE(info.SetTopicAndPartition("/a_partition/b_topic@"));
    EXPECT_EQ("", info.Partition());
    EXPECT_EQ("", info.Topic());
  }

  {
    transport::MessageInfo info;
    EXPECT_TRUE(info.SetTopicAndPartition("@@/topic_with/no_partition"));
    EXPECT_EQ("", info.Partition());
    EXPECT_EQ("/topic_with/no_partition", info.Topic());
  }
}

//////////////////////////////////////////////////
/// \brief Check [Set]InterProcess().
TEST(MessageInfoTest, InterProcess)
{
  transport::MessageInfo info;
  EXPECT_FALSE(info.IntraProcess());

  info.SetIntraProcess(true);
  EXPECT_TRUE(info.IntraProcess());
  info.SetIntraProcess(false);
  EXPECT_FALSE(info.IntraProcess());
}

//////////////////////////////////////////////////
/// \brief Check Copy constructor.
TEST(MessageInfoTest, CopyConstructor)
{
  transport::MessageInfo info;
  info.SetTopicAndPartition("@/a_partition@/b_topic");
  info.SetIntraProcess(true);
  transport::MessageInfo infoCopy(info);

  EXPECT_EQ("/a_partition", info.Partition());
  EXPECT_EQ("/b_topic", info.Topic());
  EXPECT_EQ("/a_partition", infoCopy.Partition());
  EXPECT_EQ("/b_topic", infoCopy.Topic());
  EXPECT_TRUE(infoCopy.IntraProcess());
}
