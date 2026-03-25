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
#include "gz/common/testing/CMakeTestPaths.hh"

using namespace gz::common;
using namespace gz::common::testing;

constexpr char kFakeTestPaths[] = "";

/////////////////////////////////////////////////
TEST(TestPaths, ProjectSourcePathUnset)
{
  EXPECT_EQ(BuildType::kUnknown, TestBuildType(kFakeTestPaths));

  auto testPaths = TestPathFactory(kFakeTestPaths);
  EXPECT_EQ(nullptr, testPaths);

  auto tempDir = MakeTestTempDirectoryImpl(kFakeTestPaths);
  EXPECT_EQ(nullptr, tempDir);
}

/////////////////////////////////////////////////
TEST(CMakeTestPaths, TestingProjectSourceDir)
{
  ASSERT_NE(0u, strlen(gz::common::testing::kTestingProjectSourceDir));
}

/////////////////////////////////////////////////
TEST(CMakeTestPaths, ProjectSourcePathUnset)
{
  gz::common::testing::CMakeTestPaths testPaths(kFakeTestPaths);
  std::string sourceDir;
  EXPECT_FALSE(testPaths.ProjectSourcePath(sourceDir));
  EXPECT_TRUE(sourceDir.empty());
}

/////////////////////////////////////////////////
TEST(CMakeTestPaths, TestBuildType)
{
  ASSERT_EQ(BuildType::kCMake, gz::common::testing::TestBuildType());
}

/////////////////////////////////////////////////
TEST(CMakeTestPaths, ProjectSourcePath)
{
  gz::common::testing::CMakeTestPaths testPaths;

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
TEST(CMakeTestPaths, TestTmpPath)
{
  gz::common::testing::CMakeTestPaths testPaths;

  std::string tmpDir;
  ASSERT_TRUE(testPaths.TestTmpPath(tmpDir));
  ASSERT_FALSE(tmpDir.empty());
  ASSERT_TRUE(exists(tmpDir)) << tmpDir;
  ASSERT_TRUE(isDirectory(tmpDir));
}
