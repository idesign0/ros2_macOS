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

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>

#include "gz/fuel_tools/Zip.hh"
#include "test_config.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
class ZipIntegrationTest : public ::testing::Test
{
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);
  }
};

/////////////////////////////////////////////////
// Test compressing files in a directory into a zip archive
TEST_F(ZipIntegrationTest, Extract)
{
  // extract a compressed archive
  std::string dst = std::string(PROJECT_BINARY_PATH);
  common::changeFromUnixPath(dst);
  std::string archivePath = common::joinPaths(std::string(TEST_PATH), "media",
      "box.zip");
  EXPECT_TRUE(Zip::Extract(archivePath, dst));

  // check extracted contents
  std::string boxPath = common::joinPaths(dst, "box");
  EXPECT_TRUE(gz::common::exists(boxPath)) << boxPath;
  EXPECT_TRUE(gz::common::isDirectory(boxPath)) << boxPath;

  std::string boxFilePath = common::joinPaths(boxPath, "file");
  EXPECT_TRUE(gz::common::exists(boxFilePath)) << boxFilePath;
  EXPECT_TRUE(gz::common::isFile(boxFilePath)) << boxFilePath;

  std::string boxDirPath = common::joinPaths(boxPath, "dir");
  EXPECT_TRUE(gz::common::exists(boxDirPath)) << boxDirPath;
  EXPECT_TRUE(gz::common::isDirectory(boxDirPath)) << boxDirPath;

  std::string boxFile2Path = common::joinPaths(boxDirPath, "file2");
  EXPECT_TRUE(gz::common::exists(boxFile2Path)) << boxFile2Path;
  EXPECT_TRUE(gz::common::isFile(boxFile2Path));

  // cleanup
  EXPECT_TRUE(gz::common::removeAll(boxPath));
  EXPECT_FALSE(gz::common::exists(boxPath));
}

/////////////////////////////////////////////////
// Test extracting a zip archive
TEST_F(ZipIntegrationTest, Compress)
{
  // compress a directory into a zip archive
  std::string archivePath = common::joinPaths(
      std::string(PROJECT_BINARY_PATH), "box.zip");
  std::string src = common::joinPaths(std::string(TEST_PATH), "media", "box");
  EXPECT_TRUE(Zip::Compress(src, archivePath));

  EXPECT_TRUE(gz::common::exists(archivePath));
  EXPECT_TRUE(gz::common::isFile(archivePath));

  // extract the zip archive and verify contents
  std::string dst = std::string(PROJECT_BINARY_PATH);
  EXPECT_TRUE(Zip::Extract(archivePath, dst));

  // check extracted contents
  std::string boxPath = common::joinPaths(dst, "box");
  EXPECT_TRUE(gz::common::exists(boxPath));
  EXPECT_TRUE(gz::common::isDirectory(boxPath));

  std::string boxFilePath = common::joinPaths(boxPath, "file");
  EXPECT_TRUE(gz::common::exists(boxFilePath));
  EXPECT_TRUE(gz::common::isFile(boxFilePath));

  std::string boxDirPath = common::joinPaths(boxPath, "dir");
  EXPECT_TRUE(gz::common::exists(boxDirPath));
  EXPECT_TRUE(gz::common::isDirectory(boxDirPath));

  std::string boxFile2Path = common::joinPaths(boxDirPath, "file2");
  EXPECT_TRUE(gz::common::exists(boxFile2Path));
  EXPECT_TRUE(gz::common::isFile(boxFile2Path));

  // cleanup
  EXPECT_TRUE(gz::common::removeAll(archivePath));
  EXPECT_FALSE(gz::common::exists(archivePath));

  EXPECT_TRUE(gz::common::removeAll(boxPath));
  EXPECT_FALSE(gz::common::exists(boxPath));
}
