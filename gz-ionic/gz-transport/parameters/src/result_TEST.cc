/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include "gz/transport/parameters/result.hh"

#include "gtest/gtest.h"
using namespace gz;
using namespace transport;
using namespace transport::parameters;

//////////////////////////////////////////////////
TEST(ParameterResult, ConstructorsAndGetters)
{
  {
    ParameterResult res{ParameterResultType::Success};
    EXPECT_EQ(res.ResultType(), ParameterResultType::Success);
    EXPECT_EQ(res.ParamName(), "");
    EXPECT_EQ(res.ParamType(), "");
  }
  {
    ParameterResult res{ParameterResultType::InvalidType, "bsd"};
    EXPECT_EQ(res.ResultType(), ParameterResultType::InvalidType);
    EXPECT_EQ(res.ParamName(), "bsd");
    EXPECT_EQ(res.ParamType(), "");
  }
  {
    ParameterResult res{
      ParameterResultType::InvalidType, "asd", "gz_msgs.Type"};
    EXPECT_EQ(res.ResultType(), ParameterResultType::InvalidType);
    EXPECT_EQ(res.ParamName(), "asd");
    EXPECT_EQ(res.ParamType(), "gz_msgs.Type");
  }
}

//////////////////////////////////////////////////
TEST(ParameterResult, BoolCoercion)
{
  EXPECT_TRUE(ParameterResult{ParameterResultType::Success});
  EXPECT_FALSE(ParameterResult{ParameterResultType::AlreadyDeclared});
  EXPECT_FALSE(ParameterResult{ParameterResultType::InvalidType});
  EXPECT_FALSE(ParameterResult{ParameterResultType::NotDeclared});
  EXPECT_FALSE(ParameterResult{ParameterResultType::ClientTimeout});
  EXPECT_FALSE(ParameterResult{ParameterResultType::Unexpected});
}

//   /// \brief Stream operator, for debug output.
//   std::ostream & operator<<(std::ostream &, const ParameterResult &);
// }

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
