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

#include <map>
#include <string>
#include <utility>

#include "gz/transport/TopicUtils.hh"
#include "gtest/gtest.h"

using namespace gz;

//////////////////////////////////////////////////
/// \brief Check the topic names.
TEST(TopicUtilsTest, testTopics)
{
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("abc"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("/abc"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("abc/de"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("a"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("abc/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("/abc/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("/abc/d"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic("/abc/d/e"));
  EXPECT_TRUE(transport::TopicUtils::IsValidTopic(
    std::string(transport::TopicUtils::kMaxNameLength, 'a')));

  EXPECT_FALSE(transport::TopicUtils::IsValidTopic(""));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic(" "));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("~a"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("topic1 "));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("abc//def"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("ab~cd"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("/"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("~/"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("~"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("@partition"));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic("topic:="));
  EXPECT_FALSE(transport::TopicUtils::IsValidTopic(
    std::string(transport::TopicUtils::kMaxNameLength + 1, 'a')));
}

//////////////////////////////////////////////////
/// \brief Check the namespace.
TEST(TopicUtilsTest, testNamespaces)
{
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace("/abcde"));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace("abcde"));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace("abcde/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace("/abcde/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace("/abcde/fg"));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace("/abcde/fg/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace(""));
  EXPECT_TRUE(transport::TopicUtils::IsValidNamespace(
    std::string(transport::TopicUtils::kMaxNameLength, 'a')));

  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("/"));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace(" "));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("ns "));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("abc//def"));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("ab~cd"));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("~/abcde"));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("~abcde"));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("@namespace"));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace("namespace:="));
  EXPECT_FALSE(transport::TopicUtils::IsValidNamespace(
    std::string(transport::TopicUtils::kMaxNameLength + 1, 'a')));
}


//////////////////////////////////////////////////
/// \brief Check the namespace.
TEST(TopicUtilsTest, decomposeFullyQualifiedTopic)
{
  std::string partition;
  std::string topic;

  EXPECT_TRUE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@/foo@/fiz/buz", partition, topic));
  EXPECT_EQ(std::string("/foo"), partition);
  EXPECT_EQ(std::string("/fiz/buz"), topic);

  EXPECT_TRUE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@/foo@/bar", partition, topic));
  EXPECT_EQ(std::string("/foo"), partition);
  EXPECT_EQ(std::string("/bar"), topic);

  EXPECT_TRUE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@@/bar", partition, topic));
  EXPECT_EQ(std::string(""), partition);
  EXPECT_EQ(std::string("/bar"), topic);

  EXPECT_FALSE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@/bar", partition, topic)) << partition << "|" << topic;
  EXPECT_FALSE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@@@/bar", partition, topic)) << partition << "|" << topic;
  EXPECT_FALSE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@/bar@", partition, topic)) << partition << "|" << topic;
  EXPECT_FALSE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@@", partition, topic)) << partition << "|" << topic;
  EXPECT_FALSE(transport::TopicUtils::DecomposeFullyQualifiedTopic(
    "@/foo@/", partition, topic)) << partition << "|" << topic;
}

//////////////////////////////////////////////////
/// \brief Check the partition.
TEST(TopicUtilsTest, tesPartitions)
{
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition("/abcde"));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition("abcde"));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition("abcde/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition("/abcde/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition("/abcde/fg"));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition("/abcde/fg/"));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition(""));
  EXPECT_TRUE(transport::TopicUtils::IsValidPartition(
    std::string(transport::TopicUtils::kMaxNameLength, 'a')));

  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("/"));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition(" "));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("ns "));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("abc//def"));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("ab~cd"));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("~/abcde"));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("~abcde"));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("@namespace"));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition("namespace:="));
  EXPECT_FALSE(transport::TopicUtils::IsValidPartition(
    std::string(transport::TopicUtils::kMaxNameLength + 1, 'a')));
}

//////////////////////////////////////////////////
/// \brief Check FullyQualifiedName.
TEST(TopicUtilsTest, testFullyQualifiedName)
{
  // Validation type. The key is the text under test. The value is a pair,
  // where the first element specifies if it's a valid text.
  // The second element contains the expected result in which the original
  // text will be transformed. This value only makes sense if the text is valid.
  // E.g.: {"partition/",   {true,  "@/partition@"}}
  // "partition/" is valid text for a partition name and will be transformed
  // into "@/partition@" after calling to FullyQualifiedName().
  using ValidationT = std::map<std::string, std::pair<bool, std::string>>;
  std::string longString(transport::TopicUtils::kMaxNameLength + 1, 'a');
  std::string goodString(transport::TopicUtils::kMaxNameLength - 3, 'a');

  // Partitions to test.
  ValidationT partitions =
    {
      {"@partition",   {false, ""}},
      {"@partition/@", {false, ""}},
      {"@@",           {false, ""}},
      {longString,     {false, ""}},
      {"partition",    {true,  "@/partition@"}},
      {"",             {true,  "@@"}},
      {"partition/",   {true,  "@/partition@"}},
      {goodString,     {true,  "@/" + goodString + "@"}}
    };

  // Namespaces to test.
  ValidationT namespaces =
    {
      {"~ns",      {false, ""}},
      {longString, {false, ""}},
      {"",         {true,  "/"}},
      {"abc",      {true,  "/abc/"}},
      {goodString, {true,  "/" + goodString + "/"}}
    };

  // Topics to test.
  ValidationT topics =
    {
      {"~/def",    {false, ""}},
      {"~/def/",   {false, ""}},
      {"~def",     {false, ""}},
      {"~def/",    {false, ""}},
      {longString, {false, ""}},
      {"/def",     {true,  "/def"}},
      {"def/",     {true,  "def"}},
      {"def/ghi",  {true,  "def/ghi"}},
      {"def/ghi/", {true,  "def/ghi"}},
      {goodString, {true,  goodString}}
    };

  // We try all the partition, namespaces and topics combinations.
  for (auto p : partitions)
    for (auto ns : namespaces)
      for (auto t : topics)
      {
        std::string actualTopic;
        auto pUnderTest  = p.first;
        auto nsUnderTest = ns.first;
        auto tUnderTest  = t.first;

        // If the topic starts with "/", we should ignore the namespace.
        bool isLongName;
        if (tUnderTest.front() == '/')
        {
          isLongName = p.second.second.size() + t.second.second.size() >
            transport::TopicUtils::kMaxNameLength;
        }
        else
        {
          isLongName = p.second.second.size() + ns.second.second.size() +
            t.second.second.size() > transport::TopicUtils::kMaxNameLength;
        }

        auto expectedRes = p.second.first && ns.second.first && t.second.first
          && !isLongName;
        auto actualRes   = transport::TopicUtils::FullyQualifiedName(
          pUnderTest, nsUnderTest, tUnderTest, actualTopic);
        ASSERT_TRUE(expectedRes == actualRes);

        if (expectedRes)
        {
          auto expectedP  = p.second.second;
          auto expectedNs = ns.second.second;
          auto expectedT  = t.second.second;
          std::string expectedTopic;
          // If the topic starts with "/", we should ignore the namespace.
          if (tUnderTest.front() == '/')
            expectedTopic = expectedP + expectedT;
          else
            expectedTopic = expectedP + expectedNs + expectedT;

          EXPECT_EQ(actualTopic, expectedTopic);
        }
      }
}

//////////////////////////////////////////////////
TEST(TopicUtilsTest, asValidTopic)
{
  for (auto unmodified :
    {
      "/abc",
      "abc/de",
      "a",
      "ABC/",
      "/abc",
      "/abc/d",
      "/abc/d/e",
      "a(bc)d-e_f=h+i.j"
    })
  {
    auto valid = transport::TopicUtils::AsValidTopic(unmodified);
    EXPECT_EQ(unmodified, valid);
    EXPECT_TRUE(transport::TopicUtils::IsValidTopic(valid)) << valid;
  }

  std::vector<std::pair<std::string, std::string>> modifiedStrings =
    {
      {"a b  c", "a_b__c"},
      {"a@b@c", "abc"},
      {"a:=b:=c", "abc"},
      {"a//b/c", "ab/c"},
      {"a~b~c", "abc"}
    };

  for (auto modified : modifiedStrings)
  {
    auto valid = transport::TopicUtils::AsValidTopic(modified.first);
    EXPECT_EQ(modified.second, valid);
    EXPECT_TRUE(transport::TopicUtils::IsValidTopic(valid)) << valid;
  }

  for (auto fail :
    {
      "",
      "@@@",
      "~@~",
    })
  {
    auto empty = transport::TopicUtils::AsValidTopic(fail);
    EXPECT_TRUE(empty.empty());
    EXPECT_FALSE(transport::TopicUtils::IsValidTopic(empty));
  }
}
