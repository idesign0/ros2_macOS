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
#include <gtest/gtest.h>
#include <string>
#include <gz/common/Base64.hh>

#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

class Base64 : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(Base64, Encode)
{
  std::string str = "123abc";
  std::string result;
  common::Base64::Encode(str.c_str(), str.size(), result);
}

/////////////////////////////////////////////////
TEST_F(Base64, EncodeDecode)
{
  const std::string original = "-_- face";
  std::string encoded;
  common::Base64::Encode(original.c_str(), original.size(), encoded);
  EXPECT_NE(original, encoded);
  std::cerr << encoded << std::endl;
  for (unsigned char c : encoded)
  {
    if (c == '=')
      break;

    EXPECT_TRUE(isalnum(c) || (c == '+') || (c == '/')) << c;
  }
  EXPECT_EQ(original, common::Base64::Decode(encoded));
}
