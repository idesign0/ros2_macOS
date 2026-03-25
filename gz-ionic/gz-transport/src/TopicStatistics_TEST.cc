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

#include "gtest/gtest.h"
#include "gz/transport/TopicStatistics.hh"

using namespace gz;
using namespace transport;

//////////////////////////////////////////////////
TEST(TopicsStatistics, Constructor)
{
  Statistics stats;
  EXPECT_EQ(0u, stats.Count());
  EXPECT_DOUBLE_EQ(0.0, stats.Avg());
  EXPECT_DOUBLE_EQ(0.0, stats.StdDev());
  EXPECT_DOUBLE_EQ(std::numeric_limits<double>::max(), stats.Min());
  EXPECT_DOUBLE_EQ(std::numeric_limits<double>::min(), stats.Max());

  TopicStatistics topicStats;
  EXPECT_EQ(0u, topicStats.DroppedMsgCount());
  EXPECT_DOUBLE_EQ(0.0, topicStats.PublicationStatistics().Avg());
  EXPECT_DOUBLE_EQ(0.0, topicStats.PublicationStatistics().StdDev());
  EXPECT_DOUBLE_EQ(std::numeric_limits<double>::max(),
      topicStats.PublicationStatistics().Min());
  EXPECT_DOUBLE_EQ(std::numeric_limits<double>::min(),
      topicStats.PublicationStatistics().Max());
  EXPECT_DOUBLE_EQ(0.0, topicStats.ReceptionStatistics().Avg());
  EXPECT_DOUBLE_EQ(0.0, topicStats.ReceptionStatistics().StdDev());
  EXPECT_DOUBLE_EQ(std::numeric_limits<double>::max(),
      topicStats.ReceptionStatistics().Min());
  EXPECT_DOUBLE_EQ(std::numeric_limits<double>::min(),
      topicStats.ReceptionStatistics().Max());
}

//////////////////////////////////////////////////
TEST(TopicsStatistics, DroppedMsg)
{
  TopicStatistics topicStats;
  topicStats.Update("foo", 1, 0);
  topicStats.Update("foo", 2, 1);
  EXPECT_EQ(0u, topicStats.DroppedMsgCount());

  topicStats.Update("foo", 3, 3);
  EXPECT_EQ(1u, topicStats.DroppedMsgCount());

  topicStats.Update("foo", 4, 5);
  EXPECT_EQ(2u, topicStats.DroppedMsgCount());

  topicStats.Update("foo", 5, 6);
  EXPECT_EQ(2u, topicStats.DroppedMsgCount());
}

//////////////////////////////////////////////////
TEST(TopicsStatistics, MinMax)
{
  Statistics stats;
  stats.Update(1.0);
  EXPECT_DOUBLE_EQ(1.0, stats.Min());
  EXPECT_DOUBLE_EQ(1.0, stats.Max());

  stats.Update(2.0);
  EXPECT_DOUBLE_EQ(1.0, stats.Min());
  EXPECT_DOUBLE_EQ(2.0, stats.Max());

  stats.Update(0.1);
  EXPECT_DOUBLE_EQ(0.1, stats.Min());
  EXPECT_DOUBLE_EQ(2.0, stats.Max());
}

//////////////////////////////////////////////////
TEST(TopicsStatistics, AvgStdDev)
{
  Statistics stats;
  stats.Update(1.0);
  EXPECT_EQ(1u, stats.Count());
  EXPECT_DOUBLE_EQ(1.0, stats.Avg());
  EXPECT_DOUBLE_EQ(0.0, stats.StdDev());

  stats.Update(2.0);
  EXPECT_EQ(2u, stats.Count());
  EXPECT_DOUBLE_EQ(1.5, stats.Avg());
  EXPECT_DOUBLE_EQ(0.5, stats.StdDev());

  stats.Update(3.0);
  EXPECT_EQ(3u, stats.Count());
  EXPECT_DOUBLE_EQ(2.0, stats.Avg());
  EXPECT_NEAR(0.816, stats.StdDev(), 1e-3);
}
