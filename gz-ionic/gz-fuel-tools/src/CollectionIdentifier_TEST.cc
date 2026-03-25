/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <gz/utils/ExtraTestMacros.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/CollectionIdentifier.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
/// \brief Fields can be set
TEST(CollectionIdentifier, SetFields)
{
  CollectionIdentifier id;
  id.SetName("hello");
  id.SetOwner("osrf");
  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(common::URI("https://localhost:8001/"));
  id.SetServer(srv1);

  EXPECT_EQ("hello", id.Name());
  EXPECT_EQ("osrf", id.Owner());
  EXPECT_EQ(srv1.Url(), id.Server().Url());
}

/////////////////////////////////////////////////
/// \brief Unique Name
TEST(CollectionIdentifier, UniqueName)
{
  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(common::URI("https://localhost:8001", true));

  gz::fuel_tools::ServerConfig srv2;
  srv2.SetUrl(common::URI("https://localhost:8002", true));

  gz::fuel_tools::ServerConfig srv3;
  srv3.SetUrl(common::URI("https://localhost:8003", true));

  CollectionIdentifier id;
  id.SetName("hello");
  id.SetOwner("alice");
  id.SetServer(srv1);
  EXPECT_EQ("localhost%3A8001/alice/collections/hello", id.UniqueName());

  id.SetServer(srv2);
  EXPECT_EQ("localhost%3A8002/alice/collections/hello", id.UniqueName());

  id.SetServer(srv3);
  EXPECT_EQ("localhost%3A8003/alice/collections/hello", id.UniqueName());
}

/////////////////////////////////////////////////
/// \brief Unique Name
TEST(CollectionIdentifier, Url)
{
  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(common::URI("https://localhost:8001", true));

  gz::fuel_tools::ServerConfig srv2;
  srv2.SetUrl(common::URI("https://localhost:8002", true));

  gz::fuel_tools::ServerConfig srv3;
  srv3.SetUrl(common::URI("https://localhost:8003", true));

  CollectionIdentifier id;
  id.SetName("hello");
  id.SetOwner("alice");
  id.SetServer(srv1);
  EXPECT_EQ("https://localhost:8001/alice/collections/hello", id.Url().Str());

  id.SetServer(srv2);
  EXPECT_EQ("https://localhost:8002/alice/collections/hello", id.Url().Str());

  id.SetServer(srv3);
  EXPECT_EQ("https://localhost:8003/alice/collections/hello", id.Url().Str());
}

/////////////////////////////////////////////////
/// \brief Copy constructor deep copies
TEST(CollectionIdentifier, CopyConstructorDeepCopy)
{
  CollectionIdentifier id;
  id.SetName("hello");
  id.SetOwner("watermelon");

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(common::URI("https://localhost:8001"));
  id.SetServer(srv);

  CollectionIdentifier id2(id);
  EXPECT_EQ("hello", id2.Name());
  EXPECT_EQ("watermelon", id2.Owner());
  EXPECT_EQ(id.Server().Url(), id2.Server().Url());

  id2.SetName("hello2");
  EXPECT_EQ("hello", id.Name());
  EXPECT_EQ("hello2", id2.Name());
}

/////////////////////////////////////////////////
/// \brief assignment operator deep copies
TEST(CollectionIdentifier, AssignmentOperatorDeepCopy)
{
  CollectionIdentifier id;
  id.SetName("hello");
  id.SetOwner("pineapple");
  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(common::URI("https://localhost:8001"));
  id.SetServer(srv);

  CollectionIdentifier id2(id);
  id2 = id;
  EXPECT_EQ(std::string("hello"), id2.Name());
  EXPECT_EQ(std::string("pineapple"), id2.Owner());
  EXPECT_EQ(id.Server().Url(), id2.Server().Url());


  id2.SetName("hello2");
  EXPECT_EQ("hello", id.Name());
  EXPECT_EQ("hello2", id2.Name());
}

/////////////////////////////////////////////////
TEST(CollectionIdentifier, AsString)
{
  common::Console::SetVerbosity(4);
  {
    CollectionIdentifier id;

    std::string str =
        "Name: \n"\
        "Owner: \n"\
        "Unique name: fuel.gazebosim.org/collections/\n"
        "Server:\n"
        "  URL: https://fuel.gazebosim.org\n"
        "  Version: 1.0\n"
        "  API key: \n";
    EXPECT_EQ(str, id.AsString());
  }

  {
    CollectionIdentifier id;
    id.SetName("hello");
    id.SetOwner("raspberry");

    auto str = id.AsString();

    EXPECT_NE(str.find("hello"), std::string::npos);
    EXPECT_NE(str.find("raspberry"), std::string::npos);
  }
}

/////////////////////////////////////////////////
TEST(CollectionIdentifier, AsPrettyString)
{
  common::Console::SetVerbosity(4);
  {
    CollectionIdentifier id;
    std::string str =
      "\x1B[96m\x1B[1mServer:\x1B[0m\n  "
      "\x1B[96m\x1B[1mURL: \x1B[0m\x1B[37mhttps://fuel.gazebosim.org"
      "\x1B[0m\n  \x1B[96m\x1B[1mVersion: \x1B[0m\x1B[37m1.0\x1B[0m\n";
    EXPECT_EQ(str, id.AsPrettyString());
  }

  {
    CollectionIdentifier id;
    id.SetName("hello");
    id.SetOwner("raspberry");

    auto str = id.AsPrettyString();
    EXPECT_NE(str.find("hello"), std::string::npos);
    EXPECT_NE(str.find("raspberry"), std::string::npos);
  }
}
