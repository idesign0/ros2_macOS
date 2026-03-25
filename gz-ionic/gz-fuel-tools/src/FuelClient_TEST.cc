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
#include <fstream>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/utils/ExtraTestMacros.hh>

#include "gz/fuel_tools/FuelClient.hh"
#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/Result.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"

#include <gz/common/testing/TestPaths.hh>

using namespace gz;
using namespace gz::fuel_tools;

//////////////////////////////////////////////////
/// \brief Creates a directory structure in the build directory with 1 model
/// that has 2 versions
/// Taken from LocalCache_TEST
void createLocalModel(ClientConfig &_conf)
{
  gzdbg << "Creating local model in [" << common::cwd() << "]" << std::endl;

  auto modelPath = common::joinPaths(
      "test_cache",
      sanitizeAuthority("localhost:8007"),
      "alice", "models", "My Model");

  ASSERT_TRUE(common::createDirectories(
      common::joinPaths(modelPath, "2", "meshes")));
  ASSERT_TRUE(common::createDirectories(
      common::joinPaths(modelPath, "3", "meshes")));

  {
    std::ofstream fout(common::joinPaths(modelPath, "2", "model.config"),
        std::ofstream::trunc);
    fout << "<?xml version=\"1.0\"?>";
    fout.flush();
    fout.close();

    ASSERT_TRUE(common::copyFile(common::joinPaths(modelPath, "2",
        "model.config"), common::joinPaths(modelPath, "3", "model.config")));
  }

  {
    std::ofstream fout(common::joinPaths(modelPath, "2", "model.sdf"),
        std::ofstream::trunc);
    fout << "<?xml version=\"1.0\"?>";
    fout.flush();
    fout.close();

    ASSERT_TRUE(common::copyFile(common::joinPaths(modelPath, "2", "model.sdf"),
        common::joinPaths(modelPath, "3", "model.sdf")));
  }

  {
    std::ofstream fout(common::joinPaths(modelPath, "2", "meshes", "model.dae"),
        std::ofstream::trunc);
    fout << "<?xml version=\"1.0\"?>";
    fout.flush();
    fout.close();

    ASSERT_TRUE(common::copyFile(common::joinPaths(modelPath, "2", "meshes",
        "model.dae"), common::joinPaths(modelPath, "3", "meshes",
        "model.dae")));
  }

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(common::URI("http://localhost:8007/", true));
  _conf.AddServer(srv);
}

//////////////////////////////////////////////////
/// \brief Creates a directory structure in the build directory with 1 world
/// that has 2 versions
/// Taken from LocalCache_TEST
void createLocalWorld(ClientConfig &_conf)
{
  gzdbg << "Creating local world in [" << common::cwd() << "]" << std::endl;

  auto worldPath = common::joinPaths(
      "test_cache",
      sanitizeAuthority("localhost:8007"),
      "banana", "worlds", "My World");

  ASSERT_TRUE(common::createDirectories(common::joinPaths(worldPath, "2")));
  ASSERT_TRUE(common::createDirectories(common::joinPaths(worldPath, "3")));

  {
    std::ofstream fout(common::joinPaths(worldPath, "2", "strawberry.world"),
        std::ofstream::trunc);
    fout << "<?xml version=\"1.0\"?>";
    fout.flush();
    fout.close();

    ASSERT_TRUE(common::copyFile(common::joinPaths(worldPath, "2",
        "strawberry.world"), common::joinPaths(worldPath, "3",
        "strawberry.world")));
  }

  gz::fuel_tools::ServerConfig srv;
  srv.SetUrl(gz::common::URI("http://localhost:8007/", true));
  _conf.AddServer(srv);
}

/////////////////////////////////////////////////
class FuelClientTest: public ::testing::Test
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
    ASSERT_FALSE(common::exists("test_cache/fuel.gazebosim.org"));
    ASSERT_TRUE(common::createDirectories("test_cache/fuel.gazebosim.org"));
    ASSERT_TRUE(common::isDirectory("test_cache/fuel.gazebosim.org"));
  }

  public: std::shared_ptr<gz::common::TempDirectory> tempDir;
};

/////////////////////////////////////////////////
/// \brief Nothing crashes
TEST_F(FuelClientTest, ParseModelURL)
{
  // * without client config
  // * with server API version
  // * without model version
  {
    FuelClient client;
    ModelIdentifier id;
    const std::string url{
      "https://some.example.org/1.0/openroboticstest/models/Cardboard Box"};
    EXPECT_TRUE(client.ParseModelUrl(common::URI(url), id));

    EXPECT_EQ(id.Server().Url().Str(), "https://some.example.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 0u);
  }

  // * with client config
  // * with server API version
  // * with model version
  {
    ClientConfig config;

    FuelClient client(config);
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/1.0/openroboticstest/models/Cardboard Box/4"};
    EXPECT_TRUE(client.ParseModelUrl(common::URI(url), id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 4u);
  }

  // * with client config
  // * with server API version different from config
  // * with model version
  // * with trailing /
  {
    ClientConfig config;

    FuelClient client(config);
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/5.0/openroboticstest/models/Cardboard Box/6/"
    };
    EXPECT_TRUE(client.ParseModelUrl(common::URI(url), id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 6u);
  }

  // * without client config
  // * without server API version
  // * without model version
  {
    FuelClient client;
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/openroboticstest/models/Cardboard Box"};
    EXPECT_TRUE(client.ParseModelUrl(common::URI(url), id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_FALSE(id.Server().Version().empty());
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 0u);
  }

  // * with client config
  // * without server API version
  // * with model version `tip`
  {
    ClientConfig config;

    FuelClient client(config);
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/openroboticstest/models/Cardboard Box/tip"};
    EXPECT_TRUE(client.ParseModelUrl(common::URI(url), id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 0u);
  }

  // Bad model Urls
  {
    FuelClient client;
    ModelIdentifier id;
    EXPECT_FALSE(client.ParseModelUrl(common::URI("http://bad.url"),
          id));
  }

  // Not Url
  {
    FuelClient client;
    ModelIdentifier id;
    EXPECT_FALSE(client.ParseModelUrl(common::URI("bad url"),
          id));
  }
  {
    FuelClient client;
    ModelIdentifier id;
    EXPECT_FALSE(client.ParseModelUrl(common::URI("ba://url"),
          id));
  }
  {
    FuelClient client;
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/openroboticstest/models/Cardboard Box/banana"
    };
    EXPECT_FALSE(client.ParseModelUrl(common::URI(url), id));
  }
  {
    FuelClient client;
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/banana/openroboticstest/models/Cardboard Box"
    };
    EXPECT_FALSE(client.ParseModelUrl(common::URI(url), id));
  }
  {
    FuelClient client;
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/99/openroboticstest/models/Cardboard Box"};
    EXPECT_FALSE(client.ParseModelUrl(common::URI(url), id));
  }
  {
    FuelClient client;
    ModelIdentifier id;
    const std::string url{
      "https://fuel.gazebosim.org/2/2/openroboticstest/models"
        "/Cardboard Box/banana"};
    EXPECT_FALSE(client.ParseModelUrl(common::URI(url), id));
  }
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, ParseModelFileURL)
{
  // URL - without client config
  {
    FuelClient client;
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{
      "https://fuel.gazebosim.org/1.0/openroboticstest/models/"
        "backpack/tip/files/model.sdf"};
    EXPECT_TRUE(client.ParseModelFileUrl(modelUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "backpack");
    EXPECT_EQ(filePath, "model.sdf");
  }

  // URL - with client config
  {
    ClientConfig config;

    FuelClient client(config);
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{
      "https://fuel.gazebosim.org/1.0/openroboticstest/models/backpack/"
        "tip/files/model.sdf"};
    EXPECT_TRUE(client.ParseModelFileUrl(modelUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "backpack");
    EXPECT_EQ(filePath, "model.sdf");
  }

  // URL - version different from config
  {
    ClientConfig config;

    FuelClient client(config);
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{
      "https://fuel.gazebosim.org/5.0/openroboticstest/models/backpack/"
        "tip/files/model.sdf"};
    EXPECT_TRUE(client.ParseModelFileUrl(modelUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "backpack");
    EXPECT_EQ(filePath, "model.sdf");
  }

  // Unique name - without client config
  {
    FuelClient client;
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{
      "https://fuel.gazebosim.org/openroboticstest/models/backpack/tip/"
      "files/model.sdf"};
    EXPECT_TRUE(client.ParseModelFileUrl(modelUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_FALSE(id.Server().Version().empty());
    EXPECT_EQ("1.0", id.Server().Version());
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "backpack");
    EXPECT_EQ(filePath, "model.sdf");
  }

  // Unique name - with client config
  {
    ClientConfig config;

    FuelClient client(config);
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{
      "https://fuel.gazebosim.org/openroboticstest/models/backpack/tip/"
      "files/model.sdf"};
    EXPECT_TRUE(client.ParseModelFileUrl(modelUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "backpack");
    EXPECT_EQ(filePath, "model.sdf");
  }

  // Bad URL
  {
    FuelClient client;
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{"http://bad.url"};
    EXPECT_FALSE(client.ParseModelFileUrl(modelUrl, id, filePath));
  }

  // Not URL
  {
    FuelClient client;
    ModelIdentifier id;
    std::string filePath;
    const common::URI modelUrl{"bad_url"};
    EXPECT_FALSE(client.ParseModelFileUrl(modelUrl, id, filePath));
  }
}

class FuelClientDownloadTest
    : public FuelClientTest,
      public ::testing::WithParamInterface<const char *>
{};

INSTANTIATE_TEST_SUITE_P(
    FuelClientTest, FuelClientDownloadTest,
    ::testing::Values("fuel.gazebosim.org"));

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_P(FuelClientDownloadTest, DownloadModel)
{
  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));

  // Create client
  FuelClient client(config);
  EXPECT_EQ(config.CacheLocation(), client.Config().CacheLocation());

  std::string fuelServerHost = GetParam();
  // Download model from URL
  {
    // Unversioned URL should get the latest available version
    common::URI url{
      "https://" + fuelServerHost + "/1.0/openroboticstest/models/test box"};

    // Check it is not cached
    std::string cachedPath;
    Result res1 = client.CachedModel(url, cachedPath);
    EXPECT_FALSE(res1);
    EXPECT_EQ(ResultType::FETCH_ERROR, res1.Type());

    // Download
    std::string path;
    Result res2 = client.DownloadModel(url, path);
    EXPECT_TRUE(res2);
    EXPECT_EQ(ResultType::FETCH, res2.Type());

    // Check it was downloaded to `2`
    auto modelPath = common::joinPaths(common::cwd(), "test_cache",
        fuelServerHost, "openroboticstest", "models", "test box");

    EXPECT_EQ(path, common::joinPaths(modelPath, "1"));
    EXPECT_TRUE(common::exists(common::joinPaths(modelPath, "1")));
    EXPECT_TRUE(common::exists(common::joinPaths(modelPath, "1", "model.sdf")));
    EXPECT_TRUE(common::exists(common::joinPaths(modelPath, "1",
        "model.config")));

    // Check it wasn't downloaded to model root directory
    EXPECT_FALSE(common::exists(common::joinPaths(modelPath, "model.config")));

    // Check it is cached
    Result res3 = client.CachedModel(url, cachedPath);
    EXPECT_TRUE(res3);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res3.Type());
    EXPECT_EQ(common::joinPaths(modelPath, "1"), cachedPath);
  }

  // Download model with pbr paths from URL and check that paths are fixed
  {
    // Unversioned URL should get the latest available version
    common::URI url{"https://" + fuelServerHost +
                    "/1.0/openroboticstest/models/Rescue Randy"};

    // Check it is not cached
    std::string cachedPath;
    Result res1 = client.CachedModel(url, cachedPath);
    EXPECT_FALSE(res1);
    EXPECT_EQ(ResultType::FETCH_ERROR, res1.Type());

    // Download
    std::string path;
    Result res2 = client.DownloadModel(url, path);
    EXPECT_TRUE(res2);
    EXPECT_EQ(ResultType::FETCH, res2.Type());

    // Check it was downloaded to `2`
    auto modelPath = common::joinPaths(common::cwd(), "test_cache",
        fuelServerHost, "openroboticstest", "models", "rescue randy");

    EXPECT_EQ(path, common::joinPaths(modelPath, "2"));
    EXPECT_TRUE(common::exists(common::joinPaths(modelPath, "2")));
    EXPECT_TRUE(common::exists(common::joinPaths(modelPath, "2", "model.sdf")));
    EXPECT_TRUE(common::exists(common::joinPaths(modelPath, "2",
        "model.config")));

    // Check it wasn't downloaded to model root directory
    EXPECT_FALSE(common::exists(common::joinPaths(modelPath, "model.config")));

    // Check it is cached
    Result res3 = client.CachedModel(url, cachedPath);
    EXPECT_TRUE(res3);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res3.Type());
    EXPECT_EQ(common::joinPaths(modelPath, "2"), cachedPath);

    // Check that URIs have been updated.
    std::ifstream ifs(common::joinPaths(modelPath, "2", "model.sdf"));
    std::string modelSdf((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    EXPECT_EQ(std::string::npos, modelSdf.find("<uri>model://"));
    EXPECT_EQ(std::string::npos, modelSdf.find("<albedo_map>model://"));
    EXPECT_EQ(std::string::npos, modelSdf.find("<normal_map>model://"));
    EXPECT_EQ(std::string::npos, modelSdf.find("<metalness_map>model://"));
    EXPECT_EQ(std::string::npos, modelSdf.find("<roughness_map>model://"));

    EXPECT_NE(std::string::npos, modelSdf.find("<uri>https://"));
    EXPECT_NE(std::string::npos, modelSdf.find("<albedo_map>https://"));
    EXPECT_NE(std::string::npos, modelSdf.find("<normal_map>https://"));
    EXPECT_NE(std::string::npos, modelSdf.find("<metalness_map>https://"));
    EXPECT_NE(std::string::npos, modelSdf.find("<roughness_map>https://"));
  }

  // Download model with a dependency specified within its `metadata.pbtxt`
  {
    common::URI url{"https://" + fuelServerHost +
                    "/1.0/openroboticstest/models/hatchback_red_1"};

    // The dependency will use the fuel.gazebosim.org URI regardless of what
    // fuelServerHost is because it's set in the hatchback_red_1 model
    common::URI depUrl{
        "https://fuel.gazebosim.org/1.0/openroboticstest/models/hatchback_1"};

    // Check it is not cached
    std::string cachedPath;
    Result res1 = client.CachedModel(url, cachedPath);
    EXPECT_FALSE(res1);
    EXPECT_EQ(ResultType::FETCH_ERROR, res1.Type());

    // Check the dependency is not cached
    Result res2 = client.CachedModel(depUrl, cachedPath);
    EXPECT_FALSE(res2);
    EXPECT_EQ(ResultType::FETCH_ERROR, res2.Type());

    // Download
    std::string path;
    Result res3 = client.DownloadModel(url, path);
    EXPECT_TRUE(res3);
    EXPECT_EQ(ResultType::FETCH, res3.Type());

    // Check it is cached
    Result res4 = client.CachedModel(url, cachedPath);
    EXPECT_TRUE(res4);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res4.Type());

    // Check the dependency is cached
    Result res5 = client.CachedModel(depUrl, cachedPath);
    EXPECT_TRUE(res5);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res5.Type());
  }

  // Download model with a dependency specified within its `model.config`.
  // The dependency points to fuel.gazebosim.org.
  {
    common::URI url{
      "https://" + fuelServerHost + "/1.0/openrobotics/models/hatchback red"
    };
    // The dependency will use the fuel.gazebosim.org URI regardless of what
    // fuelServerHost is because it's set in the "hatchback red" model
    common::URI depUrl{
      "https://fuel.gazebosim.org/1.0/openrobotics/models/hatchback"};

    // Check it is not cached
    std::string cachedPath;
    Result res1 = client.CachedModel(url, cachedPath);
    EXPECT_FALSE(res1);
    EXPECT_EQ(ResultType::FETCH_ERROR, res1.Type());

    // Check the dependency is not cached
    Result res2 = client.CachedModel(depUrl, cachedPath);
    EXPECT_FALSE(res2);
    EXPECT_EQ(ResultType::FETCH_ERROR, res2.Type());

    // Download
    std::string path;
    Result res3 = client.DownloadModel(url, path);
    EXPECT_TRUE(res3);
    EXPECT_EQ(ResultType::FETCH, res3.Type());

    // Check it is cached
    Result res4 = client.CachedModel(url, cachedPath);
    EXPECT_TRUE(res4);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res4.Type());

    // Check the dependency is cached
    Result res5 = client.CachedModel(depUrl, cachedPath);
    EXPECT_TRUE(res5);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res5.Type());
  }

  // Try using nonexistent URL
  {
    std::string url{
        "https://" + fuelServerHost + "/1.0/openroboticstest/models/"
          "Inexistent model"};
    std::string path;
    Result result = client.DownloadModel(common::URI(url), path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Try using bad URL
  {
    std::string url{"banana"};
    std::string path;
    Result result = client.DownloadModel(common::URI(url), path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }
}

/////////////////////////////////////////////////
// Windows doesn't support colons in filenames
// https://github.com/gazebosim/gz-fuel-tools/issues/106
TEST_F(FuelClientTest, ModelDependencies)
{
  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));

  // Create client
  FuelClient client(config);
  EXPECT_EQ(config.CacheLocation(), client.Config().CacheLocation());

  // Download model with a dependency specified within its `metadata.pbtxt`
  {
    common::URI url{
        "https://fuel.gazebosim.org/1.0/openroboticstest/models/hatchback_red_1"
    };
    common::URI depUrl{
        "https://fuel.gazebosim.org/1.0/openroboticstest/models/hatchback_1"};

    ModelIdentifier id;
    ModelIdentifier depId;

    ASSERT_TRUE(client.ParseModelUrl(url, id));
    ASSERT_TRUE(client.ParseModelUrl(depUrl, depId));

    // Check it is not cached
    std::string cachedPath;
    Result res1 = client.CachedModel(url, cachedPath);
    EXPECT_FALSE(res1);
    EXPECT_EQ(ResultType::FETCH_ERROR, res1.Type());

    // Check the dependency is not cached
    Result res2 = client.CachedModel(depUrl, cachedPath);
    EXPECT_FALSE(res2);
    EXPECT_EQ(ResultType::FETCH_ERROR, res2.Type());

    // Download on the model, do not download dependencies
    {
      std::vector<ModelIdentifier> dependencies;
      Result res3 = client.DownloadModel(id, {}, dependencies);
      EXPECT_TRUE(res3);
      EXPECT_EQ(ResultType::FETCH, res3.Type());
      EXPECT_EQ(1u, dependencies.size());
    }

    // Check that the model is cached
    {
      Result res4 = client.CachedModel(url, cachedPath);
      EXPECT_TRUE(res4);
      EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res4.Type());
    }

    // Check the dependency is not cached
    {
      Result res5 = client.CachedModel(depUrl, cachedPath);
      EXPECT_FALSE(res5);
      EXPECT_EQ(ResultType::FETCH_ERROR, res5.Type());
    }

    // Check that the dependencies are populated
    {
      std::vector<ModelIdentifier> dependencies;
      Result res6 = client.ModelDependencies(id, dependencies);
      EXPECT_TRUE(res6);
      EXPECT_EQ(1u, dependencies.size());
    }
  }
}


/////////////////////////////////////////////////
// Windows doesn't support colons in filenames
// https://github.com/gazebosim/gz-fuel-tools/issues/106
// See https://github.com/gazebosim/gz-fuel-tools/issues/231
TEST_F(FuelClientTest, CachedModel)
{
  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocalModel(config);

  // Create client
  FuelClient client(config);
  EXPECT_EQ(config.CacheLocation(), client.Config().CacheLocation());

  auto basePath = common::joinPaths(common::cwd(), "test_cache",
                                    sanitizeAuthority("localhost:8007"));

  // Cached model (no version)
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/My Model", true};
    std::string path;
    auto result = client.CachedModel(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "alice", "models", "My Model", "3"), path);
  }

  // Cached model (tip)
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/My Model/tip",
                    true};
    std::string path;
    auto result = client.CachedModel(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "alice", "models", "My Model", "3"), path);
  }

  // Cached model (version number)
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/My Model/2", true};
    std::string path;
    auto result = client.CachedModel(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "alice", "models", "My Model", "2"), path);
  }

  // Cached model file (tip)
  {
    common::URI url{
        "http://localhost:8007/1.0/alice/models/My Model/tip/files/model.sdf",
        true};
    std::string path;
    auto result = client.CachedModelFile(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "alice", "models", "My Model", "3", "model.sdf"), path);
  }

  // Deeper cached model file
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/My Model/2/files/"
                    "meshes/model.dae", true};
    std::string path;
    auto result = client.CachedModelFile(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "alice", "models", "My Model", "2", "meshes", "model.dae"), path);
  }

  // Non-cached model
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/Banana", true};
    std::string path;
    auto result = client.CachedModel(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Non-cached model (when looking for file)
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/Banana/model.sdf",
                    true};
    std::string path;
    auto result = client.CachedModelFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Non-cached model file
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/My Model/tip/files/"
                    "meshes/banana.dae", true
    };
    std::string path;
    auto result = client.CachedModelFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Model root URL to model file
  {
    common::URI url{"http://localhost:8007/1.0/alice/models/My Model", true};
    std::string path;
    auto result = client.CachedModelFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Bad model URL
  {
    common::URI url{"banana"};
    std::string path;
    auto result = client.CachedModel(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Bad model file URL
  {
    common::URI url{"banana"};
    std::string path;
    auto result = client.CachedModelFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }
}

/////////////////////////////////////////////////
/// \brief Nothing crashes
TEST_F(FuelClientTest, ParseWorldUrl)
{
  // * without client config
  // * with server API version
  // * without world version
  {
    FuelClient client;
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/1.0/openroboticstest/worlds/Cardboard Box"};
    EXPECT_TRUE(client.ParseWorldUrl(url, id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 0u);
  }

  // * with client config
  // * with server API version
  // * with world version
  {
    ClientConfig config;

    FuelClient client(config);
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/1.0/openroboticstest/worlds/Cardboard Box/4"};
    EXPECT_TRUE(client.ParseWorldUrl(url, id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 4u);
  }

  // * with client config
  // * with server API version different from config
  // * with world version
  {
    ClientConfig config;

    FuelClient client(config);
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/5.0/openroboticstest/worlds/Cardboard Box/6"};
    EXPECT_TRUE(client.ParseWorldUrl(url, id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 6u);
  }

  // * without client config
  // * without server API version
  // * without world version
  {
    FuelClient client;
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/openroboticstest/worlds/Cardboard Box"};
    EXPECT_TRUE(client.ParseWorldUrl(url, id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_FALSE(id.Server().Version().empty());
    EXPECT_EQ("1.0", id.Server().Version());
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 0u);
  }

  // * with client config
  // * without server API version
  // * with world version `tip`
  // * with trailing /
  {
    ClientConfig config;

    FuelClient client(config);
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/openroboticstest/worlds/Cardboard Box/tip/"};
    EXPECT_TRUE(client.ParseWorldUrl(url, id));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openroboticstest");
    EXPECT_EQ(id.Name(), "cardboard box");
    EXPECT_EQ(id.Version(), 0u);
  }

  // Bad world URLs
  {
    FuelClient client;
    WorldIdentifier id;
    EXPECT_FALSE(client.ParseWorldUrl(common::URI("http://bad.url"), id));
  }

  // Not URL
  {
    FuelClient client;
    WorldIdentifier id;
    EXPECT_FALSE(client.ParseWorldUrl(common::URI("bad url"), id));
  }
  {
    FuelClient client;
    WorldIdentifier id;
    EXPECT_FALSE(client.ParseWorldUrl(common::URI("ba://url"), id));
  }
  {
    FuelClient client;
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/openroboticstest/worlds/Cardboard Box/here"};
    EXPECT_FALSE(client.ParseWorldUrl(url, id));
  }
  {
    FuelClient client;
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/banana/openroboticstest/worlds/Cardboard Box"
    };
    EXPECT_FALSE(client.ParseWorldUrl(url, id));
  }
  {
    FuelClient client;
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/99/openroboticstest/worlds/Cardboard Box"};
    EXPECT_FALSE(client.ParseWorldUrl(url, id));
  }
  {
    FuelClient client;
    WorldIdentifier id;
    const common::URI url{
      "https://fuel.gazebosim.org/2/2/openroboticstest/worlds/Cardboard Box"
        "/banana"};
    EXPECT_FALSE(client.ParseWorldUrl(url, id));
  }
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, ParseWorldFileUrl)
{
  // URL - without client config
  {
    FuelClient client;
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{
      "https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty/tip/"
      "files/test.world"};
    EXPECT_TRUE(client.ParseWorldFileUrl(worldUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openrobotics");
    EXPECT_EQ(id.Name(), "empty");
    EXPECT_EQ(filePath, "test.world");
  }

  // URL - with client config
  {
    ClientConfig config;

    FuelClient client(config);
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{
      "https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty sky/tip/"
      "files/empty_sky.world"};
    EXPECT_TRUE(client.ParseWorldFileUrl(worldUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openrobotics");
    EXPECT_EQ(id.Name(), "empty sky");
    EXPECT_EQ(filePath, "empty_sky.world");
  }

  // URL - version different from config
  {
    ClientConfig config;

    FuelClient client(config);
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{
      "https://fuel.gazebosim.org/5.0/OpenRobotics/worlds/Empty/tip/"
      "files/test.world"};
    EXPECT_TRUE(client.ParseWorldFileUrl(worldUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openrobotics");
    EXPECT_EQ(id.Name(), "empty");
    EXPECT_EQ(filePath, "test.world");
  }

  // Unique name - without client config
  {
    FuelClient client;
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{
      "https://fuel.gazebosim.org/OpenRobotics/worlds/Empty sky/tip/"
      "files/empty_sky.world"};
    EXPECT_TRUE(client.ParseWorldFileUrl(worldUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_FALSE(id.Server().Version().empty());
    EXPECT_EQ("1.0", id.Server().Version());
    EXPECT_EQ(id.Owner(), "openrobotics");
    EXPECT_EQ(id.Name(), "empty sky");
    EXPECT_EQ(filePath, "empty_sky.world");
  }

  // Unique name - with client config
  {
    ClientConfig config;

    FuelClient client(config);
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{
      "https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty/tip/"
      "files/test.world"};
    EXPECT_TRUE(client.ParseWorldFileUrl(worldUrl, id, filePath));

    EXPECT_EQ(id.Server().Url().Str(), "https://fuel.gazebosim.org");
    EXPECT_EQ(id.Server().Version(), "1.0");
    EXPECT_EQ(id.Owner(), "openrobotics");
    EXPECT_EQ(id.Name(), "empty");
    EXPECT_EQ(filePath, "test.world");
  }

  // Bad URL
  {
    FuelClient client;
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{"http://bad.url"};
    EXPECT_FALSE(client.ParseWorldFileUrl(worldUrl, id, filePath));
  }

  // Not URL
  {
    FuelClient client;
    WorldIdentifier id;
    std::string filePath;
    const common::URI worldUrl{"bad_url"};
    EXPECT_FALSE(client.ParseWorldFileUrl(worldUrl, id, filePath));
  }
}

//////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(FuelClientTest, DownloadWorld)
{
  ServerConfig server;
  server.SetUrl(common::URI(
        "https://fuel.gazebosim.org"));

  ClientConfig config;
  config.AddServer(server);
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));

  // Create client
  FuelClient client(config);
  EXPECT_EQ(config.CacheLocation(), client.Config().CacheLocation());

  // Download world from URL
  {
    // Unversioned URL should get the latest available version
    common::URI url{"https://fuel.gazebosim.org/1.0/OpenRobotics/"
                    "worlds/Test world"};

    // Check it is not cached
    std::string cachedPath;
    auto res1 = client.CachedWorld(url, cachedPath);
    EXPECT_FALSE(res1);
    EXPECT_EQ(ResultType::FETCH_ERROR, res1.Type());

    // Download
    std::string path;
    auto res2 = client.DownloadWorld(url, path);
    EXPECT_TRUE(res2);
    EXPECT_EQ(ResultType::FETCH, res2.Type());

    // Check it was downloaded to `1`
    auto worldPath = common::joinPaths(common::cwd(), "test_cache",
        "fuel.gazebosim.org", "openrobotics", "worlds", "test world");

    EXPECT_EQ(path, common::joinPaths(worldPath, "2"));
    EXPECT_TRUE(common::exists(common::joinPaths(worldPath, "2")));
    EXPECT_TRUE(common::exists(common::joinPaths(worldPath, "2", "test.sdf")));

    // Check it wasn't downloaded to world root directory
    EXPECT_FALSE(common::exists(common::joinPaths(worldPath, "test.sdf")));

    // Check it is cached
    auto res3 = client.CachedWorld(url, cachedPath);
    EXPECT_TRUE(res3);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, res3.Type());
    EXPECT_EQ(common::joinPaths(worldPath, "2"), cachedPath);
  }

  // Try using nonexistent URL
  {
    common::URI url{
        "https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Bad world"};
    std::string path;
    auto result = client.DownloadWorld(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Try using bad URL
  {
    common::URI url{"banana"};
    std::string path;
    auto result = client.DownloadWorld(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }
}

/////////////////////////////////////////////////
// Windows doesn't support colons in filenames
// https://github.com/gazebosim/gz-fuel-tools/issues/106
// This is fixed in gz-fuel-tools9+, but not here to preserve behavior
TEST_F(FuelClientTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(CachedWorld))
{
  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocalWorld(config);

  // Create client
  FuelClient client(config);
  EXPECT_EQ(config.CacheLocation(), client.Config().CacheLocation());

  auto basePath = common::joinPaths(common::cwd(), "test_cache",
                                    sanitizeAuthority("localhost:8007"));

  // Cached world (no version)
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World", true};
    std::string path;
    auto result = client.CachedWorld(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "banana", "worlds", "My World", "3"), path);
  }

  // Cached world (tip)
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World/tip",
                    true};
    std::string path;
    auto result = client.CachedWorld(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "banana", "worlds", "My World", "3"), path);
  }

  // Cached world (version number)
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World/2", true};
    std::string path;
    auto result = client.CachedWorld(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "banana", "worlds", "My World", "2"), path);
  }

  // Cached world file (tip)
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World/tip/"
                    "files/strawberry.world", true};
    std::string path;
    auto result = client.CachedWorldFile(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "banana", "worlds", "My World", "3",
        "strawberry.world"), path);
  }

  // Deeper cached world file
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World/2/files/"
                    "strawberry.world"};
    std::string path;
    auto result = client.CachedWorldFile(url, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(ResultType::FETCH_ALREADY_EXISTS, result.Type());
    EXPECT_EQ(common::joinPaths(basePath,
        "banana", "worlds", "My World", "2",
        "strawberry.world"), path);
  }

  // Non-cached world
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/Banana"};
    std::string path;
    auto result = client.CachedWorld(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Non-cached world (when looking for file)
  {
    common::URI url{
        "http://localhost:8007/1.0/banana/worlds/Banana/strawberry.world"};
    std::string path;
    auto result = client.CachedWorldFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Non-cached world file
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World/tip/"
                    "files/banana.sdf"};
    std::string path;
    auto result = client.CachedWorldFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // World root URL to world file
  {
    common::URI url{"http://localhost:8007/1.0/banana/worlds/My World"};
    std::string path;
    auto result = client.CachedWorldFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Bad world URL
  {
    common::URI url{"banana"};
    std::string path;
    auto result = client.CachedWorld(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }

  // Bad world file URL
  {
    common::URI url{"banana"};
    std::string path;
    auto result = client.CachedWorldFile(url, path);
    EXPECT_FALSE(result);
    EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
  }
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, Config)
{
  FuelClient client;
  ClientConfig &config = client.Config();

  // Check a few values. More client config tests in ClientConfig_TEST
  EXPECT_FALSE(config.UserAgent().empty());
  EXPECT_FALSE(config.CacheLocation().empty());
  EXPECT_FALSE(config.Servers().empty());
}

/////////////////////////////////////////////////
/// \brief Expect model download to fail with lack of server
TEST_F(FuelClientTest, ModelDownload)
{
  FuelClient client;

  std::string path;
  Result result = client.DownloadModel(common::URI("bad"), path);
  EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
}

/////////////////////////////////////////////////
/// \brief Expect world download to fail with lack of server
TEST_F(FuelClientTest, WorldDownload)
{
  FuelClient client;

  std::string path;
  Result result = client.DownloadWorld(common::URI("bad"), path);
  EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, ModelDetails)
{
  FuelClient client;
  ModelIdentifier modelId;
  ModelIdentifier model;

  modelId.SetOwner("bad-owner");
  Result result2 = client.ModelDetails(modelId, model);
  EXPECT_EQ(ResultType::FETCH_ERROR, result2.Type());
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, WorldDetails)
{
  FuelClient client;
  WorldIdentifier worldId;
  WorldIdentifier world;

  Result result = client.WorldDetails(worldId, world);
  EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(FuelClientTest, Models)
{
  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  FuelClient client(config);
  ServerConfig serverConfig;
  ModelIdentifier modelId;

  {
    ModelIter iter = client.Models(modelId);
    EXPECT_FALSE(iter);
  }

  {
    ModelIter const iter = client.Models(modelId);
    EXPECT_FALSE(iter);
  }

  {
    // Uses fuel.gazebosim.org by default
    ModelIter iter = client.Models(serverConfig);
    EXPECT_TRUE(iter);
  }

  {
    serverConfig.Clear();
    ModelIter const iter = client.Models(serverConfig);
    EXPECT_FALSE(iter);
  }
}

//////////////////////////////////////////////////
TEST_F(FuelClientTest, ModelsCheckCached)
{
  ClientConfig config;
  std::string cacheDir = common::joinPaths(common::cwd(), "test_cache");
  common::removeAll(cacheDir );
  ASSERT_TRUE(common::createDirectories(cacheDir));
  config.SetCacheLocation(cacheDir);
  FuelClient client(config);
  ServerConfig serverConfig;
  ModelIdentifier modelId;
  modelId.SetOwner("openroboticstest");
  std::vector<Model> modelInfos;
  {
    for (ModelIter iter = client.Models(modelId, true); iter; ++iter)
    {
      modelInfos.push_back(*iter);
    }
  }
  ASSERT_FALSE(modelInfos.empty());
  // Download one of the models to test the behavior of Models() with
  // different values for _checkCache
  EXPECT_TRUE(client.DownloadModel(modelInfos.front().Identification()));
  {
    std::size_t counter = 0;
    for (ModelIter iter = client.Models(modelId, true); iter; ++iter, ++counter)
    {
    }
    std::cout << "counter: " << counter << std::endl;
    // Expect only one result with checkCache=true because we've downloaded only
    // one model
    EXPECT_EQ(counter, 1u);
    EXPECT_GT(modelInfos.size(), counter);
  }

  {
    std::size_t counter = 0;
    for (ModelIter iter = client.Models(modelId, false); iter;
         ++iter, ++counter)
    {
    }
    std::cout << "counter: " << counter << std::endl;
    EXPECT_EQ(counter, modelInfos.size());
  }
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(FuelClientTest, Worlds)
{
  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  FuelClient client(config);
  ServerConfig serverConfig;
  WorldIdentifier worldId;

  {
    WorldIter iter = client.Worlds(worldId);
    EXPECT_TRUE(iter);
  }

  {
    WorldIter const iter = client.Worlds(worldId);
    EXPECT_TRUE(iter);
  }

  {
    // Uses fuel.gazebosim.org by default
    WorldIter iter = client.Worlds(serverConfig);
    EXPECT_TRUE(iter);
  }

  {
    serverConfig.Clear();
    WorldIter const iter = client.Worlds(serverConfig);
    EXPECT_FALSE(iter);
  }
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, DownloadModelFail)
{
  FuelClient client;
  ModelIdentifier modelId;

  Result result = client.DownloadModel(modelId);
  EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, DownloadWorldFail)
{
  FuelClient client;
  WorldIdentifier worldId;

  Result result = client.DownloadWorld(worldId);
  EXPECT_EQ(ResultType::FETCH_ERROR, result.Type());
}

/////////////////////////////////////////////////
TEST_F(FuelClientTest, UploadModelFail)
{
  FuelClient client;
  ModelIdentifier modelId;

  std::vector<std::string> headers;
  Result result = client.UploadModel("path", modelId, headers);
  EXPECT_EQ(ResultType::UPLOAD_ERROR, result.Type());
}

/////////////////////////////////////////////////
// Windows doesn't support colons in filenames
// https://github.com/gazebosim/gz-fuel-tools/issues/106
// This is fixed in gz-fuel-tools9+, but not here to preserve behavior
TEST_F(FuelClientTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(PatchModelFail))
{
  FuelClient client;
  ModelIdentifier modelId;

  std::vector<std::string> headers;

  // Bad directory
  Result result = client.PatchModel(modelId, headers, "bad");
  EXPECT_EQ(ResultType::UPLOAD_ERROR, result.Type());

  // Missing metadata.pbtxt and model.config
  result = client.PatchModel(modelId, headers, common::cwd());
  EXPECT_EQ(ResultType::UPLOAD_ERROR, result.Type());

  ClientConfig config;
  config.SetCacheLocation(common::joinPaths(common::cwd(), "test_cache"));
  createLocalModel(config);

  // Bad model.config
  result = client.PatchModel(modelId, headers,
      common::joinPaths(common::cwd(), "test_cache",
        sanitizeAuthority("localhost:8007"),
        "alice", "models", "My Model", "3"));
  EXPECT_EQ(ResultType::UPLOAD_ERROR, result.Type());

  // Patch error
  result = client.PatchModel(modelId, headers);
  EXPECT_EQ(ResultType::PATCH_ERROR, result.Type());
}
