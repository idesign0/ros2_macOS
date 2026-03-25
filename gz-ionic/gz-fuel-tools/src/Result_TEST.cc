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
#include "gz/fuel_tools/Result.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
TEST(Result, DefaultConstructor)
{
  Result result;
  EXPECT_EQ(ResultType::UNKNOWN, result.Type());
}

/////////////////////////////////////////////////
TEST(Result, CopyConstructor)
{
  Result result(ResultType::UPLOAD);

  Result result2(result);
  EXPECT_EQ(ResultType::UPLOAD, result2.Type());
}

/////////////////////////////////////////////////
TEST(Result, CopyAssignmentOperator)
{
  Result result(ResultType::UPLOAD);

  Result result2;
  result2 = result;
  EXPECT_EQ(ResultType::UPLOAD, result2.Type());
}

/////////////////////////////////////////////////
TEST(ResultTest, MoveConstructor)
{
  Result result(ResultType::UPLOAD);

  Result resultMoved(std::move(result));
  EXPECT_EQ(ResultType::UPLOAD, resultMoved.Type());
}

/////////////////////////////////////////////////
TEST(ResultTest, MoveAssignmentOperator)
{
  Result result(ResultType::UPLOAD);

  Result resultMoved(ResultType::DELETE);
  resultMoved = std::move(result);
  EXPECT_EQ(ResultType::UPLOAD, resultMoved.Type());
}

/////////////////////////////////////////////////
/// \brief Create a request object with a type and check that it
TEST(Result, TypeCanBeSet)
{
  Result r1(ResultType::UPLOAD);
  EXPECT_EQ(ResultType::UPLOAD, r1.Type());

  Result r2(ResultType::DELETE);
  EXPECT_EQ(ResultType::DELETE, r2.Type());
}

/////////////////////////////////////////////////
/// \brief Check that there are readable strings returned
TEST(Result, ReadableStringsHaveLength)
{
  gz::fuel_tools::Result r(ResultType::FETCH);

  EXPECT_FALSE(Result(ResultType::DELETE).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::FETCH).ReadableResult().empty());
  EXPECT_FALSE(
      Result(ResultType::FETCH_ALREADY_EXISTS).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::UPLOAD).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::UNKNOWN).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::DELETE_NOT_FOUND).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::DELETE_ERROR).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::FETCH_NOT_FOUND).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::FETCH_ERROR).ReadableResult().empty());
  EXPECT_FALSE(
      Result(ResultType::UPLOAD_ALREADY_EXISTS).ReadableResult().empty());
  EXPECT_FALSE(Result(ResultType::UPLOAD_ERROR).ReadableResult().empty());
}

//////////////////////////////////////////////////
/// \brief Check that operator bool returns true for successful cases
TEST(Result, TrueOnSuccess)
{
  EXPECT_TRUE(Result(ResultType::DELETE));
  EXPECT_TRUE(Result(ResultType::FETCH));
  EXPECT_TRUE(Result(ResultType::FETCH_ALREADY_EXISTS));
  EXPECT_TRUE(Result(ResultType::UPLOAD));
  EXPECT_FALSE(Result(ResultType::UNKNOWN));
  EXPECT_FALSE(Result(ResultType::DELETE_NOT_FOUND));
  EXPECT_FALSE(Result(ResultType::DELETE_ERROR));
  EXPECT_FALSE(Result(ResultType::FETCH_NOT_FOUND));
  EXPECT_FALSE(Result(ResultType::FETCH_ERROR));
  EXPECT_FALSE(Result(ResultType::UPLOAD_ALREADY_EXISTS));
  EXPECT_FALSE(Result(ResultType::UPLOAD_ERROR));
}
