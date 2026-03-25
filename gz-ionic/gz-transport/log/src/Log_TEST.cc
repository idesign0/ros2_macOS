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
#include "gtest/gtest.h"

#include <chrono>
#include <ios>
#include <string>
#include <unordered_set>

#include "gz/transport/log/Log.hh"

#include "test_utils.hh"

using namespace gz;
using namespace gz::transport;
using namespace std::chrono_literals;

namespace {
constexpr const char * kCorruptDbTestPath = CORRUPT_DB_TEST_PATH;
}

//////////////////////////////////////////////////
TEST(Log, OpenMemoryDatabase)
{
  log::Log logFile;
  EXPECT_TRUE(logFile.Open(":memory:", std::ios_base::out));
}

//////////////////////////////////////////////////
TEST(Log, OpenReadOnlyMemoryDatabase)
{
  log::Log logFile;
  EXPECT_FALSE(logFile.Open(":memory:", std::ios_base::in));
  EXPECT_FALSE(logFile.Valid());
}

//////////////////////////////////////////////////
TEST(Log, MoveConstructor)
{
  log::Log logFile;
  EXPECT_TRUE(logFile.Open(":memory:", std::ios_base::out));
  log::Log logDest(std::move(logFile));
  EXPECT_TRUE(logDest.Valid());
  EXPECT_FALSE(logFile.Valid());
}

//////////////////////////////////////////////////
TEST(Log, UnopenedLog)
{
  log::Log logFile;
  EXPECT_FALSE(logFile.Valid());
  EXPECT_EQ(std::string(""), logFile.Version());
  EXPECT_EQ(nullptr, logFile.Descriptor());
  char data[] = {1, 2, 3, 4};
  EXPECT_FALSE(logFile.InsertMessage(0ns, "/foo/bar", ".fiz.buz",
    reinterpret_cast<const void *>(data), 4));

  auto batch = logFile.QueryMessages();
  EXPECT_EQ(batch.end(), batch.begin());
}

//////////////////////////////////////////////////
TEST(Log, OpenMemoryDatabaseTwice)
{
  log::Log logFile;
  EXPECT_TRUE(logFile.Open(":memory:", std::ios_base::out));
  EXPECT_FALSE(logFile.Open(":memory:", std::ios_base::out));
}

//////////////////////////////////////////////////
TEST(Log, OpenImpossibleFileName)
{
  log::Log logFile;
  EXPECT_FALSE(logFile.Open("///////////", std::ios_base::out));
}

//////////////////////////////////////////////////
TEST(Log, InsertMessage)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::string data("Hello World");

  EXPECT_TRUE(logFile.InsertMessage(
      0ns,
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data.c_str()),
      data.size()));
}

//////////////////////////////////////////////////
TEST(Log, AllMessagesNone)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto batch = logFile.QueryMessages();
  EXPECT_EQ(batch.end(), batch.begin());
}

//////////////////////////////////////////////////
TEST(Log, InsertMessageGetMessages)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::string data1("first_data");
  std::string data2("second_data");

  EXPECT_TRUE(logFile.InsertMessage(
      1s,
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data1.c_str()),
      data1.size()));

  EXPECT_TRUE(logFile.InsertMessage(
      2s,
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data2.c_str()),
      data2.size()));

  auto batch = logFile.QueryMessages();
  auto iter = batch.begin();
  ASSERT_NE(batch.end(), iter);
  EXPECT_EQ(data1, iter->Data());
  ++iter;
  ASSERT_NE(batch.end(), iter);
  EXPECT_EQ(data2, iter->Data());
  ++iter;
  EXPECT_EQ(log::MsgIter(), iter);
}

//////////////////////////////////////////////////
TEST(Log, QueryMessagesByTopicNone)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::unordered_set<std::string> noTopics;
  auto batch = logFile.QueryMessages(
        log::TopicList::Create(noTopics));

  EXPECT_EQ(batch.end(), batch.begin());
}

//////////////////////////////////////////////////
TEST(Log, Insert2Get1MessageByTopic)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::string data1("first_data");
  std::string data2("second_data");

  EXPECT_TRUE(logFile.InsertMessage(
      1s,
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data1.c_str()),
      data1.size()));

  EXPECT_TRUE(logFile.InsertMessage(
      2s,
      "/second/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data2.c_str()),
      data2.size()));

  {
    auto batch = logFile.QueryMessages(
          log::TopicList("/some/topic/name"));

    auto iter = batch.begin();
    ASSERT_NE(batch.end(), iter);
    EXPECT_EQ(data1, iter->Data());
    ++iter;
    EXPECT_EQ(batch.end(), iter);
  }

  {
    // Test with a time range specified
    auto batch = logFile.QueryMessages(
        log::TopicList("/second/topic/name", log::QualifiedTimeRange(0s, 2s)));
    auto iter = batch.begin();
    ASSERT_NE(batch.end(), iter);
    EXPECT_EQ(data2, iter->Data());
    ++iter;
    EXPECT_EQ(batch.end(), iter);
  }
}

//////////////////////////////////////////////////
TEST(Log, CheckLogTimes)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  const std::string data1("first_data");
  EXPECT_TRUE(logFile.InsertMessage(
      1s,
      "/a/topic/name",
      "a.message.type",
      reinterpret_cast<const void *>(data1.c_str()),
      data1.size()));

  EXPECT_EQ(1s, logFile.StartTime());
  EXPECT_EQ(1s, logFile.EndTime());

  const std::string data2("second_data");
  EXPECT_TRUE(logFile.InsertMessage(
      10s,
      "/another/topic/name",
      "another.message.type",
      reinterpret_cast<const void *>(data2.c_str()),
      data2.size()));
  EXPECT_EQ(1s, logFile.StartTime());
  EXPECT_EQ(10s, logFile.EndTime());
}


//////////////////////////////////////////////////
TEST(Log, CheckVersion)
{
  log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));
  EXPECT_EQ("0.1.0", logFile.Version());
}

//////////////////////////////////////////////////
TEST(Log, NullDescriptorUnopenedLog)
{
  log::Log logFile;
  EXPECT_EQ(nullptr, logFile.Descriptor());
}

//////////////////////////////////////////////////
TEST(Log, OpenCorruptDatabase)
{
  log::Log logFile;
  logFile.Open(kCorruptDbTestPath);
  EXPECT_GT(logFile.EndTime(), 0ns) << "logFile.EndTime() == "
    << logFile.EndTime().count() << "ns";;
}
