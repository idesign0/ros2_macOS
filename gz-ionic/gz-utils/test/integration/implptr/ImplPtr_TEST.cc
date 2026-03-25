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

#include <gtest/gtest.h>

#include "implptr_test_classes.hh"

#include <utility>

using namespace gz::implptr_test_classes;

/////////////////////////////////////////////////
TEST(ImplPtr, CopyConstruct)
{
  CopyableObject object(28, "golden_string");
  EXPECT_EQ(28, object.GetInt());
  EXPECT_EQ("golden_string", object.GetString());

  EXPECT_NE(object.GetInt(), CopyableObject().GetInt());
  EXPECT_NE(object.GetString(), CopyableObject().GetString());

  EXPECT_EQ(object.GetInt(), CopyableObject(object).GetInt());
  EXPECT_EQ(object.GetString(), CopyableObject(object).GetString());

  CopyableObject other(object);

  EXPECT_EQ(object.GetInt(), other.GetInt());
  EXPECT_EQ(object.GetString(), other.GetString());
}

/////////////////////////////////////////////////
TEST(ImplPtr, GetAccessor)
{
  CopyableObject object(28, "golden_string");
  int value = object.ThreadIncrementInt();
  EXPECT_EQ(value, 1);
  EXPECT_EQ(object.GetInt(), 1);

  CopyableObject copied;
  copied = object;
  value = copied.ThreadIncrementInt();
  EXPECT_EQ(value, 1);
  EXPECT_EQ(copied.GetInt(), 1);

  EXPECT_EQ(copied.ThreadIncrementInt(), object.ThreadIncrementInt());
  EXPECT_EQ(copied.GetInt(), 1);
  EXPECT_EQ(object.GetInt(), 1);
}

/////////////////////////////////////////////////
TEST(ImplPtr, CopyAssign)
{
  CopyableObject object{47, "some_object"};
  EXPECT_EQ(47, object.GetInt());
  EXPECT_EQ("some_object", object.GetString());

  CopyableObject other{};

  EXPECT_NE(object.GetInt(), other.GetInt());
  EXPECT_NE(object.GetString(), other.GetString());

  other = object;
  EXPECT_EQ(object.GetInt(), other.GetInt());
  EXPECT_EQ(object.GetString(), other.GetString());
}

/////////////////////////////////////////////////
TEST(ImplPtr, MoveConstruct)
{
  CopyableObject object(64, "move this thing");
  EXPECT_EQ(64, object.GetInt());
  EXPECT_EQ("move this thing", object.GetString());

  CopyableObject copy(object);
  CopyableObject moved(std::move(object));

  EXPECT_EQ(copy.GetInt(), moved.GetInt());
  EXPECT_EQ(copy.GetString(), moved.GetString());
}

/////////////////////////////////////////////////
TEST(ImplPtr, MoveAssign)
{
  CopyableObject object(78, "we will move assign this");
  EXPECT_EQ(78, object.GetInt());
  EXPECT_EQ("we will move assign this", object.GetString());

  CopyableObject copy(object);

  EXPECT_EQ(object.GetInt(), copy.GetInt());
  EXPECT_EQ(object.GetString(), copy.GetString());

  CopyableObject moved;

  EXPECT_NE(object.GetInt(), moved.GetInt());
  EXPECT_NE(object.GetString(), moved.GetString());

  moved = std::move(object);

  EXPECT_EQ(copy.GetInt(), moved.GetInt());
  EXPECT_EQ(copy.GetString(), moved.GetString());
}

/////////////////////////////////////////////////
TEST(UniqueImplPtr, MoveConstruct)
{
  MovableObject object(101, "moving this unique thing");
  EXPECT_EQ(101, object.GetInt());
  EXPECT_EQ("moving this unique thing", object.GetString());

  MovableObject other(101, "moving this unique thing");
  EXPECT_EQ(other.GetInt(), object.GetInt());
  EXPECT_EQ(other.GetString(), object.GetString());

  MovableObject moved(std::move(object));
  EXPECT_EQ(other.GetInt(), moved.GetInt());
  EXPECT_EQ(other.GetString(), moved.GetString());
}

/////////////////////////////////////////////////
TEST(UniqueImplPtr, GetAccessor)
{
  MovableObject object(28, "golden_string");
  int value = object.ThreadIncrementInt();
  EXPECT_EQ(value, 1);
  EXPECT_EQ(object.GetInt(), 1);

  MovableObject moved;
  moved = std::move(object);
  value = moved.ThreadIncrementInt();
  EXPECT_EQ(value, 1);
  EXPECT_EQ(moved.GetInt(), 1);
}

/////////////////////////////////////////////////
TEST(UniqueImplPtr, MoveAssign)
{
  MovableObject object(117, "assigning this unique thing");
  EXPECT_EQ(117, object.GetInt());
  EXPECT_EQ("assigning this unique thing", object.GetString());

  MovableObject other(117, "assigning this unique thing");
  EXPECT_EQ(other.GetInt(), object.GetInt());
  EXPECT_EQ(other.GetString(), object.GetString());

  MovableObject moved;
  EXPECT_NE(other.GetInt(), moved.GetInt());
  EXPECT_NE(other.GetInt(), moved.GetInt());

  moved = std::move(object);

  EXPECT_EQ(other.GetInt(), moved.GetInt());
  EXPECT_EQ(other.GetString(), moved.GetString());
}
