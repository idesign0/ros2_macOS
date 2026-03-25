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

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/TopicStorage.hh"
#include "gtest/gtest.h"

using namespace gz;
using namespace transport;

// Global variables.
static const std::string      g_topic1 = "foo"; // NOLINT(*)
static const std::string      g_topic2 = "foo2"; // NOLINT(*)

static const std::string      g_pUuid1 = "process-UUID-1"; // NOLINT(*)
static const std::string      g_addr1  = "tcp://10.0.0.1:6001"; // NOLINT(*)
static const std::string      g_nUuid1 = "node-UUID-1"; // NOLINT(*)
static const Scope_t          g_scope1 = Scope_t::ALL;
static       AdvertiseOptions g_opts1  = AdvertiseOptions();

static const std::string      g_pUuid2 = "process-UUID-2"; // NOLINT(*)
static const std::string      g_addr2  = "tcp://10.0.0.1:6002"; // NOLINT(*)
static const std::string      g_nUuid2 = "node-UUID-2"; // NOLINT(*)
static const Scope_t          g_scope2 = Scope_t::PROCESS;
static       AdvertiseOptions g_opts2  = AdvertiseOptions();

static const std::string      g_nUuid3 = "node-UUID-3"; // NOLINT(*)
static const Scope_t          g_scope3 = Scope_t::HOST;
static       AdvertiseOptions g_opts3  = AdvertiseOptions();

static const std::string      g_nUuid4 = "node-UUID-4"; // NOLINT(*)
static const Scope_t          g_scope4 = Scope_t::ALL;
static       AdvertiseOptions g_opts4  = AdvertiseOptions();

//////////////////////////////////////////////////
/// \brief Initialize global variables.
void init()
{
  g_opts1.SetScope(g_scope1);
  g_opts2.SetScope(g_scope2);
  g_opts3.SetScope(g_scope3);
  g_opts4.SetScope(g_scope4);
}

//////////////////////////////////////////////////
/// \brief Check all the methods of the TopicStorage helper class.
TEST(TopicStorageTest, TopicStorageAPI)
{
  init();

  std::map<std::string, std::vector<Publisher>> m;
  Publisher info;
  TopicStorage<Publisher> test;

  // Check HasTopic.
  EXPECT_FALSE(test.HasTopic(g_topic1));

  // Check HasAnyPublishers.
  EXPECT_FALSE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  // Check HasPublisher.
  EXPECT_FALSE(test.HasPublisher(g_addr1));
  // Check Publisher.
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  // Check Publishers.
  EXPECT_FALSE(test.Publishers(g_topic1, m));
  // Try to remove a entry not stored yet.
  EXPECT_FALSE(test.DelPublisherByNode(g_topic1, g_pUuid2, g_nUuid4));
  // Try to remove a set of entries within a process not stored.
  EXPECT_FALSE(test.DelPublishersByProc(g_pUuid1));

  // Insert one node address.
  Publisher publisher1(g_topic1, g_addr1, g_pUuid1, g_nUuid1, g_opts1);
  EXPECT_TRUE(test.AddPublisher(publisher1));

  // Insert an existing publisher.
  EXPECT_FALSE(test.AddPublisher(publisher1));

  // Check HasTopic.
  EXPECT_TRUE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));

  // Check HasAnyPublishers.
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_FALSE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_TRUE(test.HasPublisher(g_addr1));
  EXPECT_FALSE(test.HasPublisher(g_addr2));
  // Check Publisher.
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid1);
  EXPECT_EQ(info.Options().Scope(), g_scope1);
  EXPECT_FALSE(test.Publisher(g_topic1, "wrong pUuid", g_nUuid1, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid1, "wrong nUuid", info));
  // Check Publishers.
  EXPECT_TRUE(test.Publishers(g_topic1, m));
  EXPECT_EQ(m.size(), 1u);
  EXPECT_EQ(m.begin()->first, g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(0).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).NUuid(), g_nUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Options().Scope(), g_scope1);

  // Insert one node address on the same process.
  Publisher publisher2(g_topic1, g_addr1, g_pUuid1, g_nUuid2, g_opts2);
  EXPECT_TRUE(test.AddPublisher(publisher2));
  EXPECT_FALSE(test.AddPublisher(publisher2));
  // Check HasTopic.
  EXPECT_TRUE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));
  // Check HasAnyPublishers.
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_FALSE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_TRUE(test.HasPublisher(g_addr1));
  EXPECT_FALSE(test.HasPublisher(g_addr2));
  EXPECT_FALSE(test.Publisher(g_topic1, "wrong pUuid", g_nUuid2, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid1, "wrong nUuid", info));
  // Check Publisher.
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid1);
  EXPECT_EQ(info.Options().Scope(), g_scope1);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid2, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid2);
  EXPECT_EQ(info.Options().Scope(), g_scope2);
  // Check Publishers.
  EXPECT_TRUE(test.Publishers(g_topic1, m));
  EXPECT_EQ(m.size(), 1u);
  EXPECT_EQ(m.begin()->first, g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(0).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).NUuid(), g_nUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Options().Scope(), g_scope1);
  EXPECT_EQ(m[g_pUuid1].at(1).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(1).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(1).NUuid(), g_nUuid2);
  EXPECT_EQ(m[g_pUuid1].at(1).Options().Scope(), g_scope2);

  // Insert a node address on a second process.
  Publisher publisher3(g_topic1, g_addr2, g_pUuid2, g_nUuid3, g_opts3);
  EXPECT_TRUE(test.AddPublisher(publisher3));
  EXPECT_FALSE(test.AddPublisher(publisher3));
  // Check HasTopic.
  EXPECT_TRUE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));
  // Check HasAnyPublishers.
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_TRUE(test.HasPublisher(g_addr1));
  EXPECT_TRUE(test.HasPublisher(g_addr2));
  // Check Publisher.
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid1);
  EXPECT_EQ(info.Options().Scope(), g_scope1);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid2, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid2);
  EXPECT_EQ(info.Options().Scope(), g_scope2);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid2, g_nUuid3, info));
  EXPECT_EQ(info.Addr(), g_addr2);
  EXPECT_EQ(info.PUuid(), g_pUuid2);
  EXPECT_EQ(info.NUuid(), g_nUuid3);
  EXPECT_EQ(info.Options().Scope(), g_scope3);
  EXPECT_FALSE(test.Publisher(g_topic1, "wrong pUuid", g_nUuid3, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid2, "wrong nUuid", info));

  // Check Publishers.
  EXPECT_TRUE(test.Publishers(g_topic1, m));
  EXPECT_EQ(m.size(), 2u);
  EXPECT_EQ(m.begin()->first, g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(0).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).NUuid(), g_nUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Options().Scope(), g_scope1);
  EXPECT_EQ(m[g_pUuid1].at(1).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(1).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(1).NUuid(), g_nUuid2);
  EXPECT_EQ(m[g_pUuid1].at(1).Options().Scope(), g_scope2);
  EXPECT_EQ(m[g_pUuid2].at(0).Addr(), g_addr2);
  EXPECT_EQ(m[g_pUuid2].at(0).PUuid(), g_pUuid2);
  EXPECT_EQ(m[g_pUuid2].at(0).NUuid(), g_nUuid3);
  EXPECT_EQ(m[g_pUuid2].at(0).Options().Scope(), g_scope3);

  // Insert another node on process2.
  Publisher publisher4(g_topic1, g_addr2, g_pUuid2, g_nUuid4, g_opts4);
  EXPECT_TRUE(test.AddPublisher(publisher4));
  EXPECT_FALSE(test.AddPublisher(publisher4));
  // Check HasTopic.
  EXPECT_TRUE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));
  // Check HasAnyPublishers.
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_TRUE(test.HasPublisher(g_addr1));
  EXPECT_TRUE(test.HasPublisher(g_addr2));
  // Check Publisher.
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid1);
  EXPECT_EQ(info.Options().Scope(), g_scope1);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid2, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid2);
  EXPECT_EQ(info.Options().Scope(), g_scope2);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid2, g_nUuid3, info));
  EXPECT_EQ(info.Addr(), g_addr2);
  EXPECT_EQ(info.PUuid(), g_pUuid2);
  EXPECT_EQ(info.NUuid(), g_nUuid3);
  EXPECT_EQ(info.Options().Scope(), g_scope3);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid2, g_nUuid4, info));
  EXPECT_EQ(info.Addr(), g_addr2);
  EXPECT_EQ(info.PUuid(), g_pUuid2);
  EXPECT_EQ(info.NUuid(), g_nUuid4);
  EXPECT_EQ(info.Options().Scope(), g_scope4);
  EXPECT_FALSE(test.Publisher(g_topic1, "wrong pUuid", g_nUuid4, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid2, "wrong nUuid", info));
  // Check Publishers.
  EXPECT_TRUE(test.Publishers(g_topic1, m));
  EXPECT_EQ(m.size(), 2u);
  EXPECT_EQ(m.begin()->first, g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(0).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).NUuid(), g_nUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Options().Scope(), g_scope1);
  EXPECT_EQ(m[g_pUuid1].at(1).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(1).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(1).NUuid(), g_nUuid2);
  EXPECT_EQ(m[g_pUuid1].at(1).Options().Scope(), g_scope2);
  EXPECT_EQ(m[g_pUuid2].at(0).Addr(), g_addr2);
  EXPECT_EQ(m[g_pUuid2].at(0).PUuid(), g_pUuid2);
  EXPECT_EQ(m[g_pUuid2].at(0).NUuid(), g_nUuid3);
  EXPECT_EQ(m[g_pUuid2].at(0).Options().Scope(), g_scope3);
  EXPECT_EQ(m[g_pUuid2].at(1).Addr(), g_addr2);
  EXPECT_EQ(m[g_pUuid2].at(1).PUuid(), g_pUuid2);
  EXPECT_EQ(m[g_pUuid2].at(1).NUuid(), g_nUuid4);
  EXPECT_EQ(m[g_pUuid2].at(1).Options().Scope(), g_scope4);

  // Check that Print() does not break anything.
  test.Print();

  // Remove the node4's address advertised for topic.
  EXPECT_TRUE(test.DelPublisherByNode(g_topic1, g_pUuid2, g_nUuid4));
  // Check HasTopic.
  EXPECT_TRUE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));
  // Check HasAnyPublishers.
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_TRUE(test.HasPublisher(g_addr1));
  EXPECT_TRUE(test.HasPublisher(g_addr2));
  // Check Publisher.
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid1);
  EXPECT_EQ(info.Options().Scope(), g_scope1);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid2, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid2);
  EXPECT_EQ(info.Options().Scope(), g_scope2);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid2, g_nUuid3, info));
  EXPECT_EQ(info.Addr(), g_addr2);
  EXPECT_EQ(info.PUuid(), g_pUuid2);
  EXPECT_EQ(info.NUuid(), g_nUuid3);
  EXPECT_EQ(info.Options().Scope(), g_scope3);
  // Check Publishers.
  EXPECT_TRUE(test.Publishers(g_topic1, m));
  EXPECT_EQ(m.size(), 2u);
  EXPECT_EQ(m.begin()->first, g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(0).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).NUuid(), g_nUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Options().Scope(), g_scope1);
  EXPECT_EQ(m[g_pUuid1].at(1).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(1).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(1).NUuid(), g_nUuid2);
  EXPECT_EQ(m[g_pUuid1].at(1).Options().Scope(), g_scope2);
  EXPECT_EQ(m[g_pUuid2].at(0).Addr(), g_addr2);
  EXPECT_EQ(m[g_pUuid2].at(0).PUuid(), g_pUuid2);
  EXPECT_EQ(m[g_pUuid2].at(0).NUuid(), g_nUuid3);
  EXPECT_EQ(m[g_pUuid2].at(0).Options().Scope(), g_scope3);

  // Remove the node3's address advertised for topic.
  EXPECT_TRUE(test.DelPublisherByNode(g_topic1, g_pUuid2, g_nUuid3));
  // Check HasTopic.
  EXPECT_TRUE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));
  // Check HasAnyPublishers.
  EXPECT_TRUE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_FALSE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_TRUE(test.HasPublisher(g_addr1));
  EXPECT_FALSE(test.HasPublisher(g_addr2));
  // Check Publisher.
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid1);
  EXPECT_EQ(info.Options().Scope(), g_scope1);
  EXPECT_TRUE(test.Publisher(g_topic1, g_pUuid1, g_nUuid2, info));
  EXPECT_EQ(info.Addr(), g_addr1);
  EXPECT_EQ(info.PUuid(), g_pUuid1);
  EXPECT_EQ(info.NUuid(), g_nUuid2);
  EXPECT_EQ(info.Options().Scope(), g_scope2);
  // Check Publishers.
  EXPECT_TRUE(test.Publishers(g_topic1, m));
  EXPECT_EQ(m.size(), 1u);
  EXPECT_EQ(m.begin()->first, g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(0).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).NUuid(), g_nUuid1);
  EXPECT_EQ(m[g_pUuid1].at(0).Options().Scope(), g_scope1);
  EXPECT_EQ(m[g_pUuid1].at(1).Addr(), g_addr1);
  EXPECT_EQ(m[g_pUuid1].at(1).PUuid(), g_pUuid1);
  EXPECT_EQ(m[g_pUuid1].at(1).NUuid(), g_nUuid2);
  EXPECT_EQ(m[g_pUuid1].at(1).Options().Scope(), g_scope2);

  // Insert another publisher on g_topic2.
  Publisher publisher9(g_topic2, g_addr2, g_pUuid2, g_nUuid2, g_opts2);
  EXPECT_TRUE(test.AddPublisher(publisher9));

  // Remove all the addresses of process1.
  EXPECT_TRUE(test.DelPublishersByProc(g_pUuid1));

  EXPECT_TRUE(test.DelPublisherByNode(g_topic2, g_pUuid2, g_nUuid2));

  // Check HasTopic.
  EXPECT_FALSE(test.HasTopic(g_topic1));
  EXPECT_FALSE(test.HasTopic("Unknown topic"));
  // Check HasAnyPublishers.
  EXPECT_FALSE(test.HasAnyPublishers(g_topic1, g_pUuid1));
  EXPECT_FALSE(test.HasAnyPublishers(g_topic1, g_pUuid2));
  // Check HasPublishers.
  EXPECT_FALSE(test.HasPublisher(g_addr1));
  EXPECT_FALSE(test.HasPublisher(g_addr2));
  // Check Publisher.
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid1, g_nUuid1, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid1, g_nUuid2, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid2, g_nUuid3, info));
  EXPECT_FALSE(test.Publisher(g_topic1, g_pUuid2, g_nUuid4, info));
  // Check Publishers.
  EXPECT_FALSE(test.Publishers(g_topic1, m));

  // Insert a topic, remove it, and check that the map is empty.
  Publisher publisher5(g_topic1, g_addr1, g_pUuid1, g_nUuid1, g_opts1);
  EXPECT_TRUE(test.AddPublisher(publisher5));
  EXPECT_TRUE(test.DelPublisherByNode(g_topic1, g_pUuid1, g_nUuid1));
  EXPECT_FALSE(test.HasTopic(g_topic1));

  // Insert some topics, and remove all the topics from a process but keeping
  // the same topics from other processes.
  Publisher publisher6(g_topic1, g_addr1, g_pUuid1, g_nUuid1, g_opts1);
  Publisher publisher7(g_topic1, g_addr1, g_pUuid1, g_nUuid2, g_opts2);
  Publisher publisher8(g_topic1, g_addr2, g_pUuid2, g_nUuid3, g_opts3);
  EXPECT_TRUE(test.AddPublisher(publisher6));
  EXPECT_TRUE(test.AddPublisher(publisher7));
  EXPECT_TRUE(test.AddPublisher(publisher8));
  EXPECT_TRUE(test.DelPublishersByProc(g_pUuid1));
}

//////////////////////////////////////////////////
/// \brief Check TopicList().
TEST(TopicStorageTest, TopicList)
{
  init();

  Publisher publisher1(g_topic1, g_addr1, g_pUuid1, g_nUuid1, g_opts1);
  Publisher publisher2(g_topic1, g_addr1, g_pUuid1, g_nUuid2, g_opts2);
  Publisher publisher3(g_topic2, g_addr2, g_pUuid2, g_nUuid3, g_opts3);

  TopicStorage<Publisher> test;

  EXPECT_TRUE(test.AddPublisher(publisher1));
  EXPECT_TRUE(test.AddPublisher(publisher2));
  EXPECT_TRUE(test.AddPublisher(publisher3));

  std::vector<std::string> topics;
  test.TopicList(topics);

  EXPECT_EQ(2u, topics.size());
  EXPECT_TRUE(std::find(topics.begin(), topics.end(), g_topic1) !=
    topics.end());
  EXPECT_TRUE(std::find(topics.begin(), topics.end(), g_topic2) !=
    topics.end());

  test.Clear();
  topics.clear();
  test.TopicList(topics);
  EXPECT_EQ(0u, topics.size());
}

//////////////////////////////////////////////////
/// \brief Check PublishersByProc().
TEST(TopicStorageTest, PublishersByProc)
{
  init();

  Publisher publisher1(g_topic1, g_addr1, g_pUuid1, g_nUuid1, g_opts1);
  Publisher publisher2(g_topic1, g_addr1, g_pUuid1, g_nUuid2, g_opts2);
  Publisher publisher3(g_topic1, g_addr2, g_pUuid2, g_nUuid3, g_opts3);

  TopicStorage<Publisher> test;

  EXPECT_TRUE(test.AddPublisher(publisher1));
  EXPECT_TRUE(test.AddPublisher(publisher2));
  EXPECT_TRUE(test.AddPublisher(publisher3));

  // Checking a PUUID that does not exist.
  std::map<std::string, std::vector<Publisher>> pubs;
  test.PublishersByProc("unknown_puuid", pubs);
  EXPECT_TRUE(pubs.empty());

  // Checking an existent PUUID with multiple publishers.
  test.PublishersByProc(g_pUuid1, pubs);
  EXPECT_EQ(pubs.size(), 2u);
  ASSERT_TRUE(pubs.find(g_nUuid1) != pubs.end());
  EXPECT_EQ(pubs[g_nUuid1].at(0).Addr(), g_addr1);
  ASSERT_TRUE(pubs.find(g_nUuid2) != pubs.end());
  EXPECT_EQ(pubs[g_nUuid2].at(0).Addr(), g_addr1);
}

//////////////////////////////////////////////////
/// \brief Check PublishersByNode().
TEST(TopicStorageTest, PublishersByNode)
{
  init();

  Publisher publisher1(g_topic1, g_addr1, g_pUuid1, g_nUuid1, g_opts1);
  Publisher publisher2(g_topic1, g_addr1, g_pUuid1, g_nUuid2, g_opts2);
  Publisher publisher3(g_topic1, g_addr2, g_pUuid2, g_nUuid3, g_opts3);

  TopicStorage<Publisher> test;

  EXPECT_TRUE(test.AddPublisher(publisher1));
  EXPECT_TRUE(test.AddPublisher(publisher2));
  EXPECT_TRUE(test.AddPublisher(publisher3));

  // Checking a PUUID that does not exist.
  std::vector<Publisher> pubs;
  test.PublishersByNode("unknown_puuid", g_nUuid1, pubs);
  EXPECT_TRUE(pubs.empty());

  // Checking a NUUID that does not exist.
  test.PublishersByNode(g_pUuid1, "unknown_nuuid", pubs);
  EXPECT_TRUE(pubs.empty());

  // Checking an existent NUUID with multiple publishers.
  test.PublishersByNode(g_pUuid1, g_nUuid1, pubs);
  EXPECT_EQ(pubs.size(), 1u);
  EXPECT_EQ(pubs.at(0).Addr(), g_addr1);
}

//////////////////////////////////////////////////
/// \brief Check HasTopic(<topic>, <type>).
TEST(TopicStorageTest, HasTopicWithType)
{
  init();

  std::string ctrl = "ctrl_address";
  MessagePublisher publisher1(g_topic1, g_addr1, ctrl, g_pUuid1, g_nUuid1,
    "type1", AdvertiseMessageOptions());
  MessagePublisher publisher2(g_topic1, g_addr1, ctrl, g_pUuid1, g_nUuid2,
    "type2", AdvertiseMessageOptions());

  TopicStorage<MessagePublisher> test;

  EXPECT_FALSE(test.HasTopic(g_topic1));

  EXPECT_TRUE(test.AddPublisher(publisher1));
  EXPECT_FALSE(test.HasTopic(g_topic1, "type2"));

  EXPECT_TRUE(test.AddPublisher(publisher2));
  EXPECT_TRUE(test.HasTopic(g_topic1));
}
