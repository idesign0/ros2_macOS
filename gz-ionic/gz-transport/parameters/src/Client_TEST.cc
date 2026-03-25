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

#include "gz/transport/parameters/Client.hh"
#include "gz/transport/parameters/Registry.hh"

#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/stringmsg.pb.h>

#include "gtest/gtest.h"

using namespace gz;
using namespace transport;
using namespace transport::parameters;

class ParametersClientTest : public ::testing::Test {
 protected:
  void SetUp() override {
    registry_.DeclareParameter("parameter1", std::make_unique<msgs::Boolean>());
    registry_.DeclareParameter(
      "parameter2", std::make_unique<msgs::StringMsg>());
    auto anotherStrMsg = std::make_unique<msgs::StringMsg>();
    anotherStrMsg->set_data("asd");
    registry_.DeclareParameter("parameter3", std::move(anotherStrMsg));
    registry_other_ns_.DeclareParameter(
      "another_param1", std::make_unique<msgs::Boolean>());
    anotherStrMsg = std::make_unique<msgs::StringMsg>();
    anotherStrMsg->set_data("bsd");
    registry_other_ns_.DeclareParameter(
      "another_param2", std::move(anotherStrMsg));
  }

  void TearDown() override {}

  ParametersRegistry registry_{""};
  ParametersRegistry registry_other_ns_{"/ns"};
};

//////////////////////////////////////////////////
TEST(ParametersClient, ConstructDestruct)
{
  ParametersClient client;
}

//////////////////////////////////////////////////
TEST_F(ParametersClientTest, Parameter)
{
  {
    ParametersClient client;
    {
      msgs::Boolean msg;
      EXPECT_TRUE(client.Parameter("parameter1", msg));
      EXPECT_EQ(msg.data(), false);
    }
    {
      std::unique_ptr<google::protobuf::Message> msg;
      EXPECT_TRUE(client.Parameter("parameter2", msg));
      ASSERT_NE(nullptr, msg);
      auto downcastedMsg = dynamic_cast<msgs::StringMsg *>(msg.get());
      EXPECT_EQ(downcastedMsg->data(), "");
    }
    {
      msgs::Boolean msg;
      auto ret = client.Parameter("parameter3", msg);
      EXPECT_FALSE(ret);
      EXPECT_EQ(ret.ResultType(), ParameterResultType::InvalidType);
      EXPECT_EQ(ret.ParamName(), "parameter3");
    }
  }
  {
    ParametersClient client{"/ns"};
    {
      msgs::Boolean msg;
      EXPECT_TRUE(client.Parameter("another_param1", msg));
      EXPECT_EQ(msg.data(), false);
    }
    {
      msgs::StringMsg msg;
      EXPECT_TRUE(client.Parameter("another_param2", msg));
      EXPECT_EQ(msg.data(), "bsd");
    }
  }
}

//////////////////////////////////////////////////
TEST_F(ParametersClientTest, SetParameter)
{
  {
    ParametersClient client;
    msgs::StringMsg msg;
    msg.set_data("testing");
    client.SetParameter("parameter2", msg);
    msgs::StringMsg msg_got;
    EXPECT_TRUE(registry_.Parameter("parameter2", msg_got));
    EXPECT_EQ(msg_got.data(), "testing");
  }
  {
    ParametersClient client;
    msgs::Boolean msg;
    auto ret = client.SetParameter("parameter2", msg);
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.ResultType(), ParameterResultType::InvalidType);
    EXPECT_EQ(ret.ParamName(), "parameter2");
  }
  {
    ParametersClient client;
    msgs::Boolean msg;
    auto ret = client.SetParameter("parameter_doesnt_exist", msg);
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.ResultType(), ParameterResultType::NotDeclared);
    EXPECT_EQ(ret.ParamName(), "parameter_doesnt_exist");
  }
  {
    ParametersClient client{"/ns"};
    msgs::Boolean msg;
    msg.set_data(true);
    client.SetParameter("another_param1", msg);
    msgs::Boolean msg_got;
    EXPECT_TRUE(registry_other_ns_.Parameter("another_param1", msg_got));
    EXPECT_EQ(msg_got.data(), true);
  }
}

//////////////////////////////////////////////////
TEST_F(ParametersClientTest, DeclareParameter)
{
  {
    ParametersClient client;
    msgs::StringMsg msg;
    msg.set_data("declaring");
    client.DeclareParameter("new_parameter", msg);
    msgs::StringMsg msg_got;
    EXPECT_TRUE(registry_.Parameter("new_parameter", msg_got));
    EXPECT_EQ(msg_got.data(), "declaring");
  }
  {
    ParametersClient client;
    msgs::Boolean msg;
    auto ret = client.DeclareParameter("parameter1", msg);
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.ResultType(), ParameterResultType::AlreadyDeclared);
  }
  {
    ParametersClient client{"/ns"};
    msgs::Boolean msg;
    msg.set_data(true);
    msgs::Boolean msg_got;
    client.DeclareParameter("new_parameter", msg);
    EXPECT_TRUE(registry_other_ns_.Parameter("new_parameter", msg_got));
    EXPECT_EQ(msg_got.data(), true);
  }
}

//////////////////////////////////////////////////
TEST_F(ParametersClientTest, ListParameters)
{
  {
    ParametersClient client;
    auto declarations = client.ListParameters();
    bool foundParam1{false};
    bool foundParam2{false};
    bool foundParam3{false};
    for (auto decl : declarations.parameter_declarations()) {
      if (decl.name() == "parameter1" && decl.type() == "gz_msgs.Boolean") {
        foundParam1 = true;
      }
      if (decl.name() == "parameter2" && decl.type() == "gz_msgs.StringMsg") {
        foundParam2 = true;
      }
      if (decl.name() == "parameter3" && decl.type() == "gz_msgs.StringMsg") {
        foundParam3 = true;
      }
    }
    EXPECT_TRUE(foundParam1) << "expected to find declaration for parameter1";
    EXPECT_TRUE(foundParam2) << "expected to find declaration for parameter2";
    EXPECT_TRUE(foundParam3) << "expected to find declaration for parameter3";
  }
  {
    ParametersClient client{"/ns"};
    auto declarations = client.ListParameters();
    bool foundParam1{false};
    bool foundParam2{false};
    for (auto decl : declarations.parameter_declarations()) {
      if (
        decl.name() == "another_param1" && decl.type() == "gz_msgs.Boolean")
      {
        foundParam1 = true;
      }
      if (
        decl.name() == "another_param2" && decl.type() == "gz_msgs.StringMsg")
      {
        foundParam2 = true;
      }
    }
    EXPECT_TRUE(foundParam1)
      << "expected to find declaration for another_param1";
    EXPECT_TRUE(foundParam2)
      << "expected to find declaration for another_param2";
  }
}
