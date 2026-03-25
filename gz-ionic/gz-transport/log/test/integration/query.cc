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
#include <regex>
#include <string>
#include <vector>

#include "gz/transport/log/Log.hh"
#include "gtest/gtest.h"

using namespace gz;
using namespace gz::transport;
using namespace std::chrono_literals;

//////////////////////////////////////////////////
/// \brief container for test message data
typedef struct
{
  std::chrono::nanoseconds time;
  std::string topic;
  std::string type;
  std::string data;
} TestMessage;

//////////////////////////////////////////////////
std::vector<TestMessage> StandardTestMessages()
{
  std::vector<TestMessage> messages;

  messages.push_back({
      1s,
      "/topic/one",
      "msg.type.1",
      "topic1_type1_num1",
      });
  messages.push_back({
      1250ms,
      "/topic/one",
      "msg.type.2",
      "topic1_type2_num1",
      });
  messages.push_back({
      1750ms,
      "/topic/two",
      "msg.type.1",
      "topic2_type1_num1",
      });
  messages.push_back({
      2s,
      "/topic/one",
      "msg.type.1",
      "topic1_type1_num2",
      });
  messages.push_back({
      2250ms,
      "/topic/one",
      "msg.type.2",
      "topic1_type2_num2",
      });
  messages.push_back({
      2750ms,
      "/topic/two",
      "msg.type.2",
      "topic2_type1_num2",
      });
  messages.push_back({
      3s,
      "/topic/one",
      "msg.type.1",
      "topic1_type1_num3",
      });
  messages.push_back({
      3250ms,
      "/topic/one",
      "msg.type.2",
      "topic1_type2_num3",
      });
  messages.push_back({
      3750ms,
      "/topic/two",
      "msg.type.1",
      "topic2_type1_num3",
      });

  return messages;
}

//////////////////////////////////////////////////
void CheckEquality(const TestMessage &_golden, const log::Message &_uut)
{
  EXPECT_EQ(_golden.time, _uut.TimeReceived());
  EXPECT_EQ(_golden.topic, _uut.Topic());
  EXPECT_EQ(_golden.type, _uut.Type());
  EXPECT_EQ(_golden.data, _uut.Data());
}

//////////////////////////////////////////////////
void InsertMessages(log::Log &_log,
    const std::vector<TestMessage> &_messages)
{
  for (const TestMessage &msg : _messages)
  {
    ASSERT_TRUE(_log.InsertMessage(
        msg.time,
        msg.topic,
        msg.type,
        reinterpret_cast<const void *>(msg.data.c_str()),
        msg.data.size()));
  }
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsAfterInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(
      log::AllTopics(log::QualifiedTimeRange::From(beginTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 3;
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(6u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsAfterInclusiveCopy)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::INCLUSIVE);
  log::AllTopics options_orig(log::QualifiedTimeRange::From(beginTime));
  log::AllTopics options(options_orig);
  auto batch = logFile.QueryMessages(options);

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 3;
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(6u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsAfterInclusiveMove)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::INCLUSIVE);
  log::AllTopics options_orig(log::QualifiedTimeRange::From(beginTime));
  log::AllTopics options(std::move(options_orig));
  auto batch = logFile.QueryMessages(options);

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 3;
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(6u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsAfterExclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::EXCLUSIVE);
  auto batch = logFile.QueryMessages(
      log::AllTopics(log::QualifiedTimeRange::From(beginTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 4;
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(5u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsBeforeInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime endTime(3s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(
      log::AllTopics(log::QualifiedTimeRange::Until(endTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin();
  auto uutIter = batch.begin();
  for (; goldenIter != (testMessages.end() - 2) && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(7u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsBeforeExclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime endTime(3s, log::QualifiedTime::Qualifier::EXCLUSIVE);
  auto batch = logFile.QueryMessages(
      log::AllTopics(log::QualifiedTimeRange::Until(endTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin();
  auto uutIter = batch.begin();
  for (; goldenIter != (testMessages.end() - 3) && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(6u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsBetweenInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::INCLUSIVE);
  log::QualifiedTime endTime(3s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(
      log::AllTopics(log::QualifiedTimeRange(beginTime, endTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 3;
  auto uutIter = batch.begin();
  for (; goldenIter != (testMessages.end() - 2) && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(4u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryAllTopicsBetweenExclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime endTime(3s, log::QualifiedTime::Qualifier::EXCLUSIVE);
  auto batch = logFile.QueryMessages(
      log::AllTopics(log::QualifiedTimeRange(beginTime, endTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 4;
  auto uutIter = batch.begin();
  for (; goldenIter != (testMessages.end() - 3) && uutIter != batch.end();
       ++goldenIter, ++uutIter, ++num_msgs)
  {
    CheckEquality(*goldenIter, *uutIter);
  }
  EXPECT_EQ(2u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryTopicPatternAllTime)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  auto batch = logFile.QueryMessages(log::TopicPattern(std::regex(".*/two")));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin();
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter)
  {
    if (goldenIter->topic == "/topic/two")
    {
      CheckEquality(*goldenIter, *uutIter);
      ++uutIter;
      ++num_msgs;
    }
  }
  EXPECT_EQ(3u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryTopicPatternAfterInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(log::TopicPattern(
        std::regex(".*/one"), log::QualifiedTimeRange::From(beginTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 3;
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter)
  {
    if (goldenIter->topic == "/topic/one")
    {
      CheckEquality(*goldenIter, *uutIter);
      ++uutIter;
      ++num_msgs;
    }
  }
  EXPECT_EQ(4u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryTopicPatternBeforeInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime endTime(3s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(log::TopicPattern(
        std::regex(".*/one"), log::QualifiedTimeRange::Until(endTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin();
  auto uutIter = batch.begin();
  for (; goldenIter != (testMessages.end() - 2) && uutIter != batch.end();
       ++goldenIter)
  {
    if (goldenIter->topic == "/topic/one")
    {
      CheckEquality(*goldenIter, *uutIter);
      ++uutIter;
      ++num_msgs;
    }
  }
  EXPECT_EQ(5u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryTopicListAllTime)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  auto batch = logFile.QueryMessages(log::TopicList("/topic/two"));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin();
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter)
  {
    if (goldenIter->topic == "/topic/two")
    {
      CheckEquality(*goldenIter, *uutIter);
      ++uutIter;
      ++num_msgs;
    }
  }
  EXPECT_EQ(3u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryTopicListAfterInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime beginTime(2s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(log::TopicList(
        "/topic/one", log::QualifiedTimeRange::From(beginTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin() + 3;
  auto uutIter = batch.begin();
  for (; goldenIter != testMessages.end() && uutIter != batch.end();
       ++goldenIter)
  {
    if (goldenIter->topic == "/topic/one")
    {
      CheckEquality(*goldenIter, *uutIter);
      ++uutIter;
      ++num_msgs;
    }
  }
  EXPECT_EQ(4u, num_msgs);
}

//////////////////////////////////////////////////
TEST(QueryMessages, QueryTopicListBeforeInclusive)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto testMessages = StandardTestMessages();
  InsertMessages(logFile, testMessages);

  log::QualifiedTime endTime(3s, log::QualifiedTime::Qualifier::INCLUSIVE);
  auto batch = logFile.QueryMessages(log::TopicList(
        "/topic/one", log::QualifiedTimeRange::Until(endTime)));

  std::size_t num_msgs = 0;
  auto goldenIter = testMessages.begin();
  auto uutIter = batch.begin();
  for (; goldenIter != (testMessages.end() - 2) && uutIter != batch.end();
       ++goldenIter)
  {
    if (goldenIter->topic == "/topic/one")
    {
      CheckEquality(*goldenIter, *uutIter);
      ++uutIter;
      ++num_msgs;
    }
  }
  EXPECT_EQ(5u, num_msgs);
}
