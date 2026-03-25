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
#include <gtest/gtest.h>
#include <string>
#include <vector>


#include "gz/common/StringUtils.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(StringUtils, SplitNoDelimiterPresent)
{
  char delim = ':';
  std::string orig = "Hello World!";
  std::vector<std::string> pieces = common::Split(orig, delim);
  ASSERT_LT(0u, pieces.size());
  EXPECT_EQ(1u, pieces.size());
  EXPECT_EQ(orig, pieces[0]);
}

/////////////////////////////////////////////////
TEST(StringUtils, SplitOneDelimiterInMiddle)
{
  char delim = ' ';
  std::string orig = "Hello World!";
  std::vector<std::string> pieces = common::Split(orig, delim);
  ASSERT_LT(1u, pieces.size());
  EXPECT_EQ(2u, pieces.size());
  EXPECT_EQ("Hello", pieces[0]);
  EXPECT_EQ("World!", pieces[1]);
}

/////////////////////////////////////////////////
TEST(StringUtils, SplitOneDelimiterAtBeginning)
{
  char delim = ':';
  std::string orig = ":Hello World!";
  std::vector<std::string> pieces = common::Split(orig, delim);
  ASSERT_LT(1u, pieces.size());
  EXPECT_EQ(2u, pieces.size());
  EXPECT_EQ("", pieces[0]);
  EXPECT_EQ("Hello World!", pieces[1]);
}

/////////////////////////////////////////////////
TEST(StringUtils, SplitOneDelimiterAtEnd)
{
  char delim = '!';
  std::string orig = "Hello World!";
  std::vector<std::string> pieces = common::Split(orig, delim);
  ASSERT_LT(1u, pieces.size());
  EXPECT_EQ(2u, pieces.size());
  EXPECT_EQ("Hello World", pieces[0]);
  EXPECT_EQ("", pieces[1]);
}

/////////////////////////////////////////////////
TEST(Join, Simple)
{
  {
    std::string delim = " ";
    auto pieces = std::vector<std::string>{"Hello", "World"};
    auto joined = common::Join(pieces, delim);
    EXPECT_EQ("Hello World", joined);
  }

  {
    std::string delim = "x";
    auto pieces = std::vector<std::string>{"Hello", "World"};
    auto joined = common::Join(pieces, delim);
    EXPECT_EQ("HelloxWorld", joined);
  }

  {
    std::string delim = "x";
    auto pieces = std::vector<std::string>{
      "foo", "bar", "baz", "bing"};
    auto joined = common::Join(pieces, delim);
    EXPECT_EQ("fooxbarxbazxbing", joined);
  }
}

/////////////////////////////////////////////////
TEST(Join, EmptyDelim)
{
  std::string delim = "";
  auto pieces = std::vector<std::string>{"Hello", "World"};
  auto joined = common::Join(pieces, delim);
  EXPECT_EQ("HelloWorld", joined);
}

/////////////////////////////////////////////////
TEST(Join, EmptyPart)
{
  std::string delim = " ";
  auto pieces = std::vector<std::string>{"Hello", "", "World"};
  auto joined = common::Join(pieces, delim);
  EXPECT_EQ("Hello World", joined);
}

/////////////////////////////////////////////////
TEST(Join, EmptyParts)
{
  std::string delim = "x";
  auto pieces = std::vector<std::string>{};
  auto joined = common::Join(pieces, delim);
  EXPECT_EQ("", joined);
}

/////////////////////////////////////////////////
TEST(StartsWith, NotInString)
{
  std::string big = "Hello World!";
  std::string little = "asdf";
  EXPECT_FALSE(common::StartsWith(big, little));
}

/////////////////////////////////////////////////
TEST(StartsWith, InMiddle)
{
  std::string big = "Hello World!";
  std::string little = "ello";
  EXPECT_FALSE(common::StartsWith(big, little));
}

/////////////////////////////////////////////////
TEST(StartsWith, AtEnd)
{
  std::string big = "Hello World!";
  std::string little = "!";
  EXPECT_FALSE(common::StartsWith(big, little));
}

/////////////////////////////////////////////////
TEST(StartsWith, AtBeginning)
{
  std::string big = "Hello World!";
  std::string little = "He";
  EXPECT_TRUE(common::StartsWith(big, little));
}

/////////////////////////////////////////////////
TEST(EndsWith, NotInString)
{
  std::string big = "Hello World!";
  std::string little = "asdf";
  EXPECT_FALSE(common::EndsWith(big, little));
}

/////////////////////////////////////////////////
TEST(EndsWith, InMiddle)
{
  std::string big = "Hello World!";
  std::string little = "ello";
  EXPECT_FALSE(common::EndsWith(big, little));
}

/////////////////////////////////////////////////
TEST(EndsWith, AtEnd)
{
  std::string big = "Hello World!";
  std::string little = "!";
  EXPECT_TRUE(common::EndsWith(big, little));
}

/////////////////////////////////////////////////
TEST(EndsWith, AtBeginning)
{
  std::string big = "Hello World!";
  std::string little = "He";
  EXPECT_FALSE(common::EndsWith(big, little));
}

/////////////////////////////////////////////////
TEST(EndsWith, PluralCast)
{
  EXPECT_EQ("cows", common::PluralCast("cow", 0));
  EXPECT_EQ("cow",  common::PluralCast("cow", 1));
  EXPECT_EQ("cows", common::PluralCast("cow", 2));
  EXPECT_EQ("cows", common::PluralCast("cow", 4));

  EXPECT_EQ("cacti",  common::PluralCast("cactus", "cacti", 0));
  EXPECT_EQ("cactus", common::PluralCast("cactus", "cacti", 1));
  EXPECT_EQ("cacti",  common::PluralCast("cactus", "cacti", 2));
  EXPECT_EQ("cacti",  common::PluralCast("cactus", "cacti", 4));

  EXPECT_EQ("bear", common::PluralCast("bear", -1));
  EXPECT_EQ("bears", common::PluralCast("bear", -2));
  EXPECT_EQ("bears", common::PluralCast("bear", -3));
  EXPECT_EQ("bears", common::PluralCast("bear", -4));

  EXPECT_EQ("ox", common::PluralCast("ox", "oxen", -1));
  EXPECT_EQ("oxen", common::PluralCast("ox", "oxen", -2));
  EXPECT_EQ("oxen", common::PluralCast("ox", "oxen", -3));
  EXPECT_EQ("oxen", common::PluralCast("ox", "oxen", -4));
}
