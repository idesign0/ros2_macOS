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

#include "gz/transport/parameters/Registry.hh"

#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/stringmsg.pb.h>

#include "gtest/gtest.h"

using namespace gz;
using namespace transport;
using namespace transport::parameters;

//////////////////////////////////////////////////
TEST(ParametersRegistry, ConstructDestruct)
{
  ParametersRegistry registry{""};
}

//////////////////////////////////////////////////
TEST(ParametersRegistry, DeclareParameter)
{
  ParametersRegistry registry{""};
  EXPECT_THROW(
    registry.DeclareParameter("will_fail", nullptr),
    std::invalid_argument);
  registry.DeclareParameter(
    "parameter1", std::make_unique<gz::msgs::Boolean>());
  auto ret = registry.DeclareParameter(
    "parameter1", std::make_unique<gz::msgs::Boolean>());
  EXPECT_FALSE(ret);
  EXPECT_EQ(ret.ResultType(), ParameterResultType::AlreadyDeclared);
  EXPECT_EQ(ret.ParamName(), "parameter1");
}

//////////////////////////////////////////////////
TEST(ParametersRegistry, Parameter)
{
  ParametersRegistry registry{""};
  gz::msgs::Boolean msg;
  auto ret = registry.Parameter("will_fail", msg);
  EXPECT_FALSE(ret);
  EXPECT_EQ(ret.ResultType(), ParameterResultType::NotDeclared);
  EXPECT_EQ(ret.ParamName(), "will_fail");
  EXPECT_TRUE(registry.DeclareParameter(
    "parameter1", std::make_unique<gz::msgs::Boolean>()));
  EXPECT_TRUE(registry.Parameter("parameter1", msg));
  EXPECT_FALSE(msg.data());
  gz::msgs::StringMsg msg2;
  ret = registry.Parameter("parameter1", msg2);
  EXPECT_FALSE(ret);
  EXPECT_EQ(ret.ResultType(), ParameterResultType::InvalidType);
  EXPECT_EQ(ret.ParamName(), "parameter1");
  EXPECT_EQ(ret.ParamType(), "gz_msgs.Boolean");
}

//////////////////////////////////////////////////
TEST(ParametersRegistry, SetParameter)
{
  ParametersRegistry registry{""};
  auto ret = registry.SetParameter(
    "will_fail", std::make_unique<gz::msgs::Boolean>());
  EXPECT_FALSE(ret);
  EXPECT_EQ(ret.ResultType(), ParameterResultType::NotDeclared);
  EXPECT_EQ(ret.ParamName(), "will_fail");
  registry.DeclareParameter(
    "parameter1", std::make_unique<gz::msgs::Boolean>());
  auto unique = std::make_unique<gz::msgs::Boolean>();
  unique->set_data(true);
  EXPECT_TRUE(registry.SetParameter("parameter1", std::move(unique)));
  gz::msgs::Boolean msg;
  EXPECT_TRUE(registry.Parameter("parameter1", msg));
  EXPECT_TRUE(msg.data());
  ret = registry.SetParameter(
    "parameter1", std::make_unique<gz::msgs::StringMsg>());
  EXPECT_FALSE(ret);
  EXPECT_EQ(ret.ResultType(), ParameterResultType::InvalidType);
  EXPECT_EQ(ret.ParamName(), "parameter1");
  EXPECT_EQ(ret.ParamType(), "gz_msgs.Boolean");
}

//////////////////////////////////////////////////
TEST(ParametersRegistry, ListParameters)
{
  ParametersRegistry registry{""};
  auto declarations = registry.ListParameters();
  EXPECT_EQ(declarations.parameter_declarations_size(), 0);
  registry.DeclareParameter(
    "parameter1", std::make_unique<gz::msgs::Boolean>());
  registry.DeclareParameter(
    "parameter2", std::make_unique<gz::msgs::StringMsg>());
  declarations = registry.ListParameters();
  EXPECT_EQ(declarations.parameter_declarations_size(), 2);
  bool foundParam1 = false;
  bool foundParam2 = false;
  for (auto decl : declarations.parameter_declarations()) {
    if (decl.name() == "parameter1" && decl.type() == "gz_msgs.Boolean") {
      foundParam1 = true;
    }
    if (decl.name() == "parameter2" && decl.type() == "gz_msgs.StringMsg") {
      foundParam2 = true;
    }
  }
  EXPECT_TRUE(foundParam1) << "expected to find declaration for parameter1";
  EXPECT_TRUE(foundParam2) << "expected to find declaration for parameter2";
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
