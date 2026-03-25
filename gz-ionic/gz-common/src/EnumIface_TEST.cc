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

#include "gz/common/EnumIface.hh"

#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

class EnumIfaceTest : public common::testing::AutoLogFixture { };

enum MyType
{
  MY_TYPE_BEGIN = 0,
  TYPE1 = MY_TYPE_BEGIN,
  TYPE2 = 1,
  MY_TYPE_END
};

auto myTypeIface = gz::common::gzEnum(
  MY_TYPE_BEGIN, MY_TYPE_END, {
    "TYPE1",
    "TYPE2",
    "MY_TYPE_END"
});

/////////////////////////////////////////////////
TEST_F(EnumIfaceTest, StringCoversion)
{
  MyType type = MyType::TYPE2;

  // Set value from string
  myTypeIface.Set(type, "TYPE1");
  EXPECT_EQ(type, TYPE1);

  // Convert value to string
  std::string typeStr = myTypeIface.Str(type);
  EXPECT_EQ(typeStr, "TYPE1");

  EXPECT_TRUE(myTypeIface.Str(static_cast<MyType>(4)).empty());
}

/////////////////////////////////////////////////
TEST_F(EnumIfaceTest, Iterator)
{
  common::EnumIterator<MyType> end {MY_TYPE_END};
  ASSERT_EQ(end.Value(), MY_TYPE_END);

  common::EnumIterator<MyType> begin {MY_TYPE_BEGIN};
  ASSERT_EQ(*begin, MY_TYPE_BEGIN);

  int i = 0;

  // Prefix ++ operator
  for (common::EnumIterator<MyType> typeIter {MY_TYPE_BEGIN};
       typeIter != end; ++typeIter, ++i)
  {
    ASSERT_EQ(*typeIter, i);
    if (i == 0)
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE1");
    else
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE2");
  }

  // Postfix ++ operator
  i = 0;
  for (common::EnumIterator<MyType> typeIter {MY_TYPE_BEGIN};
       typeIter != end; ++typeIter, ++i)
  {
    ASSERT_EQ(typeIter.Value(), i);
    if (i == 0)
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE1");
    else
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE2");
  }

  // Prefix -- operator
  i = MY_TYPE_END - 1;
  for (common::EnumIterator<MyType> typeIter = --end;
       typeIter != begin; --typeIter, --i)
  {
    ASSERT_EQ(*typeIter, i);
    if (i == 0)
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE1");
    else
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE2");
  }

  // Postfix -- operator
  i = MY_TYPE_END - 1;
  common::EnumIterator<MyType> end2{MY_TYPE_END};
  for (common::EnumIterator<MyType> typeIter = --end2;
       typeIter != begin; --typeIter, --i)
  {
    ASSERT_EQ(*typeIter, i);
    if (i == 0)
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE1");
    else
      ASSERT_EQ(myTypeIface.Str(*typeIter), "TYPE2");
  }
}
