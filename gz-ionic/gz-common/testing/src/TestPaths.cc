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
#include "gz/common/testing/TestPaths.hh"
#include "gz/common/testing/BazelTestPaths.hh"
#include "gz/common/testing/CMakeTestPaths.hh"

#include <gz/common/Console.hh>
#include <gz/common/Util.hh>

namespace gz::common::testing
{
//////////////////////////////////////////////////
TestPaths::TestPaths(const std::string &_projectSourcePath)
  : projectSourcePath(_projectSourcePath)
{
}

//////////////////////////////////////////////////
TestPaths::~TestPaths() = default;

//////////////////////////////////////////////////
BuildType TestBuildType(const std::string &_projectSourcePath)
{
  std::string gz_bazel;
  bool gz_bazel_set = common::env("GZ_BAZEL", gz_bazel);
  bool gz_cmake_set = !_projectSourcePath.empty();

  if (gz_bazel_set && gz_cmake_set)
  {
    gzwarn << "Detected settings from Bazel and CMake, preferring CMake\n";
  }

  if (gz_cmake_set)
    return BuildType::kCMake;
  else if (gz_bazel_set)
    return BuildType::kBazel;
  else
    return BuildType::kUnknown;
}

//////////////////////////////////////////////////
std::unique_ptr<TestPaths>
TestPathFactory(const std::string &_projectSourcePath)
{
  std::unique_ptr<TestPaths> ret {nullptr};

  switch(TestBuildType(_projectSourcePath))
  {
    case BuildType::kBazel:
      ret = std::make_unique<BazelTestPaths>(_projectSourcePath);
      break;
    case BuildType::kCMake:
      ret = std::make_unique<CMakeTestPaths>(_projectSourcePath);
      break;
    case BuildType::kUnknown:
      ret = nullptr;
      break;
    default:
      ret = nullptr;
      break;
  }
  return ret;
}

//////////////////////////////////////////////////
std::shared_ptr<gz::common::TempDirectory>
MakeTestTempDirectoryImpl(const std::string &_projectSourcePath,
                          const std::string &_prefix,
                          const std::string &_subDir,
                          bool _cleanup)
{
  auto testPaths = TestPathFactory(_projectSourcePath);

  if (!testPaths)
    return nullptr;

  std::string dataDir;
  testPaths->TestTmpPath(dataDir);

  if (dataDir.empty())
    return nullptr;

  return std::make_shared<gz::common::TempDirectory>(
      dataDir, _prefix, _subDir, _cleanup);
}

}  // namespace gz::common::testing
