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

#include <chrono>

#include "gz/transport/log/QualifiedTime.hh"
#include "gtest/gtest.h"

using namespace gz::transport;
using namespace std::chrono_literals;

//////////////////////////////////////////////////
TEST(QualifiedTime, DefaultIndeterminate)
{
  log::QualifiedTime qt;
  EXPECT_TRUE(qt.IsIndeterminate());
  EXPECT_EQ(nullptr, qt.GetQualifier());
  EXPECT_EQ(nullptr, qt.GetTime());
}


//////////////////////////////////////////////////
TEST(QualifiedTime, TimeOnlyConstructor)
{
  log::QualifiedTime qt(24h);
  EXPECT_FALSE(qt.IsIndeterminate());
  ASSERT_NE(nullptr, qt.GetTime());
  EXPECT_EQ(24h, *qt.GetTime());
  ASSERT_NE(nullptr, qt.GetQualifier());
  EXPECT_EQ(log::QualifiedTime::Qualifier::INCLUSIVE, *qt.GetQualifier());
}

//////////////////////////////////////////////////
TEST(QualifiedTime, TimeAndQualifierConstructor)
{
  {
    log::QualifiedTime qt(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
    EXPECT_FALSE(qt.IsIndeterminate());
    ASSERT_NE(nullptr, qt.GetTime());
    EXPECT_EQ(24h, *qt.GetTime());
    ASSERT_NE(nullptr, qt.GetQualifier());
    EXPECT_EQ(log::QualifiedTime::Qualifier::EXCLUSIVE, *qt.GetQualifier());
  }
  {
    log::QualifiedTime qt(24h, log::QualifiedTime::Qualifier::INCLUSIVE);
    EXPECT_FALSE(qt.IsIndeterminate());
    ASSERT_NE(nullptr, qt.GetTime());
    EXPECT_EQ(24h, *qt.GetTime());
    ASSERT_NE(nullptr, qt.GetQualifier());
    EXPECT_EQ(log::QualifiedTime::Qualifier::INCLUSIVE, *qt.GetQualifier());
  }
}

//////////////////////////////////////////////////
TEST(QualifiedTime, CopyAssignment)
{
  log::QualifiedTime qt1(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt2 = qt1;
  EXPECT_EQ(qt1, qt2);

  ASSERT_NE(nullptr, qt2.GetTime());
  EXPECT_EQ(24h, *qt2.GetTime());
  ASSERT_NE(nullptr, qt2.GetQualifier());
  EXPECT_EQ(log::QualifiedTime::Qualifier::EXCLUSIVE, *qt2.GetQualifier());
}

//////////////////////////////////////////////////
TEST(QualifiedTime, CopyConstructor)
{
  log::QualifiedTime qt1(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt2(qt1);
  EXPECT_EQ(qt1, qt2);

  ASSERT_NE(nullptr, qt2.GetTime());
  EXPECT_EQ(24h, *qt2.GetTime());
  ASSERT_NE(nullptr, qt2.GetQualifier());
  EXPECT_EQ(log::QualifiedTime::Qualifier::EXCLUSIVE, *qt2.GetQualifier());
}

//////////////////////////////////////////////////
TEST(QualifiedTime, EqualityOperators)
{
  log::QualifiedTime qt1(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt2(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt3(48h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt4(24h, log::QualifiedTime::Qualifier::INCLUSIVE);
  log::QualifiedTime qt5;

  EXPECT_TRUE(qt1 == qt2);
  EXPECT_FALSE(qt1 == qt3);
  EXPECT_FALSE(qt1 == qt4);
  EXPECT_FALSE(qt1 == qt5);
  EXPECT_FALSE(qt5 == qt5);

  EXPECT_FALSE(qt1 != qt2);
  EXPECT_TRUE(qt1 != qt3);
  EXPECT_TRUE(qt1 != qt4);
  EXPECT_TRUE(qt1 != qt5);
  EXPECT_TRUE(qt5 != qt5);
}

//////////////////////////////////////////////////
TEST(QualifiedTime, SetTime)
{
  log::QualifiedTime qt;
  qt.SetTime(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  EXPECT_FALSE(qt.IsIndeterminate());
  ASSERT_NE(nullptr, qt.GetTime());
  EXPECT_EQ(24h, *qt.GetTime());
  ASSERT_NE(nullptr, qt.GetQualifier());
  EXPECT_EQ(log::QualifiedTime::Qualifier::EXCLUSIVE, *qt.GetQualifier());
}

//////////////////////////////////////////////////
TEST(QualifiedTime, ClearTime)
{
  log::QualifiedTime qt(24h);
  EXPECT_FALSE(qt.IsIndeterminate());
  qt.Clear();
  EXPECT_TRUE(qt.IsIndeterminate());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, ConstructWithBeginAndEnd)
{
  log::QualifiedTimeRange range(24h, 48h);
  EXPECT_TRUE(range.Valid());
  auto start = range.Beginning();
  auto end = range.Ending();
  ASSERT_NE(nullptr, start.GetTime());
  EXPECT_EQ(24h, *start.GetTime());
  ASSERT_NE(nullptr, end.GetTime());
  EXPECT_EQ(48h, *end.GetTime());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, SetRange)
{
  log::QualifiedTimeRange range(24h, 48h);
  range.SetRange(72h, 120h);
  EXPECT_TRUE(range.Valid());
  auto start = range.Beginning();
  auto end = range.Ending();
  ASSERT_NE(nullptr, start.GetTime());
  EXPECT_EQ(72h, *start.GetTime());
  ASSERT_NE(nullptr, end.GetTime());
  EXPECT_EQ(120h, *end.GetTime());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, SetBeginning)
{
  log::QualifiedTimeRange range(24h, 48h);
  range.SetBeginning(32h);
  EXPECT_TRUE(range.Valid());
  auto start = range.Beginning();
  ASSERT_NE(nullptr, start.GetTime());
  EXPECT_EQ(32h, *start.GetTime());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, SetEnding)
{
  log::QualifiedTimeRange range(24h, 48h);
  range.SetEnding(32h);
  EXPECT_TRUE(range.Valid());
  auto end = range.Ending();
  ASSERT_NE(nullptr, end.GetTime());
  EXPECT_EQ(32h, *end.GetTime());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, EqualTimesValid)
{
  EXPECT_TRUE(log::QualifiedTimeRange(24h, 24h).Valid());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, CopyConstructor)
{
  log::QualifiedTimeRange range(24h, 48h);
  log::QualifiedTimeRange dest_range(range);
  EXPECT_TRUE(dest_range.Valid());
  auto start = dest_range.Beginning();
  auto end = dest_range.Ending();
  ASSERT_NE(nullptr, start.GetTime());
  EXPECT_EQ(24h, *start.GetTime());
  ASSERT_NE(nullptr, end.GetTime());
  EXPECT_EQ(48h, *end.GetTime());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, CopyAssignment)
{
  log::QualifiedTimeRange range(24h, 48h);
  log::QualifiedTimeRange dest_range = range;
  EXPECT_TRUE(dest_range.Valid());
  auto start = dest_range.Beginning();
  auto end = dest_range.Ending();
  ASSERT_NE(nullptr, start.GetTime());
  EXPECT_EQ(24h, *start.GetTime());
  ASSERT_NE(nullptr, end.GetTime());
  EXPECT_EQ(48h, *end.GetTime());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, AllTime)
{
  EXPECT_TRUE(log::QualifiedTimeRange::AllTime().Valid());
  EXPECT_TRUE(log::QualifiedTimeRange::AllTime().Beginning().IsIndeterminate());
  EXPECT_TRUE(log::QualifiedTimeRange::AllTime().Ending().IsIndeterminate());
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, From)
{
  auto uut = log::QualifiedTimeRange::From(24h);
  EXPECT_TRUE(uut.Valid());
  EXPECT_TRUE(uut.Ending().IsIndeterminate());
  ASSERT_NE(nullptr, uut.Beginning().GetTime());
  EXPECT_EQ(24h, *(uut.Beginning().GetTime()));
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, Until)
{
  auto uut = log::QualifiedTimeRange::Until(24h);
  EXPECT_TRUE(uut.Valid());
  EXPECT_TRUE(uut.Beginning().IsIndeterminate());
  ASSERT_NE(nullptr, uut.Ending().GetTime());
  EXPECT_EQ(24h, *(uut.Ending().GetTime()));
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, EqualityOperators)
{
  log::QualifiedTime qt1(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt2(24h, log::QualifiedTime::Qualifier::INCLUSIVE);

  EXPECT_TRUE(
    log::QualifiedTimeRange(qt1, qt2) == log::QualifiedTimeRange(qt1, qt2));
  EXPECT_FALSE(
    log::QualifiedTimeRange(qt1, qt2) != log::QualifiedTimeRange(qt1, qt2));

  EXPECT_FALSE(
    log::QualifiedTimeRange(qt1, qt2) == log::QualifiedTimeRange(qt2, qt1));
  EXPECT_TRUE(
    log::QualifiedTimeRange(qt1, qt2) != log::QualifiedTimeRange(qt2, qt1));

  EXPECT_FALSE(
    log::QualifiedTimeRange(qt1, qt2) == log::QualifiedTimeRange(qt2, qt2));
  EXPECT_TRUE(
    log::QualifiedTimeRange(qt1, qt2) != log::QualifiedTimeRange(qt2, qt2));
}

//////////////////////////////////////////////////
TEST(QualifiedTimeRange, AssignmentOperator)
{
  log::QualifiedTime qt1(24h, log::QualifiedTime::Qualifier::EXCLUSIVE);
  log::QualifiedTime qt2;
  log::QualifiedTimeRange range1(qt1, qt2);
  log::QualifiedTimeRange range2(qt2, qt1);

  EXPECT_FALSE(range1 == range2);
  range2 = range1;
  EXPECT_TRUE(range1 == range2);
}
