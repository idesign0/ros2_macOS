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

// All these helper functions have been copied from
// https://github.com/gazebosim/gz-common/raw/ign-common3/src/Filesystem_TEST.cc

#ifndef _WIN32
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

/////////////////////////////////////////////////
bool createAndSwitchToTempDir(std::string &_newTempPath)
{
  std::string tmppath;
  const char *tmp = getenv("TMPDIR");
  if (tmp)
  {
    tmppath = std::string(tmp);
  }
  else
  {
    tmppath = std::string("/tmp");
  }

  tmppath += "/XXXXXX";

  char *dtemp = mkdtemp(const_cast<char *>(tmppath.c_str()));
  if (dtemp == nullptr)
  {
    return false;
  }
  if (chdir(dtemp) < 0)
  {
    return false;
  }

  char resolved[PATH_MAX];
  if (realpath(dtemp, resolved) == nullptr)
  {
    return false;
  }

  _newTempPath = std::string(resolved);

  return true;
}

/////////////////////////////////////////////////
bool createNewEmptyFile(const std::string &_filename)
{
  int fd = open(_filename.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd < 0)
  {
    return false;
  }

  close(fd);

  return true;
}

#else
#include <windows.h>
#include <winnt.h>
#include <time.h>
#include <cstdint>
#include <string>  // NOLINT

/////////////////////////////////////////////////
bool createAndSwitchToTempDir(std::string &_newTempPath)
{
  char tempPath[MAX_PATH + 1];
  DWORD pathLen = ::GetTempPathA(MAX_PATH, tempPath);
  if (pathLen >= MAX_PATH || pathLen <= 0)
  {
    return false;
  }
  std::string pathToCreate(tempPath);
  srand(static_cast<uint32_t>(time(nullptr)));

  for (int count = 0; count < 50; ++count)
  {
    // Try creating a new temporary directory with a randomly generated name.
    // If the one we chose exists, keep trying another path name until we reach
    // some limit.
    std::string newDirName;
    newDirName.append(std::to_string(::GetCurrentProcessId()));
    newDirName.push_back('_');
    // On Windows, rand_r() doesn't exist as an alternative to rand(), so the
    // cpplint warning is spurious.  This program is not multi-threaded, so
    // it is safe to suppress the threadsafe_fn warning here.
    newDirName.append(
       std::to_string(rand()    // NOLINT(runtime/threadsafe_fn)
                      % ((int16_t)0x7fff)));

    pathToCreate += newDirName;
    if (::CreateDirectoryA(pathToCreate.c_str(), nullptr))
    {
      _newTempPath = pathToCreate;
      return ::SetCurrentDirectoryA(_newTempPath.c_str()) != 0;
    }
  }

  return false;
}

/////////////////////////////////////////////////
bool createNewEmptyFile(const std::string &_filename)
{
  return ::CreateFileA(_filename.c_str(),
                       FILE_READ_DATA,
                       FILE_SHARE_READ,
                       nullptr,
                       OPEN_ALWAYS,
                       0,
                       nullptr) != INVALID_HANDLE_VALUE;
}

#endif

#include <gtest/gtest.h>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include "gz/fuel_tools/Zip.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
class ZipTest : public ::testing::Test
{
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);
  }
};

/////////////////////////////////////////////////
/// \brief Test basic API
TEST_F(ZipTest, API)
{
  // Extract invalid paths
  EXPECT_FALSE(Zip::Extract("", ""));
  EXPECT_FALSE(Zip::Extract("aaa.zip", "/tmp"));

  // Compress invalid paths
  EXPECT_FALSE(Zip::Extract("", ""));
  EXPECT_FALSE(Zip::Compress("aaa", "aaa.zip"));
}

/////////////////////////////////////////////////
/// \brief Test compress and extract
TEST_F(ZipTest, CompressAndExtract)
{
  std::string newTempDir;
  ASSERT_TRUE(createAndSwitchToTempDir(newTempDir));
  auto d = gz::common::joinPaths(newTempDir, "d1", "d2");
  ASSERT_TRUE(gz::common::createDirectories(d));
  auto f = gz::common::joinPaths(d, "new_file");
  ASSERT_TRUE(createNewEmptyFile(f));
  EXPECT_TRUE(gz::common::exists(f));

  // Compress.
  auto d1 = gz::common::joinPaths(newTempDir, "d1");
  auto zipOutFile = gz::common::joinPaths(newTempDir, "new_file.zip");
  EXPECT_TRUE(Zip::Compress(d1, zipOutFile));
  EXPECT_TRUE(gz::common::exists(zipOutFile));

  // Extract.
  auto extractOutDir = gz::common::joinPaths(newTempDir, "extract");
  EXPECT_TRUE(Zip::Extract(zipOutFile, extractOutDir));
  auto extractOutFile =
    gz::common::joinPaths(extractOutDir, "d1", "d2", "new_file");
  EXPECT_TRUE(gz::common::exists(extractOutFile));

  // Clean.
  gz::common::removeAll(newTempDir);
}
