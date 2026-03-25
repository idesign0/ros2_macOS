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

#include <cctype>
#include <iostream>
#include <string>

#include "gz/common/Uuid.hh"
#include "gtest/gtest.h"

using namespace gz;

//////////////////////////////////////////////////
/// \brief Check the Uuid helper class.
TEST(UuidTest, testToString)
{
  common::Uuid uuid1;
  common::Uuid uuid2;

  // Two UUIDs should always be different.
  EXPECT_NE(uuid1.String(), uuid2.String());

  std::ostringstream output;
  output  << uuid1;
  for (auto i = 0; i < 8; ++i)
    EXPECT_GT(isxdigit(output.str()[i]), 0);
  EXPECT_EQ(output.str()[8], '-');
  for (auto i = 9; i < 13; ++i)
    EXPECT_GT(isxdigit(output.str()[i]), 0);
  EXPECT_EQ(output.str()[13], '-');
  for (auto i = 14; i < 18; ++i)
    EXPECT_GT(isxdigit(output.str()[i]), 0);
  EXPECT_EQ(output.str()[18], '-');
  for (auto i = 19; i < 23; ++i)
    EXPECT_GT(isxdigit(output.str()[i]), 0);
  EXPECT_EQ(output.str()[23], '-');
  for (auto i = 24; i < 36; ++i)
    EXPECT_GT(isxdigit(output.str()[i]), 0);
}
