/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

#include "gz/common/Util.hh"
#include "gz/common/StringUtils.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/common/TempDirectory.hh"

#ifdef _WIN32
  #define snprintf _snprintf
#endif

using namespace gz;

const char kPluginPath[] = "GZ_PLUGIN_PATH";
const char kFilePath[] = "GZ_FILE_PATH";

class TestTempDirectory : public common::TempDirectory
{
  public: TestTempDirectory():
          common::TempDirectory("systempaths", "gz_common", true)
  {
  }
};

class SystemPathsFixture : public ::testing::Test
{
  // Documentation inherited
  public: virtual void SetUp()
    {
      this->temp = std::make_unique<TestTempDirectory>();
      ASSERT_TRUE(this->temp->Valid());

      common::env(kPluginPath, this->backupPluginPath);
      common::unsetenv(kPluginPath);

      common::env(kFilePath, this->backupFilePath);
      common::unsetenv(kFilePath);

#ifdef _WIN32
      this->filesystemRoot = "C:\\";
#else
      this->filesystemRoot = "/";
#endif
    }

  // Documentation inherited
  public: virtual void TearDown()
    {
      common::setenv(kPluginPath, this->backupPluginPath);
      common::setenv(kFilePath, this->backupFilePath);
      this->temp.reset();
    }

  /// \brief Backup of plugin paths to be restored after the test
  public: std::string backupPluginPath;

  /// \brief Backup of file paths to be restored after the test
  public: std::string backupFilePath;

  /// \brief Root of filesystem according to each platform
  public: std::string filesystemRoot;

  /// \brief Temporary directory to execute test in
  public: std::unique_ptr<TestTempDirectory> temp;
};

std::string SystemPathsJoin(const std::vector<std::string> &_paths)
{
  return common::Join(_paths, common::SystemPaths::Delimiter());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SystemPaths)
{
  common::setenv(kPluginPath,
      SystemPathsJoin({"/tmp/plugin", "/test/plugin/now"}));

  common::SystemPaths paths;
  const std::list<std::string> pathList3 = paths.PluginPaths();
  EXPECT_EQ(static_cast<unsigned int>(2), pathList3.size());
  EXPECT_STREQ("/tmp/plugin/", pathList3.front().c_str());
  EXPECT_STREQ("/test/plugin/now/", pathList3.back().c_str());

  paths.ClearPluginPaths();

  EXPECT_EQ(static_cast<unsigned int>(2), paths.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, FileSystemPaths)
{
  std::vector<std::string> filePaths {"/tmp/file",
                                      "/test/file/now"};
  common::setenv(kFilePath, SystemPathsJoin(filePaths));

  common::SystemPaths paths;
  EXPECT_EQ(kFilePath, paths.FilePathEnv());
  const std::list<std::string> pathList3 = paths.FilePaths();
  EXPECT_EQ(static_cast<unsigned int>(2), pathList3.size());
  EXPECT_STREQ("/tmp/file/", pathList3.front().c_str());
  EXPECT_STREQ("/test/file/now/", pathList3.back().c_str());

  paths.ClearFilePaths();

  EXPECT_EQ(static_cast<unsigned int>(0), paths.FilePaths().size());

  std::string dir1 = "test_dir1";
  common::createDirectories(dir1);
  auto file1 = common::copyFromUnixPath(
      common::joinPaths(dir1, "test_f1"));
  std::ofstream fout;
  fout.open(file1, std::ofstream::out);
  fout << "asdf";
  fout.close();

  EXPECT_EQ("", paths.FindFile("no_such_file"));
  EXPECT_EQ("", paths.FindFile("test_f1"));

  filePaths.push_back("test_dir1");
  common::setenv(kFilePath, SystemPathsJoin(filePaths));
  paths.SetFilePathEnv(kFilePath);
  EXPECT_EQ(file1, paths.FindFile("test_f1"));
  EXPECT_EQ(file1, paths.FindFile("model://test_f1"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, InitialNoSearchPaths)
{
  common::SystemPaths sp;
  EXPECT_EQ(0u, sp.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, AddOneSearchPath)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("./");
  auto paths = sp.PluginPaths();
  EXPECT_EQ(1, std::count(paths.begin(), paths.end(), "./"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, ClearSearchPaths)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("./");
  EXPECT_EQ(1u, sp.PluginPaths().size());
  sp.ClearPluginPaths();
  EXPECT_EQ(0u, sp.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SearchPathGetsTrailingSlash)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("/usr/local/lib");
  auto paths = sp.PluginPaths();
  EXPECT_EQ(1, std::count(paths.begin(), paths.end(), "/usr/local/lib/"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SearchPathUsesForwardSlashes)
{
#ifdef _WIN32
  std::string before = "C:\\user\\alice\\gazebolibs\\";
  std::string after = "C:/user/alice/gazebolibs/";
#else
  std::string before = "\\usr\\lib\\";
  std::string after = "/usr/lib/";
#endif
  common::SystemPaths sp;
  sp.AddPluginPaths(before);
  auto paths = sp.PluginPaths();
  EXPECT_EQ(after, *paths.begin());
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, FindFileURI)
{
  auto dir1 = common::absPath("test_dir1");
  auto dir2 = common::absPath("test_dir2");
  common::createDirectories(dir1);
  common::createDirectories(dir2);
  auto file1 = common::absPath(
      common::joinPaths(dir1, "test_f1"));
  auto file2 = common::absPath(
      common::joinPaths(dir2, "test_f2"));

  std::ofstream fout;
  fout.open(file1, std::ofstream::out);
  fout << "asdf";
  fout.close();
  fout.open(file2, std::ofstream::out);
  fout << "asdf";
  fout.close();

  common::SystemPaths sp;
  auto filesystemRootUnix = common::copyToUnixPath(
          this->filesystemRoot);

  EXPECT_EQ("", sp.FindFileURI("file://no_such_file"));
  EXPECT_EQ(file1, sp.FindFileURI("file:test_dir1/test_f1"));
  EXPECT_EQ(file1, sp.FindFileURI("file://" +
                                  common::copyToUnixPath(file1)));
  EXPECT_EQ("", sp.FindFileURI("osrf://unknown.protocol"));
  EXPECT_EQ("", sp.FindFileURI(this->filesystemRoot + "no_such_file"));
  EXPECT_EQ("", sp.FindFileURI("file://" + filesystemRootUnix +
                               "no_such_file"));
  EXPECT_EQ("", sp.FindFileURI("not_an_uri"));

#ifdef _WIN32
  EXPECT_EQ(this->filesystemRoot + "Windows",
            sp.FindFileURI("file://" + filesystemRootUnix + "Windows"));
  // TODO(anyone): This test should not work,
  // because 'file://C:\\Windows' is not a
  // valid URI. However, until the URI class is upgraded to resolve this as
  // invalid URI, this test has to be commented out.
  // EXPECT_EQ("",
  //     sp.FindFileURI("file://" + this->filesystemRoot + "Windows"));
#endif

  auto robotCb = [dir1](const common::URI &_uri)
  {
    return _uri.Scheme() == "robot" ?
           common::joinPaths(dir1, _uri.Path().Str()) : "";
  };
  auto osrfCb = [dir2](const common::URI &_uri)
  {
    return _uri.Scheme() == "osrf" ?
           common::joinPaths(dir2, gz::common::copyFromUnixPath(
           _uri.Path().Str())) : "";
  };
  auto robot2Cb = [dir2](const common::URI &_uri)
  {
    return _uri.Scheme() == "robot" ?
           common::joinPaths(dir2, _uri.Path().Str()) : "";
  };

  EXPECT_EQ("", sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ("", sp.FindFileURI("osrf://test_f2"));

  sp.AddFindFileURICallback(robotCb);
  EXPECT_EQ(file1, sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ("", sp.FindFileURI("osrf://test_f2"));

  sp.AddFindFileURICallback(osrfCb);
  EXPECT_EQ(file1, sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ(file2, sp.FindFileURI("osrf://test_f2"));

  // Test that th CB from SetFindFileURICallback is called first even when a
  // second handler for the same protocol is available
  sp.AddFindFileURICallback(robot2Cb);
  EXPECT_EQ(file1, sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ(file2, sp.FindFileURI("osrf://test_f2"));

  // URI + env var
  common::setenv(kFilePath, dir1);
  sp.SetFilePathEnv(kFilePath);
  EXPECT_EQ(kFilePath, sp.FilePathEnv());
  EXPECT_EQ(file1, sp.FindFileURI("anything://test_f1"));
  EXPECT_NE(file2, sp.FindFileURI("anything://test_f2"));

  std::string newEnv{"GZ_NEW_FILE_PATH"};
  common::setenv(newEnv, dir2);
  sp.SetFilePathEnv(newEnv);
  EXPECT_EQ(newEnv, sp.FilePathEnv());
  EXPECT_NE(file1, sp.FindFileURI("anything://test_f1"));
  EXPECT_EQ(file2, sp.FindFileURI("anything://test_f2"));
}

TEST_F(SystemPathsFixture, ClearFindFileURICallbacks)
{
  auto testCb = [this](const common::URI &)
  {
    return this->temp->Path();
  };
  common::SystemPaths sp;
  sp.AddFindFileURICallback(testCb);
  EXPECT_EQ(this->temp->Path(), sp.FindFileURI("test://test_f1"));

  sp.ClearFindFileURICallbacks();
  EXPECT_EQ("", sp.FindFileURI("test://test_f1"));
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, FindFile)
{
  auto dir1 = common::absPath("test_dir1");
  auto dir2 = common::absPath("test_dir2");
  common::createDirectories(dir1);
  common::createDirectories(dir2);
  auto file1 = common::absPath(
      common::joinPaths(dir1, "test_f1"));
  auto file2 = common::absPath(
      common::joinPaths(dir2, "test_f2"));

  std::ofstream fout;
  fout.open(file1, std::ofstream::out);
  fout << "asdf";
  fout.close();
  fout.open(file2, std::ofstream::out);
  fout << "asdf";
  fout.close();

  // LocateLocalFile
  common::SystemPaths sp;
  EXPECT_EQ(common::copyToUnixPath(file1),
            sp.LocateLocalFile("test_f1", {dir1, dir2}));

  EXPECT_EQ(common::copyToUnixPath(file2),
            sp.LocateLocalFile("test_f2", {dir1, dir2}));

  EXPECT_EQ("", sp.LocateLocalFile("test_f3", {dir1, dir2}));

  // FindFile
  EXPECT_EQ("", sp.FindFile(this->filesystemRoot + "no_such_file"));
  EXPECT_EQ("", sp.FindFile("no_such_file"));
  EXPECT_EQ(file1, sp.FindFile(common::joinPaths("test_dir1", "test_f1")));

  EXPECT_EQ(file1, sp.FindFile("file:test_dir1/test_f1"));

  // Existing absolute paths
#ifndef _WIN32
  const auto tmpDir = "/tmp";
  const auto uriTmpDir = "file:///tmp";
  const auto homeDir = "/home";
  const auto badDir = "/bad";
  const auto uriBadDir = "file:///bad";
#else
  const auto tmpDir = "C:\\Windows";
  const auto uriTmpDir = "file://C:/Windows";
  const auto homeDir = "C:\\Users";
  const auto badDir = "C:\\bad";
  const auto uriBadDir = "file://C:/bad";
#endif
  {
    EXPECT_EQ(tmpDir, sp.FindFile(uriTmpDir));
    EXPECT_EQ(homeDir, sp.FindFile(homeDir));
    EXPECT_EQ("", sp.FindFile(badDir));
    EXPECT_EQ("", sp.FindFile(uriBadDir));
  }

  // Windows path with only one forward slash
#ifdef _WIN32
  const auto tmpDirForwardSlash = "C:/Windows";
  const auto homeDirForwardSlash = "C:/Users";
  const auto badDirForwardSlash = "C:/bad";
  {
    // We do not expect equality as  sp.FindFile could normalize the
    // path and return a different string
    // The behaviour tested here is just that C:/Windows, C:/Users are found
    EXPECT_NE("", sp.FindFile(tmpDirForwardSlash));
    EXPECT_NE("", sp.FindFile(homeDirForwardSlash));
    EXPECT_EQ("", sp.FindFile(badDirForwardSlash));
  }
#endif

  // Custom callback
  {
    auto homeCb = [homeDir](const std::string &_s)
    {
      return _s == "home" ? homeDir : "";
    };
    auto badCb = [badDir](const std::string &_s)
    {
      return _s == "bad" ? badDir : "";
    };

    EXPECT_EQ("", sp.FindFile("home"));
    EXPECT_EQ("", sp.FindFile("bad"));
    EXPECT_EQ("", sp.FindFile("banana"));

    sp.AddFindFileCallback(homeCb);

    EXPECT_EQ(homeDir, sp.FindFile("home"));
    EXPECT_EQ("", sp.FindFile("bad"));
    EXPECT_EQ("", sp.FindFile("banana"));

    sp.AddFindFileCallback(badCb);

    EXPECT_EQ(homeDir, sp.FindFile("home"));
    EXPECT_EQ("", sp.FindFile("bad"));
    EXPECT_EQ("", sp.FindFile("banana"));
  }
}

TEST_F(SystemPathsFixture, ClearFindFileCallbacks)
{
  auto testCb = [this](const std::string &)
  {
    return this->temp->Path();
  };
  common::SystemPaths sp;
  sp.AddFindFileCallback(testCb);
  EXPECT_EQ(this->temp->Path(), sp.FindFile("test_file"));

  sp.ClearFindFileCallbacks();
  EXPECT_EQ("", sp.FindFile("test_file"));
}
//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, NormalizeDirectoryPath)
{
  EXPECT_EQ(common::SystemPaths::NormalizeDirectoryPath("a/b/c/"),
                                                                  "a/b/c/");
  EXPECT_EQ(common::SystemPaths::NormalizeDirectoryPath("a/b/c"),
                                                                  "a/b/c/");
  EXPECT_EQ(common::SystemPaths::NormalizeDirectoryPath("/a/b/c/"),
                                                                  "/a/b/c/");
  EXPECT_EQ(common::SystemPaths::NormalizeDirectoryPath("/a/b/c"),
                                                                  "/a/b/c/");
  EXPECT_EQ(common::SystemPaths::NormalizeDirectoryPath("a\\b\\c"),
                                                                  "a/b/c/");
  EXPECT_EQ(common::SystemPaths::NormalizeDirectoryPath("a\\b\\c\\"),
                                                                  "a/b/c/");
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, PathsFromEnv)
{
  common::setenv(kPluginPath,
      SystemPathsJoin({"/tmp/plugin", "/test/plugin/now/", "/tmp/plugin"}));

  auto paths = common::SystemPaths::PathsFromEnv(kPluginPath);

  EXPECT_EQ(paths.size(), 2u);

  unsigned int count = 0;
  for (auto const &path : paths)
  {
    if (count == 0)
    {
      EXPECT_EQ(path, "/tmp/plugin/");
    }
    if (count == 1)
    {
      EXPECT_EQ(path, "/test/plugin/now/");
    }
    if (count == 2)
    {
      FAIL();
    }

    ++count;
  }
}
