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
#include <string>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/TempDirectory.hh>
#include <gz/common/testing/TestPaths.hh>
#include <gz/common/Util.hh>

#include "gz/fuel_tools/config.hh"
#include "gz/fuel_tools/ClientConfig.hh"

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
class ClientConfigTest: public ::testing::Test
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
/// \brief Initially only the default server in config
TEST_F(ClientConfigTest, InitiallyDefaultServers)
{
  ClientConfig config;
  EXPECT_EQ(1u, config.Servers().size());
}

/////////////////////////////////////////////////
/// \brief Servers can be added
TEST_F(ClientConfigTest, ServersCanBeAdded)
{
  ClientConfig config;
  ServerConfig srv;
  srv.SetUrl(common::URI("http://asdf"));
  config.AddServer(srv);

  ASSERT_EQ(2u, config.Servers().size());
  EXPECT_EQ(std::string("http://asdf"), config.Servers().back().Url().Str());
}

/////////////////////////////////////////////////
/// \brief We can load the default configuration file.
TEST_F(ClientConfigTest, CustomDefaultConfiguration)
{
  ClientConfig config;
  ASSERT_EQ(1u, config.Servers().size());
  EXPECT_EQ("https://fuel.gazebosim.org",
    config.Servers().front().Url().Str());
  std::string defaultCacheLocation = gz::common::joinPaths(
    homePath(), ".gz", "fuel");
  EXPECT_EQ(defaultCacheLocation, config.CacheLocation());
}

/////////////////////////////////////////////////
/// \brief We can load custom settings in a configuration file.
TEST_F(ClientConfigTest, CustomConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    url: https://api.gazebosim.org"     << std::endl
      << ""                                       << std::endl
      << "  -"                                    << std::endl
      << "    url: https://myserver"              << std::endl
      << ""                                       << std::endl
      << "# Where are the assets stored in disk." << std::endl
      << "cache:"                                 << std::endl
      << "  path: " + cachePath()                 << std::endl
      << std::endl;
  ofs.close();

  EXPECT_TRUE(config.LoadConfig(testPath));

  ASSERT_EQ(3u, config.Servers().size());
  EXPECT_EQ("https://fuel.gazebosim.org",
    config.Servers().front().Url().Str());
  EXPECT_EQ("https://myserver",
    config.Servers().back().Url().Str());

  EXPECT_EQ(cachePath(), config.CacheLocation());
}

/////////////////////////////////////////////////
/// \brief A server contains an already used URL.
TEST_F(ClientConfigTest, RepeatedServerConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    url: https://fuel.gazebosim.org"    << std::endl
      << ""                                       << std::endl
      << "  -"                                    << std::endl
      << "    url: https://fuel.gazebosim.org"    << std::endl
      << ""                                       << std::endl
      << "# Where are the assets stored in disk." << std::endl
      << "cache:"                                 << std::endl
      << "  path: " + cachePath()                 << std::endl
      << std::endl;
  ofs.close();

  EXPECT_TRUE(config.LoadConfig(testPath));
}

/////////////////////////////////////////////////
/// \brief A server without URL is not valid.
TEST_F(ClientConfigTest, NoServerUrlConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    banana: coconut"                           << std::endl
      << std::endl;
  ofs.close();

  EXPECT_FALSE(config.LoadConfig(testPath));
}

/////////////////////////////////////////////////
/// \brief A server with an empty URL is not valid.
TEST_F(ClientConfigTest, EmptyServerUrlConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    url: "                              << std::endl
      << std::endl;
  ofs.close();

  EXPECT_FALSE(config.LoadConfig(testPath));
}

/////////////////////////////////////////////////
/// \brief The "cache" option requires to set "path".
TEST_F(ClientConfigTest, NoCachePathConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"    << std::endl
      << "cache:" << std::endl
      << std::endl;
  ofs.close();

  EXPECT_FALSE(config.LoadConfig(testPath));
}

/////////////////////////////////////////////////
/// \brief The path parameter cannot be empty.
TEST_F(ClientConfigTest, EmptyCachePathConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"     << std::endl
      << "cache:"  << std::endl
      << "  path:" << std::endl
      << std::endl;
  ofs.close();

  EXPECT_FALSE(config.LoadConfig(testPath));
}

/////////////////////////////////////////////////
TEST_F(ClientConfigTest, UserAgent)
{
  ClientConfig config;
  EXPECT_EQ("GazeboFuelTools-" GZ_FUEL_TOOLS_VERSION_FULL,
            config.UserAgent());

  config.SetUserAgent("my_user_agent");
  EXPECT_EQ("my_user_agent", config.UserAgent());
}

/////////////////////////////////////////////////
TEST_F(ClientConfigTest, AsString)
{
  common::Console::SetVerbosity(4);
  {
    ClientConfig client;

    std::string str = client.AsString();
    gzdbg << str << std::endl;

#ifndef _WIN32
    EXPECT_NE(str.find(".gz/fuel"), std::string::npos);
#else
    EXPECT_NE(str.find(".gz\\fuel"), std::string::npos);
#endif
    EXPECT_NE(str.find("https://fuel.gazebosim.org"), std::string::npos);
  }

  {
    ClientConfig client;
    client.Clear();
    std::string str = "Config path: \nCache location: \nServers:\n";
    EXPECT_EQ(str, client.AsString());
  }

  {
    ServerConfig server;
    server.Clear();
    std::string str = "URL: \nVersion: 1.0\nAPI key: \n";
    EXPECT_EQ(str, server.AsString());
  }

  {
    ServerConfig srv;
    srv.SetUrl(common::URI("http://serverurl.com"));
    srv.SetVersion("2.0");
    srv.SetApiKey("ABCD");

    auto str = srv.AsString();
    gzdbg << str << std::endl;

    EXPECT_NE(str.find("http://serverurl.com"), std::string::npos);
    EXPECT_EQ(str.find("local_name"), std::string::npos);
    EXPECT_NE(str.find("2.0"), std::string::npos);
    EXPECT_NE(str.find("ABCD"), std::string::npos);
  }

  {
    ClientConfig client;
    client.SetCacheLocation("cache/location");

    ServerConfig srv;
    srv.SetUrl(common::URI("http://serverurl.com"));
    client.AddServer(srv);

    auto str = client.AsString();
    gzdbg << str << std::endl;

    EXPECT_NE(str.find("cache/location"), std::string::npos);
    EXPECT_NE(str.find("http://serverurl.com"), std::string::npos);
  }
}

/////////////////////////////////////////////////
TEST_F(ClientConfigTest, AsPrettyString)
{
  common::Console::SetVerbosity(4);

  {
    ServerConfig server;
    server.Clear();
    std::string str = "\x1B[96m\x1B[1mVersion: \x1B[0m\x1B[37m1.0\x1B[0m\n";
    EXPECT_EQ(str, server.AsPrettyString());
  }

  {
    ServerConfig srv;
    srv.SetUrl(common::URI("http://serverurl.com"));
    srv.SetVersion("2.0");
    srv.SetApiKey("ABCD");

    auto str = srv.AsPrettyString();
    gzdbg << str << std::endl;

    EXPECT_NE(str.find("http://serverurl.com"), std::string::npos);
    EXPECT_EQ(str.find("local_name"), std::string::npos);
    EXPECT_NE(str.find("2.0"), std::string::npos);
    EXPECT_NE(str.find("ABCD"), std::string::npos);
  }
}
