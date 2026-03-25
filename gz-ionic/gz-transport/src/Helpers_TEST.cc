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
#include "gtest/gtest.h"

#include "gz/transport/Helpers.hh"
#include "test_utils.hh"

#include <gz/utils/Environment.hh>

using namespace gz;

//////////////////////////////////////////////////
/// \brief Check the env() function.
TEST(HelpersTest, env)
{
  // Create a random string.
  std::string name = testing::getRandomNumber();

  // Check that an unknown environment variable returns false.
  std::string value;
  EXPECT_FALSE(transport::env(name, value));

  // Create a random environment variable and give it its name as value.
  ASSERT_TRUE(gz::utils::setenv(name, name));

  // Check that we find the environment variable and the value is correct.
  EXPECT_TRUE(transport::env(name, value));
  EXPECT_EQ(name, value);
}

/////////////////////////////////////////////////
TEST(HelpersTest, SplitNoDelimiterPresent)
{
  char delim = ':';
  std::string orig = "Hello World!";
  std::vector<std::string> pieces = transport::split(orig, delim);
  ASSERT_LT(0u, pieces.size());
  EXPECT_EQ(1u, pieces.size());
  EXPECT_EQ(orig, pieces[0]);
}

/////////////////////////////////////////////////
TEST(HelpersTest, SplitOneDelimiterInMiddle)
{
  char delim = ' ';
  std::string orig = "Hello World!";
  std::vector<std::string> pieces = transport::split(orig, delim);
  ASSERT_LT(1u, pieces.size());
  EXPECT_EQ(2u, pieces.size());
  EXPECT_EQ("Hello", pieces[0]);
  EXPECT_EQ("World!", pieces[1]);
}

/////////////////////////////////////////////////
TEST(HelpersTest, SplitOneDelimiterAtBeginning)
{
  char delim = ':';
  std::string orig = ":Hello World!";
  std::vector<std::string> pieces = transport::split(orig, delim);
  ASSERT_LT(1u, pieces.size());
  EXPECT_EQ(2u, pieces.size());
  EXPECT_EQ("", pieces[0]);
  EXPECT_EQ("Hello World!", pieces[1]);
}

/////////////////////////////////////////////////
TEST(HelpersTest, SplitOneDelimiterAtEnd)
{
  char delim = '!';
  std::string orig = "Hello World!";
  std::vector<std::string> pieces = transport::split(orig, delim);
  ASSERT_LT(1u, pieces.size());
  EXPECT_EQ(2u, pieces.size());
  EXPECT_EQ("Hello World", pieces[0]);
  EXPECT_EQ("", pieces[1]);
}
