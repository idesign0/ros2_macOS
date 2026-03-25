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

#include "gz/common/testing/BazelTestPaths.hh"

#include <gz/common/Console.hh>
#include <gz/common/Util.hh>

namespace gz::common::testing
{

//////////////////////////////////////////////////
BazelTestPaths::~BazelTestPaths() = default;

//////////////////////////////////////////////////
bool BazelTestPaths::ProjectSourcePath(std::string &_sourceDir)
{
  std::string test_srcdir, bazel_path;

  if (common::env("TEST_SRCDIR", test_srcdir))
  {
    // bzlmod puts run files in _main instead of workspace name
    _sourceDir = gz::common::joinPaths(test_srcdir, "_main");
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////
bool BazelTestPaths::TestTmpPath(std::string &_tmpDir)
{
  return common::env("TEST_UNDECLARED_OUTPUTS_DIR", _tmpDir);
}

}  // namespace gz::common::testing
