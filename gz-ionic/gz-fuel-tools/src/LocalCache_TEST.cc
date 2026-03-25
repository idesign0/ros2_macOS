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

#include <fstream>
#include <set>
#include <string>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/testing/TestPaths.hh>
#include <gz/utils/ExtraTestMacros.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"

#include "LocalCache.hh"

using namespace gz;
using namespace fuel_tools;

/// \brief Creates a directory structure in the build directory with 6 models
void createLocal6Models(ClientConfig &_conf)
{
  gzdbg << "Creating 6 local models in [" << common::cwd() << "]" << std::endl;

  auto serverPath =
    common::joinPaths("test_cache", sanitizeAuthority("localhost:8001"));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "alice", "models", "am1", "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "alice", "models", "am2", "1")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "bob", "models", "bm1", "1")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "bob", "models", "bm2", "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "trudy", "models", "tm1", "3")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "trudy", "models", "tm2", "2")));

  std::ofstream fout(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      std::ofstream::trunc);
  fout << "<?xml version=\"1.0\"?>";
  fout.flush();
  fout.close();

  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "alice", "models", "am2", "1", "model.config")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "bob", "models", "bm1", "1", "model.config")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "bob", "models", "bm2", "2", "model.config")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "trudy", "models", "tm1", "3", "model.config")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "trudy", "models", "tm2", "2", "model.config")));

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(common::URI("http://localhost:8001/", true));
  _conf.AddServer(srv);
}

/// \brief Creates a directory structure in the build directory with 3 models
void createLocal3Models(ClientConfig &_conf)
{
  gzdbg << "Creating 3 local models in [" << common::cwd() << "]" << std::endl;

  auto serverPath =
    common::joinPaths("test_cache", sanitizeAuthority("localhost:8007"));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "alice", "models", "am1", "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "bob", "models", "bm1", "1")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "trudy", "models", "tm1", "3")));

  std::ofstream fout(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      std::ofstream::trunc);
  fout << "<?xml version=\"1.0\"?>";
  fout.flush();
  fout.close();

  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "bob", "models", "bm1", "1", "model.config")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "models", "am1", "2", "model.config"),
      common::joinPaths(serverPath,
      "trudy", "models", "tm1", "3", "model.config")));

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(gz::common::URI("http://localhost:8007/", true));
  _conf.AddServer(srv);
}

/// \brief Creates a directory structure in the build directory with 6 worlds
void createLocal6Worlds(ClientConfig &_conf)
{
  gzdbg << "Creating 6 local worlds in [" << common::cwd() << "]" << std::endl;

  auto serverPath =
    common::joinPaths("test_cache", sanitizeAuthority("localhost:8001"));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "alice", "worlds", "am2", "1")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "bob", "worlds", "bm1", "1")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "bob", "worlds", "bm2", "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "trudy", "worlds", "tm1", "3")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "trudy", "worlds", "tm2", "2")));

  std::ofstream fout(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      std::ofstream::trunc);
  fout << "<?xml version=\"1.0\"?>";
  fout.flush();
  fout.close();

  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "alice", "worlds", "am2", "1", "world.world")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "bob", "worlds", "bm1", "1", "world.world")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "bob", "worlds", "bm2", "2", "world.world")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "trudy", "worlds", "tm1", "3", "world.world")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "trudy", "worlds", "tm2", "2", "world.world")));

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(gz::common::URI("http://localhost:8001/", true));
  _conf.AddServer(srv);
}

/// \brief Creates a directory structure in the build directory with 3 worlds
void createLocal3Worlds(ClientConfig &_conf)
{
  gzdbg << "Creating 3 local worlds in [" << common::cwd() << "]" << std::endl;

  auto serverPath = common::joinPaths(
    "test_cache", sanitizeAuthority("localhost:8007"));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "bob", "worlds", "bm1", "1")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(serverPath,
      "trudy", "worlds", "tm1", "3")));

  std::ofstream fout(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      std::ofstream::trunc);
  fout << "<?xml version=\"1.0\"?>";
  fout.flush();
  fout.close();

  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "bob", "worlds", "bm1", "1", "world.world")));
  ASSERT_TRUE(common::copyFile(common::joinPaths(serverPath,
      "alice", "worlds", "am1", "2", "world.world"),
      common::joinPaths(serverPath,
      "trudy", "worlds", "tm1", "3", "world.world")));

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(common::URI("http://localhost:8007/", true));
  _conf.AddServer(srv);
}

/////////////////////////////////////////////////
class LocalCacheTest : public ::testing::Test
{
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);
    tempDir = gz::common::testing::MakeTestTempDirectory();
    ASSERT_TRUE(tempDir->Valid()) << tempDir->Path();

    gz::common::chdir(tempDir->Path());
    ASSERT_FALSE(common::exists("test_cache"));
    ASSERT_TRUE(common::createDirectories("test_cache"));
    ASSERT_TRUE(common::isDirectory("test_cache"));

    // This suppresses some unnecessary console spam.
    common::createDirectories(
        common::joinPaths("test_cache", "fuel.gazebosim.org"));
  }

  public: std::shared_ptr<gz::common::TempDirectory> tempDir;
};

/////////////////////////////////////////////////
/// \brief Iterate through all models in cache
TEST_F(LocalCacheTest, AllModels)
{
  ClientConfig conf;
  conf.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocal6Models(conf);
  createLocal3Models(conf);

  gz::fuel_tools::LocalCache cache(&conf);

  auto iter = cache.AllModels();
  std::set<std::string> uniqueNames;
  while (iter)
  {
    uniqueNames.insert(iter->Identification().UniqueName());
    ++iter;
  }
  EXPECT_EQ(9u, uniqueNames.size());

  EXPECT_NE(uniqueNames.end(), uniqueNames.find(
      "localhost%3A8001/alice/models/am1"));
}

/////////////////////////////////////////////////
/// \brief Get all models that match some fields
/// \brief Iterate through all models in cache
TEST_F(LocalCacheTest, MatchingModels)
{
  ClientConfig conf;
  conf.Clear();
  conf.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocal6Models(conf);
  createLocal3Models(conf);

  gz::fuel_tools::LocalCache cache(&conf);

  ModelIdentifier am1;
  am1.SetServer(conf.Servers().front());
  am1.SetOwner("alice");
  am1.SetName("am1");
  auto iter = cache.MatchingModels(am1);

  std::set<std::string> uniqueNames;
  while (iter)
  {
    EXPECT_EQ("alice", iter->Identification().Owner());
    EXPECT_EQ("am1", iter->Identification().Name());

    uniqueNames.insert(iter->Identification().UniqueName());
    ++iter;
  }
  EXPECT_EQ(1u, uniqueNames.size());

  am1.SetServer(conf.Servers().back());
  auto iter2 = cache.MatchingModels(am1);
  while (iter2)
  {
    EXPECT_EQ("alice", iter2->Identification().Owner());
    EXPECT_EQ("am1", iter2->Identification().Name());

    uniqueNames.insert(iter2->Identification().UniqueName());
    ++iter2;
  }
  EXPECT_EQ(2u, uniqueNames.size());
}

/////////////////////////////////////////////////
/// \brief Get a specific model from cache
/// \brief Iterate through all models in cache
TEST_F(LocalCacheTest, MatchingModel)
{
  ClientConfig conf;
  conf.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocal6Models(conf);

  gz::fuel_tools::LocalCache cache(&conf);

  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(common::URI("http://localhost:8001/", true));

  gz::fuel_tools::ServerConfig srv2;
  srv2.SetUrl(common::URI("http://localhost:8002/", true));

  ModelIdentifier am1;
  am1.SetServer(srv1);
  am1.SetOwner("alice");
  am1.SetName("am1");
  auto am1Model = cache.MatchingModel(am1);
  ASSERT_TRUE(am1Model);
  EXPECT_EQ("alice", am1Model.Identification().Owner());
  EXPECT_EQ("am1", am1Model.Identification().Name());
  EXPECT_EQ("http://localhost:8001/",
      am1Model.Identification().Server().Url().Str());

  ModelIdentifier tm2;
  tm2.SetServer(srv1);
  tm2.SetOwner("trudy");
  tm2.SetName("tm2");
  EXPECT_TRUE(cache.MatchingModel(tm2));

  ModelIdentifier bogus1;
  bogus1.SetServer(srv2);
  bogus1.SetOwner("trudy");
  bogus1.SetName("tm2");
  EXPECT_FALSE(cache.MatchingModel(bogus1));

  ModelIdentifier bogus2;
  bogus2.SetServer(srv1);
  bogus2.SetOwner("tfudy");
  bogus2.SetName("tm2");
  EXPECT_FALSE(cache.MatchingModel(bogus2));

  ModelIdentifier bogus3;
  bogus3.SetServer(srv1);
  bogus3.SetOwner("trudy");
  bogus3.SetName("tm3");
  EXPECT_FALSE(cache.MatchingModel(bogus3));
}

/////////////////////////////////////////////////
/// \brief Iterate through all worlds in cache
/// \brief Iterate through all models in cache
TEST_F(LocalCacheTest, AllWorlds)
{
  ClientConfig conf;
  conf.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocal6Worlds(conf);
  createLocal3Worlds(conf);

  gz::fuel_tools::LocalCache cache(&conf);

  auto iter = cache.AllWorlds();
  std::set<std::string> uniqueNames;
  while (iter)
  {
    uniqueNames.insert(iter->UniqueName());
    ++iter;
  }
  EXPECT_EQ(9u, uniqueNames.size());
  EXPECT_NE(uniqueNames.end(), uniqueNames.find(
    sanitizeAuthority("localhost:8001") + "/alice/worlds/am1"));
}

/////////////////////////////////////////////////
/// \brief Get all worlds that match some fields
/// \brief Iterate through all models in cache
TEST_F(LocalCacheTest, MatchingWorlds)
{
  ClientConfig conf;
  conf.Clear();
  conf.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocal6Worlds(conf);
  createLocal3Worlds(conf);

  gz::fuel_tools::LocalCache cache(&conf);

  WorldIdentifier am1;
  am1.SetServer(conf.Servers().front());
  am1.SetOwner("alice");
  am1.SetName("am1");
  auto iter = cache.MatchingWorlds(am1);

  std::set<std::string> uniqueNames;
  while (iter)
  {
    EXPECT_EQ("alice", iter->Owner());
    EXPECT_EQ("am1", iter->Name());

    uniqueNames.insert(iter->UniqueName());
    ++iter;
  }
  EXPECT_EQ(1u, uniqueNames.size());
}

/////////////////////////////////////////////////
/// \brief Get a specific world from cache
/// \brief Iterate through all models in cache
TEST_F(LocalCacheTest, MatchingWorld)
{
  ClientConfig conf;
  conf.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocal6Worlds(conf);

  gz::fuel_tools::LocalCache cache(&conf);

  gz::fuel_tools::ServerConfig srv1;
  srv1.SetUrl(gz::common::URI("http://localhost:8001/", true));

  gz::fuel_tools::ServerConfig srv2;
  srv2.SetUrl(gz::common::URI("http://localhost:8002/", true));

  WorldIdentifier am1;
  am1.SetServer(srv1);
  am1.SetOwner("alice");
  am1.SetName("am1");
  EXPECT_TRUE(cache.MatchingWorld(am1));
  EXPECT_EQ("alice", am1.Owner());
  EXPECT_EQ("am1", am1.Name());
  EXPECT_EQ("http://localhost:8001/", am1.Server().Url().Str());

  WorldIdentifier tm2;
  tm2.SetServer(srv1);
  tm2.SetOwner("trudy");
  tm2.SetName("tm2");
  EXPECT_TRUE(cache.MatchingWorld(tm2));

  WorldIdentifier bogus1;
  bogus1.SetServer(srv2);
  bogus1.SetOwner("trudy");
  bogus1.SetName("tm2");
  EXPECT_FALSE(cache.MatchingWorld(bogus1));

  WorldIdentifier bogus2;
  bogus2.SetServer(srv1);
  bogus2.SetOwner("tfudy");
  bogus2.SetName("tm2");
  EXPECT_FALSE(cache.MatchingWorld(bogus2));

  WorldIdentifier bogus3;
  bogus3.SetServer(srv1);
  bogus3.SetOwner("trudy");
  bogus3.SetName("tm3");
  EXPECT_FALSE(cache.MatchingWorld(bogus3));
}
