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
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <gz/common/Filesystem.hh>
#include <gz/utils/ExtraTestMacros.hh>

#include "gz.hh"
#include "test_config.hh"

using namespace gz;

// Std streams
static std::streambuf *g_stdOutFile;
static std::streambuf *g_stdErrFile;

/////////////////////////////////////////////////
class CmdLine : public ::testing::Test
{
  // Documentation inherited
  public: void SetUp() override
  {
    cmdVerbosity("4");

    // Redirect stdout and stderr to streams.
    g_stdOutFile = std::cout.rdbuf(this->stdOutBuffer.rdbuf());
    g_stdErrFile = std::cerr.rdbuf(this->stdErrBuffer.rdbuf());

    this->testCachePath = common::joinPaths(std::string(PROJECT_BINARY_PATH),
        "test_cache");

    // Clear and recreate the cache at the start of every test. Doing it here
    // instead of on teardown leaves the folder intact for debugging if needed
    common::removeAll(testCachePath);
    ASSERT_TRUE(common::createDirectories(testCachePath));
    ASSERT_TRUE(common::createDirectories(
        common::joinPaths(testCachePath, "fuel.gazebosim.org")));
    setenv("GZ_FUEL_CACHE_PATH", this->testCachePath.c_str(), true);
  }

  // Documentation inherited
  public: void TearDown() override
  {
    // Clear all streams (including state flags).
    this->stdOutBuffer.str("");
    this->stdOutBuffer.clear();
    this->stdErrBuffer.str("");
    this->stdErrBuffer.clear();

    // Restore stdout and stderr redirections.
    std::cout.rdbuf(g_stdOutFile);
    std::cerr.rdbuf(g_stdErrFile);
  }

  /// \brief Buffer for std::cout
  public: std::stringstream stdOutBuffer;

  /// \brief Buffer for std::cerr
  public: std::stringstream stdErrBuffer;

  /// \brief Path for a test cache within the build folder
  public: std::string testCachePath;
};

/////////////////////////////////////////////////
TEST_F(CmdLine, ModelListFail)
{
  EXPECT_FALSE(listModels("fake_url"));

  EXPECT_NE(this->stdOutBuffer.str().find("Invalid URL"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(CmdLine, ModelListConfigServerUgly)
{
  EXPECT_TRUE(listModels("https://fuel.gazebosim.org",
                         "openroboticstest", "true"));

  EXPECT_NE(this->stdOutBuffer.str().find("https://fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_EQ(this->stdOutBuffer.str().find("owners"), std::string::npos)
      << this->stdOutBuffer.str();
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
// https://github.com/gazebosim/gz-fuel-tools/issues/254
TEST_F(CmdLine,
       DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(ModelListConfigServerPretty))
{
  EXPECT_TRUE(listModels("https://staging-fuel.gazebosim.org"));

  EXPECT_NE(this->stdOutBuffer.str().find(
        "https://staging-fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("owners"), std::string::npos)
      << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("models"), std::string::npos)
      << this->stdOutBuffer.str();

  EXPECT_EQ(this->stdOutBuffer.str().find("https://fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_EQ(this->stdOutBuffer.str().find(
      "https://staging-fuel.gazebosim.org/1.0/"), std::string::npos)
      << this->stdOutBuffer.str();
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
// https://github.com/gazebosim/gz-fuel-tools/issues/254
TEST_F(CmdLine,
       DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(ModelListConfigServerPrettyOwner))
{
  EXPECT_TRUE(listModels("https://staging-fuel.gazebosim.org",
      "openrobotics"));

  EXPECT_NE(this->stdOutBuffer.str().find(
        "https://staging-fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("1 owners"), std::string::npos)
      << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("models"), std::string::npos)
      << this->stdOutBuffer.str();

  // If pagination fails, we only get the first 20 models
  EXPECT_EQ(this->stdOutBuffer.str().find("20 models"), std::string::npos)
      << this->stdOutBuffer.str();

  EXPECT_EQ(this->stdOutBuffer.str().find("https://fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_EQ(this->stdOutBuffer.str().find(
      "https://staging-fuel.gazebosim.org/1.0/"), std::string::npos)
      << this->stdOutBuffer.str();
}

/////////////////////////////////////////////////
TEST_F(CmdLine, ModelDownloadBadUrl)
{
  EXPECT_FALSE(downloadUrl("fake_url"));

  EXPECT_NE(this->stdOutBuffer.str().find("Malformed URL"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty());
}

/////////////////////////////////////////////////
TEST_F(CmdLine, ModelDownloadWrongUrl)
{
  EXPECT_FALSE(downloadUrl(
      "https://site.com/1.0/ownername/modelname"));

  EXPECT_NE(this->stdOutBuffer.str().find("Invalid URL"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty());
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(CmdLine, ModelDownloadUnversioned)
{
  // Download
  EXPECT_TRUE(downloadUrl(
    "https://fuel.gazebosim.org/1.0/openroboticstest/models/test box"));

  // Check output
  EXPECT_NE(this->stdOutBuffer.str().find("Download succeeded"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();

  // Check files
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "test box")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "test box", "1")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "test box", "1", "model.sdf")));
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(CmdLine, DownloadConfigCache)
{
  unsetenv("GZ_FUEL_CACHE_PATH");

  // Test config
  std::ofstream ofs;
  auto testPath = common::joinPaths(this->testCachePath, "test_conf.yaml");
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    url: https://fuel.gazebosim.org"  << std::endl
      << ""                                       << std::endl
      << "cache:"                                 << std::endl
      << "  path: " << this->testCachePath << std::endl
      << std::endl;
  ofs.close();
  EXPECT_TRUE(common::exists(testPath)) << testPath;

  // Download
  EXPECT_TRUE(downloadUrl(
      "https://fuel.gazebosim.org/1.0/openroboticstest/models/Test box",
      testPath.c_str()));

  // Check output
  EXPECT_NE(this->stdOutBuffer.str().find("Download succeeded"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();

  // Check files
  auto modelPath = common::joinPaths(this->testCachePath,
      "fuel.gazebosim.org", "openroboticstest", "models", "test box");
  EXPECT_TRUE(common::isDirectory(modelPath));
  EXPECT_TRUE(common::isDirectory(common::joinPaths(modelPath, "1")));
  EXPECT_TRUE(common::isFile(common::joinPaths(modelPath, "1", "model.sdf")));
}

/////////////////////////////////////////////////
TEST_F(CmdLine, WorldListFail)
{
  EXPECT_FALSE(listWorlds("fake_url"));

  EXPECT_NE(this->stdOutBuffer.str().find("Invalid URL"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty());
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
// https://github.com/gazebosim/gz-fuel-tools/issues/254
TEST_F(CmdLine,
       DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(WorldListConfigServerUgly))
{
  EXPECT_TRUE(listWorlds(
        "https://staging-fuel.gazebosim.org", "", "true"));

  EXPECT_NE(this->stdOutBuffer.str().find(
        "https://staging-fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_EQ(this->stdOutBuffer.str().find("owners"), std::string::npos)
      << this->stdOutBuffer.str();
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
// https://github.com/gazebosim/gz-fuel-tools/issues/254
TEST_F(CmdLine,
       DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(WorldListConfigServerPretty))
{
  EXPECT_TRUE(listWorlds("https://staging-fuel.gazebosim.org"));

  EXPECT_NE(this->stdOutBuffer.str().find(
        "https://staging-fuel.gazebosim.org"), std::string::npos)
    << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("owners"), std::string::npos)
      << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("worlds"), std::string::npos)
      << this->stdOutBuffer.str();

  EXPECT_EQ(this->stdOutBuffer.str().find("https://fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_EQ(this->stdOutBuffer.str().find(
      "https://staging-fuel.gazebosim.org/1.0/"), std::string::npos)
      << this->stdOutBuffer.str();
}

/////////////////////////////////////////////////
TEST_F(CmdLine, WorldListCustomServerPrettyOwner)
{
  EXPECT_TRUE(listWorlds("https://staging-fuel.gazebosim.org",
      "openrobotics"));

  EXPECT_NE(this->stdOutBuffer.str().find(
        "https://staging-fuel.gazebosim.org"), std::string::npos)
    << this->stdOutBuffer.str();
  EXPECT_NE(this->stdOutBuffer.str().find("worlds"), std::string::npos)
      << this->stdOutBuffer.str();

  // If pagination fails, we only get the first 20 worlds
  EXPECT_EQ(this->stdOutBuffer.str().find("20 worlds"), std::string::npos)
      << this->stdOutBuffer.str();

  EXPECT_EQ(this->stdOutBuffer.str().find("https://fuel.gazebosim.org"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_EQ(this->stdOutBuffer.str().find(
      "https://staging-fuel.gazebosim.org/1.0/"), std::string::npos)
      << this->stdOutBuffer.str();
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(CmdLine, WorldDownloadBadUrl)
{
  EXPECT_FALSE(downloadUrl("fake_url"));

  EXPECT_NE(this->stdOutBuffer.str().find("Malformed URL"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();
}

/////////////////////////////////////////////////
TEST_F(CmdLine, WorldDownloadWrongUrl)
{
  EXPECT_FALSE(downloadUrl(
      "https://site.com/1.0/ownername/worldname"));

  EXPECT_NE(this->stdOutBuffer.str().find("Invalid URL"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();
}

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_F(CmdLine, WorldDownloadUnversioned)
{
  // Download
  EXPECT_TRUE(downloadUrl(
      "https://fuel.gazebosim.org/1.0/openrobotics/worlds/Test world"));

  // Check output
  EXPECT_NE(this->stdOutBuffer.str().find("Download succeeded"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();

  // Check files
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openrobotics", "worlds", "test world")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openrobotics", "worlds", "test world", "2")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openrobotics", "worlds", "test world", "2", "test.sdf")));
}

class DownloadCollectionTest
    : public CmdLine,
      public ::testing::WithParamInterface<int>
{};

INSTANTIATE_TEST_SUITE_P(CollectionTest, DownloadCollectionTest,
                        // cppcheck-suppress syntaxError
                        ::testing::Values(1, 2, 16)); // NOLINT

/////////////////////////////////////////////////
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_P(DownloadCollectionTest, AllItems)
{
  std::string url =
   "https://fuel.gazebosim.org/1.0/openroboticstest/collections/testcollection";

  // Download
  EXPECT_TRUE(downloadUrl(url.c_str(), nullptr, nullptr, nullptr, GetParam()));

  // Check output
  EXPECT_NE(this->stdOutBuffer.str().find("Download succeeded"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();

  // Check files
  // Model: Backpack
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack", "3")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack", "3", "model.sdf")));

  // Model: TEAMBASE
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "teambase")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "teambase", "2")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
     "openroboticstest", "models", "teambase", "2", "model.sdf")));

  // World: Test World
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world", "2")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world", "2", "test.sdf")));

  // World: Test World 2
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2", "1")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2", "1", "test.sdf")));
}

/////////////////////////////////////////////////
/// Download only models
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_P(DownloadCollectionTest, Models)
{
  // Download
  EXPECT_TRUE(
      downloadUrl("https://fuel.gazebosim.org/1.0/openroboticstest/"
                  "collections/testcollection",
                  nullptr, nullptr, "model", GetParam()));

  // Check output
  EXPECT_NE(this->stdOutBuffer.str().find("Download succeeded"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();

  // Check files
  // Model: Backpack
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack", "3")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack", "3", "model.sdf")));

  // Model: TEAMBASE
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "teambase")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "teambase", "2")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "teambase", "2", "model.sdf")));

  // World: Test World
  EXPECT_FALSE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world")));

  // World: Test World 2
  EXPECT_FALSE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2")));
}

/////////////////////////////////////////////////
/// Download only worlds
// Protocol "https" not supported or disabled in libcurl for Windows
// https://github.com/gazebosim/gz-fuel-tools/issues/105
TEST_P(DownloadCollectionTest, Worlds)
{
  // Download
  EXPECT_TRUE(
      downloadUrl("https://fuel.gazebosim.org/1.0/openroboticstest/"
                  "collections/testcollection",
                  nullptr, nullptr, "world", GetParam()));

  // Check output
  EXPECT_NE(this->stdOutBuffer.str().find("Download succeeded"),
      std::string::npos) << this->stdOutBuffer.str();
  EXPECT_TRUE(this->stdErrBuffer.str().empty()) << this->stdErrBuffer.str();

  // Check files
  // Model: Backpack
  EXPECT_FALSE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "backpack")));

  // Model: TEAMBASE
  EXPECT_FALSE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "models", "teambase")));

  // World: Test World
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world", "2")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world", "2", "test.sdf")));

  // World: Test World 2
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2")));
  EXPECT_TRUE(common::isDirectory(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2", "1")));
  EXPECT_TRUE(common::isFile(
    common::joinPaths(this->testCachePath, "fuel.gazebosim.org",
      "openroboticstest", "worlds", "test world2", "1", "test.sdf")));
}
