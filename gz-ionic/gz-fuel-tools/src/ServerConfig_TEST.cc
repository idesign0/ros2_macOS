/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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
#include <string>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/TempDirectory.hh>
#include <gz/common/testing/TestPaths.hh>
#include <gz/common/Util.hh>

#include "gz/fuel_tools/ServerConfig.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
/// \brief Get home directory.
/// \return Home directory or empty string if home wasn't found.
/// \ToDo: Move this function to gz::common::Filesystem
std::string homePath()
{
  std::string homePath;
#ifndef _WIN32
  gz::common::env("HOME", homePath);
#else
  gz::common::env("USERPROFILE", homePath);
#endif
  return homePath;
}

/////////////////////////////////////////////////
class ServerConfigTest: public ::testing::Test
{
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);
    tempDir = gz::common::testing::MakeTestTempDirectory();
    ASSERT_TRUE(tempDir->Valid()) << tempDir->Path();

    gz::common::chdir(tempDir->Path());
  }

  public: std::string cachePath()
  {
    return this->tempDir->Path();
  }

  public: std::shared_ptr<gz::common::TempDirectory> tempDir;
};

/////////////////////////////////////////////////
TEST_F(ServerConfigTest, ApiKey)
{
  ServerConfig config;
  EXPECT_TRUE(config.ApiKey().empty());

  config.SetApiKey("my_api_key");
  EXPECT_EQ("my_api_key", config.ApiKey());

  config.SetApiKey("my_other_api_key");
  EXPECT_EQ("my_other_api_key", config.ApiKey());
}

/////////////////////////////////////////////////
TEST_F(ServerConfigTest, Url)
{
  // Invalid URL string
  {
    ServerConfig srv;
    srv.SetUrl(common::URI("asdf"));
    EXPECT_TRUE(srv.Url().Str().empty());
  }

  // Valid URL
  {
    ServerConfig srv;
    srv.SetUrl(common::URI("http://banana:8080"));
    EXPECT_EQ("http://banana:8080", srv.Url().Str());
    EXPECT_EQ("http", srv.Url().Scheme());
    EXPECT_EQ("banana:8080", srv.Url().Path().Str());
    EXPECT_FALSE(srv.Url().Authority());
  }

  // Trailing /
  {
    ServerConfig srv;
    srv.SetUrl(common::URI("http://banana:8080"));
    EXPECT_EQ("http://banana:8080", srv.Url().Str());
    EXPECT_EQ("http", srv.Url().Scheme());
    EXPECT_EQ("banana:8080", srv.Url().Path().Str());
    EXPECT_FALSE(srv.Url().Authority());
  }

  // Set from URI
  {
    auto url = common::URI();
    url.SetScheme("http");
    url.Path() = common::URIPath("banana:8080");

    ServerConfig srv;
    srv.SetUrl(url);
    EXPECT_EQ("http://banana:8080", srv.Url().Str());
    EXPECT_EQ("http", srv.Url().Scheme());
    EXPECT_EQ("banana:8080", srv.Url().Path().Str());
    EXPECT_FALSE(srv.Url().Authority());
  }
}
