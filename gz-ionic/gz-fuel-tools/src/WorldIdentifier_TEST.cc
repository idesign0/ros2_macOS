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
#include <string>
#include <gz/common/Console.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
/// \brief Fields can be set
TEST(WorldIdentifier, SetFields)
{
  WorldIdentifier id;
  id.SetName("hello");
  id.SetOwner("acai");
  id.SetVersion(6);

  EXPECT_EQ(std::string("hello"), id.Name());
  EXPECT_EQ(std::string("acai"), id.Owner());
  EXPECT_EQ(6u, id.Version());

  EXPECT_FALSE(id.Private());
  id.SetPrivate(true);
  EXPECT_TRUE(id.Private());
  id.SetPrivate(false);
  EXPECT_FALSE(id.Private());
}

/////////////////////////////////////////////////
/// \brief Unique Name
TEST(WorldIdentifier, UniqueName)
{
  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(gz::common::URI("https://localhost:8001/", true));

  gz::fuel_tools::ServerConfig srv2;
  srv2.SetUrl(gz::common::URI("https://localhost:8002", true));

  gz::fuel_tools::ServerConfig srv3;
  srv3.SetUrl(gz::common::URI("https://localhost:8003/", true));

  WorldIdentifier id;
  id.SetName("hello");
  id.SetOwner("alice");

  id.SetServer(srv1);
  EXPECT_EQ("localhost%3A8001/alice/worlds/hello", id.UniqueName());

  id.SetServer(srv2);
  EXPECT_EQ("localhost%3A8002/alice/worlds/hello", id.UniqueName());

  id.SetServer(srv3);
  EXPECT_EQ("localhost%3A8003/alice/worlds/hello", id.UniqueName());
}

/////////////////////////////////////////////////
/// \brief Url
TEST(WorldIdentifier, Url)
{
  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(gz::common::URI("https://localhost:8001/", true));

  gz::fuel_tools::ServerConfig srv2;
  srv2.SetUrl(gz::common::URI("https://localhost:8002", true));

  gz::fuel_tools::ServerConfig srv3;
  srv3.SetUrl(gz::common::URI("https://localhost:8003/", true));

  WorldIdentifier id;
  id.SetName("hello");
  id.SetOwner("alice");

  id.SetServer(srv1);
  EXPECT_EQ("https://localhost:8001/alice/worlds/hello", id.Url().Str());

  id.SetServer(srv2);
  EXPECT_EQ("https://localhost:8002/alice/worlds/hello", id.Url().Str());

  id.SetServer(srv3);
  EXPECT_EQ("https://localhost:8003/alice/worlds/hello", id.Url().Str());
}

/////////////////////////////////////////////////
/// \brief Copy constructor deep copies
TEST(WorldIdentifier, CopyConstructorDeepCopy)
{
  WorldIdentifier id;
  id.SetName("hello");
  id.SetOwner("watermelon");
  EXPECT_FALSE(id.SetVersionStr("NaN"));
  EXPECT_TRUE(id.SetVersionStr(""));

  WorldIdentifier id2(id);
  EXPECT_EQ(std::string("hello"), id2.Name());
  EXPECT_EQ(std::string("watermelon"), id2.Owner());
  EXPECT_EQ("tip", id2.VersionStr());

  id2.SetName("hello2");
  EXPECT_EQ(std::string("hello"), id.Name());
  EXPECT_EQ(std::string("hello2"), id2.Name());
}

/////////////////////////////////////////////////
/// \brief assignment operator deep copies
TEST(WorldIdentifier, AssignmentOperatorDeepCopy)
{
  WorldIdentifier id;
  id.SetName("hello");
  id.SetOwner("pineapple");
  id.SetVersionStr("tip");

  WorldIdentifier id2(id);
  id2 = id;
  EXPECT_EQ(std::string("hello"), id2.Name());
  EXPECT_EQ(std::string("pineapple"), id2.Owner());
  EXPECT_EQ(0u, id2.Version());

  id2.SetName("hello2");
  EXPECT_EQ(std::string("hello"), id.Name());
  EXPECT_EQ(std::string("hello2"), id2.Name());
}

/////////////////////////////////////////////////
TEST(WorldIdentifier, AsString)
{
  common::Console::SetVerbosity(4);
  {
    WorldIdentifier id;
    std::string str =
        "Name: \n"\
        "Owner: \n"\
        "Version: tip\n"\
        "Unique name: fuel.gazebosim.org/worlds/\n"
        "Local path: \n"
        "Server:\n"
        "  URL: https://fuel.gazebosim.org\n"
        "  Version: 1.0\n"
        "  API key: \n";
    EXPECT_EQ(str, id.AsString());
  }

  {
    WorldIdentifier id;
    id.SetName("hello");
    id.SetOwner("raspberry");
    id.SetVersionStr("55");

    auto str = id.AsString();
    gzdbg << str << std::endl;

    EXPECT_NE(str.find("hello"), std::string::npos);
    EXPECT_NE(str.find("raspberry"), std::string::npos);
    EXPECT_NE(str.find("55"), std::string::npos);
  }
}

/////////////////////////////////////////////////
TEST(WorldIdentifier, AsPrettyString)
{
  common::Console::SetVerbosity(4);
  {
    WorldIdentifier id;
    std::string str =
      "\x1B[96m\x1B[1mServer:\x1B[0m\n  "
      "\x1B[96m\x1B[1mURL: \x1B[0m\x1B[37mhttps://fuel.gazebosim.org"
      "\x1B[0m\n  \x1B[96m\x1B[1mVersion: \x1B[0m\x1B[37m1.0\x1B[0m\n";
    EXPECT_EQ(str, id.AsPrettyString());
  }

  {
    WorldIdentifier id;
    id.SetName("hello");
    id.SetOwner("raspberry");
    id.SetVersionStr("55");

    auto str = id.AsPrettyString();
    gzdbg << str << std::endl;

    EXPECT_NE(str.find("hello"), std::string::npos);
    EXPECT_NE(str.find("raspberry"), std::string::npos);
    EXPECT_NE(str.find("55"), std::string::npos);
  }
}
