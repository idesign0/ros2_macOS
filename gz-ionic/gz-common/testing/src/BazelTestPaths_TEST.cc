/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include "gz/common/Filesystem.hh"
#include "gz/common/testing/TestPaths.hh"
#include "gz/common/testing/BazelTestPaths.hh"

using namespace gz::common;

/////////////////////////////////////////////////
TEST(BazelTestPaths, ProjectSourcePath)
{
  gz::common::testing::BazelTestPaths testPaths;

  std::string sourceDir;
  ASSERT_TRUE(testPaths.ProjectSourcePath(sourceDir));
  ASSERT_FALSE(sourceDir.empty());
  ASSERT_TRUE(exists(sourceDir)) << sourceDir;
  ASSERT_TRUE(isDirectory(sourceDir));

  auto installedDir = joinPaths(sourceDir, "testing", "test_files");
  EXPECT_TRUE(exists(installedDir)) << installedDir;
  EXPECT_TRUE(isDirectory(installedDir));

  auto installedFile = joinPaths(installedDir, "example.txt");
  EXPECT_TRUE(exists(installedFile));
  EXPECT_TRUE(isFile(installedFile));
}

/////////////////////////////////////////////////
TEST(BazelTestPaths, TestTmpPath)
{
  gz::common::testing::BazelTestPaths testPaths;

  std::string tmpDir;
  ASSERT_TRUE(testPaths.TestTmpPath(tmpDir));
  ASSERT_FALSE(tmpDir.empty());
  ASSERT_TRUE(exists(tmpDir)) << tmpDir;
  ASSERT_TRUE(isDirectory(tmpDir));
}

/////////////////////////////////////////////////
TEST(BazelTestPaths, TestBuildType)
{
  using BuildType = gz::common::testing::BuildType;
  ASSERT_EQ(BuildType::kBazel, gz::common::testing::TestBuildType());
}

/////////////////////////////////////////////////
TEST(BazelTestPaths, TestPathFactory)
{
  auto testPaths = gz::common::testing::TestPathFactory();
  ASSERT_NE(nullptr, testPaths);
}
