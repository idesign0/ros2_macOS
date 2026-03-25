/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <gz/common/TempDirectory.hh>
#include <gz/utils/ExtraTestMacros.hh>

/////////////////////////////////////////////////
TEST(TempDirectory, tempDirectoryPath)
{
  // OS TMPDIR should never be empty
  ASSERT_FALSE(gz::common::tempDirectoryPath().empty());
}

/////////////////////////////////////////////////
TEST(TempDirectory, createTempDirectory)
{
  // OS TMPDIR should never be empty
  auto tmpDir = gz::common::tempDirectoryPath();
  ASSERT_FALSE(tmpDir.empty());

  // Nominal case
  // eg /tmp/fooXXXXXX
  auto tmp = gz::common::createTempDirectory("foo", tmpDir);
  ASSERT_FALSE(tmp.empty());
  EXPECT_NE(std::string::npos, tmp.find("foo"));
  EXPECT_NE(std::string::npos, tmp.find(tmpDir));

  // Should also work for subdirectories
  // eg /tmp/bar/fooXXXXXX
  tmpDir = gz::common::joinPaths(tmpDir, "bar");
  auto tmp2 = gz::common::createTempDirectory("bar", tmpDir);
  ASSERT_FALSE(tmp2.empty());
}

/////////////////////////////////////////////////
TEST(TempDirectory, createTempDirectoryEmptyBase)
{
  // OS TMPDIR should never be empty
  auto tmpDir = gz::common::tempDirectoryPath();
  ASSERT_FALSE(tmpDir.empty());

  // Create with an empty basename (eg /tmp/XXXXXX)
  auto tmp = gz::common::createTempDirectory("", tmpDir);
  ASSERT_FALSE(tmp.empty());
  EXPECT_EQ(std::string::npos, tmp.find("foo"));
  EXPECT_NE(std::string::npos, tmp.find(tmpDir));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectory)
{
  std::string path;
  auto curDir = gz::common::cwd();
  {
    gz::common::TempDirectory tmp("temp_dir", "gz", true);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_TRUE(tmp.DoCleanup());
    // Current directory changed to tempdir
    EXPECT_NE(curDir, gz::common::cwd());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(gz::common::exists(path));
    EXPECT_EQ(path, gz::common::cwd());
  }
  // Current directory changed back to previous
  EXPECT_EQ(curDir, gz::common::cwd());
  EXPECT_FALSE(gz::common::exists(path));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectoryRoot)
{
  std::string path;
  auto curDir = gz::common::cwd();
  {
    auto p = gz::common::tempDirectoryPath();
    gz::common::TempDirectory tmp(p, "temp_dir", "gz", true);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_TRUE(tmp.DoCleanup());
    EXPECT_NE(curDir, gz::common::cwd());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(gz::common::exists(path));
    EXPECT_EQ(path, gz::common::cwd());
  }
  // Current directory changed back to previous
  EXPECT_EQ(curDir, gz::common::cwd());
  EXPECT_FALSE(gz::common::exists(path));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectoryEmptyRoot)
{
  std::string path;
  auto curDir = gz::common::cwd();
  {
    gz::common::TempDirectory tmp("", "temp_dir", "gz", true);
    EXPECT_FALSE(tmp.Valid());
    EXPECT_TRUE(tmp.DoCleanup());
    // Since not successfully created, no update
    EXPECT_EQ(curDir, gz::common::cwd());
    path = tmp.Path();
    EXPECT_TRUE(path.empty());
    EXPECT_FALSE(gz::common::exists(path));
  }
  EXPECT_EQ(curDir, gz::common::cwd());
  EXPECT_FALSE(gz::common::exists(path));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectoryNoClean)
{
  std::string path;
  auto curDir = gz::common::cwd();
  {
    gz::common::TempDirectory tmp("temp_dir", "gz", false);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_FALSE(tmp.DoCleanup());
    EXPECT_NE(curDir, gz::common::cwd());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(gz::common::exists(path));
    EXPECT_EQ(path, gz::common::cwd());
  }
  // Current directory always changes back, regardless of doClean
  EXPECT_EQ(curDir, gz::common::cwd());
  EXPECT_TRUE(gz::common::exists(path));
  EXPECT_TRUE(gz::common::removeDirectory(path));
}

/////////////////////////////////////////////////
TEST(TempDirectory, TempDirectoryNoCleanLater)
{
  std::string path;
  auto curDir = gz::common::cwd();
  {
    gz::common::TempDirectory tmp("temp_dir", "gz", true);
    EXPECT_TRUE(tmp.Valid());
    EXPECT_TRUE(tmp.DoCleanup());
    EXPECT_NE(curDir, gz::common::cwd());
    path = tmp.Path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(gz::common::exists(path));
    EXPECT_EQ(path, gz::common::cwd());
    tmp.DoCleanup(false);
    EXPECT_FALSE(tmp.DoCleanup());
  }
  // Current directory always changes back, regardless of doClean
  EXPECT_EQ(curDir, gz::common::cwd());
  EXPECT_TRUE(gz::common::exists(path));
  EXPECT_TRUE(gz::common::removeDirectory(path));
}
