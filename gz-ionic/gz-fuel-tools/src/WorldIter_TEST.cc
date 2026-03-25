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
#include <cstdio>
#include <string>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/WorldIter.hh"

#include "WorldIterPrivate.hh"

using namespace gz;
using namespace fuel_tools;

namespace gz
{
  namespace fuel_tools
  {
    /// \brief Forward Declaration
    class WorldIterTestFixture;

    /// \brief Class to misuse friendship
    class WorldIterTest
    {
      public: static WorldIter EmptyWorldIter()
        {
          return WorldIterFactory::Create();
        }

      public: static WorldIter WorldIterThreeWorldIds()
        {
          gz::fuel_tools::ServerConfig srv;
          srv.SetUrl(common::URI("https://gazebosim.org"));

          std::vector<WorldIdentifier> ids;
          for (int i = 0; i < 3; i++)
          {
            WorldIdentifier id;
            char buf[10];
            std::snprintf(buf, sizeof(buf), "world%d", i);
            std::string name = buf;
            std::snprintf(buf, sizeof(buf), "owner%d", i);
            std::string owner = buf;
            id.SetName(name);
            id.SetOwner(owner);
            id.SetServer(srv);
            ids.push_back(id);
          }
          return WorldIterFactory::Create(ids);
        }
    };
  }
}

/////////////////////////////////////////////////
/// \brief No ids, iter should evaluate to false
TEST(WorldIterTestFixture, FalseIfNoWorlds)
{
  EXPECT_FALSE(WorldIterTest::EmptyWorldIter());
}

/////////////////////////////////////////////////
TEST(WorldIterTestFixture, ConstBool)
{
  const WorldIter constIter = WorldIterTest::EmptyWorldIter();
  EXPECT_FALSE(constIter);
}

/////////////////////////////////////////////////
/// \brief Iter should be ready to move if there are ids
TEST(WorldIterTestFixture, TrueIfSomeWorlds)
{
  EXPECT_TRUE(WorldIterTest::WorldIterThreeWorldIds());
}

/////////////////////////////////////////////////
/// \brief Iter should move through 3 ids
TEST(WorldIterTestFixture, MoveThroughIds)
{
  WorldIter iter = WorldIterTest::WorldIterThreeWorldIds();
  EXPECT_TRUE(iter);
  EXPECT_EQ("world0", iter->Name());

  ++iter;
  EXPECT_TRUE(iter);
  EXPECT_EQ("world1", iter->Name());

  ++iter;
  EXPECT_TRUE(iter);
  EXPECT_EQ("world2", iter->Name());

  ++iter;
  EXPECT_FALSE(iter);
}
