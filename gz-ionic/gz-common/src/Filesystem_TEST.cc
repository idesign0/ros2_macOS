/*
 * Copyright 2017 Open Source Robotics Foundation
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

#include <gz/utils/ExtraTestMacros.hh>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/TempDirectory.hh>
#include <gz/common/Util.hh>

#include <gz/common/testing/Utils.hh>

#include <filesystem>
#include <fstream>

// The symlink tests should always work on UNIX systems
#ifndef _WIN32
#define BUILD_SYMLINK_TESTS
#endif  // _WIN32

using namespace gz;
using namespace common;

namespace fs = std::filesystem;

/////////////////////////////////////////////////
class TestTempDirectory : public TempDirectory
{
  public: TestTempDirectory():
    TempDirectory("filesystem", "gz_common", true)
  {
  }
};

/////////////////////////////////////////////////
bool create_new_file_symlink(const std::string &_symlink,
                             const std::string &_target)
{
  try
  {
    fs::create_symlink(_target, _symlink);
  }
  catch(const std::exception& e)
  {
    gzerr << "Failed to create link: " << e.what() << '\n';
    return false;
  }
  return true;
}

/////////////////////////////////////////////////
bool create_new_dir_symlink(const std::string &_symlink,
                            const std::string &_target)
{
  try
  {
    fs::create_directory_symlink(_target, _symlink);
  }
  catch(const std::exception& e)
  {
    gzerr << "Failed to create link: " << e.what() << '\n';
    return false;
  }
  return true;
}

/////////////////////////////////////////////////
bool create_new_file_hardlink(const std::string &_hardlink,
                              const std::string &_target)
{
  try
  {
    fs::create_hard_link(_target, _hardlink);
  }
  catch(const std::exception& e)
  {
    gzerr << "Failed to create link: " << e.what() << '\n';
    return false;
  }
  return true;
}

/// \brief Test Filesystem
class FilesystemTest : public ::testing::Test
{
  // Documentation inherited
  protected: void SetUp() override
  {
    Console::SetVerbosity(4);
    this->temp = std::make_unique<TestTempDirectory>();
    ASSERT_TRUE(this->temp->Valid());
  }

  protected: void TearDown() override
  {
    this->temp.reset();
  }

  protected: std::unique_ptr<TestTempDirectory> temp;
};

/////////////////////////////////////////////////
/// \brief Test file operations
TEST_F(FilesystemTest, fileOps)
{
  EXPECT_FALSE(cwd().empty());
  EXPECT_TRUE(exists(cwd()));
  EXPECT_TRUE(common::isDirectory(cwd()));
  EXPECT_FALSE(isFile(cwd()));

  std::ofstream tmpOut("test.tmp");
  tmpOut << "Output" << std::endl;
  tmpOut.close();

  EXPECT_TRUE(isFile("test.tmp"));
  EXPECT_FALSE(common::isDirectory("test.tmp"));

  EXPECT_TRUE(copyFile("test.tmp", "test2.tmp"));
  EXPECT_TRUE(exists("test.tmp"));
  EXPECT_TRUE(exists("test2.tmp"));

  std::ifstream testIn("test.tmp");
  std::string testInContent((std::istreambuf_iterator<char>(testIn)),
                            (std::istreambuf_iterator<char>()));

  std::ifstream test2In("test2.tmp");
  std::string test2InContent((std::istreambuf_iterator<char>(test2In)),
                             (std::istreambuf_iterator<char>()));
  // We must close the file stream for test2.tmp, or else Windows
  // will refuse to remove the file from the filesystem in the
  // upcoming test.
  test2In.close();

  EXPECT_EQ(testInContent, test2InContent);

  EXPECT_TRUE(moveFile("test2.tmp", "test3.tmp"));

  EXPECT_FALSE(exists("test2.tmp"));
  EXPECT_TRUE(exists("test3.tmp"));

  std::ifstream test3In("test3.tmp");
  std::string test3InContent((std::istreambuf_iterator<char>(test3In)),
                             (std::istreambuf_iterator<char>()));
  // We must close this file stream for test3.tmp or else Windows
  // will refuse to remove the file from the filesystem in the
  // upcoming test.
  test3In.close();

  EXPECT_EQ(testInContent, test3InContent);

  // Suppress warnings on the next two, as they are expected to warn when
  // trying to copy from/to the same file
  EXPECT_FALSE(copyFile("test3.tmp", "test3.tmp", FSWO_SUPPRESS_WARNINGS));
  EXPECT_FALSE(copyFile("test3.tmp", "./test3.tmp", FSWO_SUPPRESS_WARNINGS));

  std::remove("test.tmp");

  // This file shouldn't exist, but we'll try to remove just in case the
  // test failed.
  // Suppress warnings since we expect this to be false
  EXPECT_FALSE(removeFile("test2.tmp", FSWO_SUPPRESS_WARNINGS));

  EXPECT_TRUE(removeFile("test3.tmp"));

  EXPECT_FALSE(exists("test3.tmp"));
}

/////////////////////////////////////////////////
/// \brief Test file operations
TEST_F(FilesystemTest, moreFileOps)
{
  EXPECT_FALSE(copyFile("__wrong__.tmp", "test2.tmp", FSWO_SUPPRESS_WARNINGS));
  EXPECT_FALSE(exists("test2.tmp"));
  EXPECT_FALSE(copyFile("test.tmp", "__wrong_dir__/__wrong__.tmp",
        FSWO_SUPPRESS_WARNINGS));
  EXPECT_FALSE(exists("__wrong_dir__"));

  EXPECT_FALSE(moveFile("__wrong__.tmp", "test3.tmp", FSWO_SUPPRESS_WARNINGS));
  EXPECT_FALSE(exists("test3.tmp"));
  EXPECT_FALSE(moveFile("test2.tmp", "__wrong_dir__/__wrong__.tmp",
        FSWO_SUPPRESS_WARNINGS));
  EXPECT_FALSE(exists("__wrong_dir__"));
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, exists)
{
  ASSERT_TRUE(common::testing::createNewEmptyFile("newfile"));
  ASSERT_TRUE(createDirectory("fstestexists"));

  const auto &relativeSubdir = joinPaths("fstestexists", "newfile2");
  const auto &absoluteSubdir = joinPaths(cwd(), relativeSubdir);
  ASSERT_TRUE(common::testing::createNewEmptyFile(relativeSubdir));

  EXPECT_TRUE(exists("fstestexists"));
  EXPECT_TRUE(isDirectory("fstestexists"));

  EXPECT_FALSE(exists("notcreated"));
  EXPECT_FALSE(isDirectory("notcreated"));

  EXPECT_TRUE(exists("newfile"));
  EXPECT_FALSE(isDirectory("newfile"));

  // file in relative subdirectory
  EXPECT_TRUE(exists(relativeSubdir));
  EXPECT_TRUE(isFile(relativeSubdir));
  EXPECT_FALSE(isDirectory(relativeSubdir));

  // file in absolute subdirectory
  EXPECT_TRUE(exists(absoluteSubdir));
  EXPECT_TRUE(isFile(absoluteSubdir));
  EXPECT_FALSE(isDirectory(absoluteSubdir));
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, relative)
{
  #ifndef _WIN32
    std::string absPath {"/tmp/fstest"};
    std::string relPath {"../fstest"};
  #else
    std::string absPath {"C:\\Users\\user\\Desktop\\test.txt"};
    std::string relPath {"user\\Desktop\\test.txt"};
  #endif
  EXPECT_FALSE(isRelativePath(absPath));
  EXPECT_TRUE(isRelativePath(relPath));
}

// The symlink tests require special permissions to work on Windows,
// so they will be disabled by default. For more information, see:
// https://github.com/gazebosim/gz-common/issues/21
#ifdef BUILD_SYMLINK_TESTS
/////////////////////////////////////////////////
TEST_F(FilesystemTest, symlink_exists)
{
  // There are 5 cases we want to test for links (Unix doesn't allow hard links
  // to directories or to non-existent files):
  // 1. symbolic link to existing file
  // 2. symbolic link to non-existent file
  // 3. symbolic link to existing directory
  // 4. symbolic link to non-existent directory
  // 5. hard link to existing file
  ASSERT_TRUE(common::testing::createNewEmptyFile("newfile"));
  ASSERT_TRUE(createDirectory("newdir"));

  // Case 1
  ASSERT_TRUE(create_new_file_symlink("symlink-file", "newfile"));
  EXPECT_TRUE(exists("symlink-file"));
  EXPECT_TRUE(removeFile("symlink-file"));

  // Case 2
  ASSERT_TRUE(create_new_file_symlink("symlink-file-broken", "nonexistent"));
  EXPECT_FALSE(exists("symlink-file-broken"));

  // Case 3
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir", "newdir"));
  EXPECT_TRUE(exists("symlink-dir"));
  EXPECT_TRUE(removeDirectory("symlink-dir"));

  // Case 4
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir-broken", "nonexistent-dir"));
  EXPECT_FALSE(exists("symlink-dir-broken"));

  // Case 5
  ASSERT_TRUE(create_new_file_hardlink("hardlink-file", "newfile"));
  EXPECT_TRUE(exists("hardlink-file"));
  EXPECT_TRUE(removeFile("hardlink-file"));
}
#endif

/////////////////////////////////////////////////
TEST_F(FilesystemTest, cwd)
{
  const auto origCwd = cwd();

  TestTempDirectory new_temp_dir;
  ASSERT_TRUE(new_temp_dir.Valid());

  std::string path = cwd();
  EXPECT_EQ(path, new_temp_dir.Path());

  EXPECT_TRUE(chdir(origCwd));
  EXPECT_EQ(origCwd, cwd());
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, append)
{
  std::string path = "tmp";
  path = joinPaths(path, "hello");

  ASSERT_EQ(path, separator("tmp") + "hello");

  path = joinPaths(path, "there", "again");

  EXPECT_EQ(path, separator("tmp") +
            separator("hello") +
            separator("there") + "again");

  path = joinPaths("base", "/before", "after/");

#ifndef _WIN32
  EXPECT_EQ(path, "base/before/after/");
#else
  EXPECT_EQ(path, "base\\before\\after\\");
#endif

  path = joinPaths("base", "/before", "after///");
#ifndef _WIN32
  EXPECT_EQ(path, "base/before/after/");
#else
  EXPECT_EQ(path, "base\\before\\after\\");
#endif

  path = joinPaths("///base", "/before", "after");
#ifndef _WIN32
  EXPECT_EQ(path, "/base/before/after");
#else
  EXPECT_EQ(path, "\\base\\before\\after");
#endif

  path = joinPaths("/base", "/before", "after");
#ifndef _WIN32
  EXPECT_EQ(path, "/base/before/after");
#else
  EXPECT_EQ(path, "\\base\\before\\after");
#endif

  path = joinPaths("///base", "///before//", "/after///");
#ifndef _WIN32
  EXPECT_EQ(path, "/base/before/after/");
#else
  EXPECT_EQ(path, "\\base\\before\\after\\");
#endif

  path = joinPaths("base", "/before", "after");
#ifndef _WIN32
  EXPECT_EQ(path, "base/before/after");
#else
  EXPECT_EQ(path, "base\\before\\after");
#endif

  // Make sure that the slashes in the middle of string are not altered.
  path = joinPaths("https://fuel.gazebosim.org", "/models", "box");
  EXPECT_EQ(path, "https://fuel.gazebosim.org/models/box");
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, parentPath)
{
  std::string child = joinPaths("path", "to", "a", "child");
  std::string parent = parentPath(child);
  // Expect no separator at the end
  EXPECT_EQ('a', *parent.rbegin());
  // Expect parent string to match beginning of child string
  EXPECT_EQ(parent, child.substr(0, parent.size()));
  // join "a" with "child" and expect to match end of child string
  EXPECT_EQ(separator("a") + "child", child.substr(parent.size()-1));

  std::string child_with_slash = joinPaths("path", "to", "a", "child");
  child_with_slash = separator(child_with_slash);
  std::string parent2 = parentPath(child_with_slash);
  EXPECT_EQ(parent, parent2);

  std::string childOnly = "child";
  EXPECT_EQ(childOnly, parentPath(childOnly));
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(cwd_error))
{
  // This test intentionally creates a directory, switches to it, removes
  // the directory, and then tries to call cwd() on it to cause
  // cwd() to fail.  Windows does not allow you to remove an
  // in-use directory, so this test is restricted to Unix.
  TestTempDirectory new_temp_dir;
  ASSERT_TRUE(new_temp_dir.Valid());
  ASSERT_EQ(rmdir(new_temp_dir.Path().c_str()), 0);
  EXPECT_EQ(cwd(), "");
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, decomposition)
{
  std::string absolute = joinPaths("", "home", "bob", "foo");
  EXPECT_EQ(basename(absolute), "foo");
#ifndef _WIN32
  EXPECT_EQ(parentPath(absolute), "/home/bob");
#else
  EXPECT_EQ(parentPath(absolute), "\\home\\bob");
#endif

  std::string relative = joinPaths("baz", "foobar");
  EXPECT_EQ(basename(relative), "foobar");
  EXPECT_EQ(parentPath(relative), "baz");

  std::string bname = "bzzz";
  EXPECT_EQ(basename(bname), "bzzz");
  EXPECT_EQ(parentPath(bname), "bzzz");

  std::string nobase = joinPaths("baz", "");
  EXPECT_EQ(basename(nobase), "baz");
#ifndef _WIN32
  EXPECT_EQ(parentPath(nobase), "baz/");
#else
  EXPECT_EQ(parentPath(nobase), "baz\\");
#endif

  std::string multiple_slash = separator("baz") + separator("") + separator("")
    + separator("");
  EXPECT_EQ(basename(multiple_slash), "baz");
#ifndef _WIN32
  EXPECT_EQ(parentPath(multiple_slash), "baz//");
#else
  EXPECT_EQ(parentPath(multiple_slash), "baz\\\\");
#endif

  std::string multiple_slash_middle = separator("") + separator("home")
    + separator("") + separator("") + separator("bob") + separator("foo");
  EXPECT_EQ(basename(multiple_slash_middle), "foo");
#ifndef _WIN32
  EXPECT_EQ(parentPath(multiple_slash_middle), "/home///bob");
#else
  EXPECT_EQ(parentPath(multiple_slash_middle), "\\home\\\\\\bob");
#endif

  std::string multiple_slash_start = separator("") + separator("")
    + separator("") + separator("home") + separator("bob") + separator("foo");
  EXPECT_EQ(basename(multiple_slash_start), "foo");
#ifndef _WIN32
  EXPECT_EQ(parentPath(multiple_slash_start), "///home/bob");
#else
  EXPECT_EQ(parentPath(multiple_slash_start), "\\\\\\home\\bob");
#endif

  std::string slash_only = separator("") + separator("");
  EXPECT_EQ(basename(slash_only), separator(""));
  EXPECT_EQ(parentPath(slash_only), "");

  std::string multiple_slash_only = separator("") + separator("")
    + separator("") + separator("");
  EXPECT_EQ(basename(multiple_slash_only), separator(""));
#ifndef _WIN32
  EXPECT_EQ(parentPath(multiple_slash_only), "//");
#else
  EXPECT_EQ(parentPath(multiple_slash_only), "\\\\");
#endif
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, directory_iterator)
{
  ASSERT_TRUE(common::testing::createNewEmptyFile("newfile"));
  ASSERT_TRUE(createDirectory("newdir"));
#ifdef BUILD_SYMLINK_TESTS
  ASSERT_TRUE(create_new_file_symlink("symlink-file", "newfile"));
  ASSERT_TRUE(create_new_file_symlink("symlink-file-broken", "nonexistent"));
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir", "newdir"));
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir-broken", "nonexistent-dir"));
  ASSERT_TRUE(create_new_file_hardlink("hardlink-file", "newfile"));
#endif

  std::set<std::string> found_items;

  DirIter endIter;
  for (DirIter dirIter("."); dirIter != endIter; ++dirIter)
  {
    found_items.insert(basename(*dirIter));
  }

  EXPECT_EQ(found_items.find("."), found_items.end());
  EXPECT_EQ(found_items.find(".."), found_items.end());
  EXPECT_NE(found_items.find("newfile"), found_items.end());
  EXPECT_NE(found_items.find("newdir"), found_items.end());
#ifdef BUILD_SYMLINK_TESTS
  EXPECT_NE(found_items.find("symlink-file"), found_items.end());
  EXPECT_NE(found_items.find("symlink-file-broken"), found_items.end());
  EXPECT_NE(found_items.find("symlink-dir"), found_items.end());
  EXPECT_NE(found_items.find("symlink-dir-broken"), found_items.end());
  EXPECT_NE(found_items.find("hardlink-file"), found_items.end());
#endif

  found_items.clear();
  for (DirIter dirIter(""); dirIter != endIter; ++dirIter)
  {
    found_items.insert(basename(*dirIter));
  }

  EXPECT_EQ(found_items.size(), 0UL);

  found_items.clear();
  for (DirIter dirIter("newDir"); dirIter != endIter; ++dirIter)
  {
    found_items.insert(basename(*dirIter));
  }

  EXPECT_EQ(found_items.size(), 0UL);
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, createDirectories)
{
  // test creating directories using relative path
  std::string rel_dir = separator("rel_dir") + "subdir";
  ASSERT_TRUE(createDirectories(rel_dir));
  EXPECT_TRUE(exists(rel_dir));
  EXPECT_TRUE(isDirectory(rel_dir));

  // test creating directories using abs path
  std::string path = cwd();
  EXPECT_EQ(path, this->temp->Path());

  std::string abs_dir = separator(cwd()) + separator("abs_dir") + "subdir";
  ASSERT_TRUE(createDirectories(abs_dir));
  EXPECT_TRUE(exists(abs_dir));
  EXPECT_TRUE(isDirectory(abs_dir));
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, copyDirectories)
{
  // Create an empty directory
  std::string dirToBeCopied = "dir_to_be_copied";
  EXPECT_FALSE(exists(dirToBeCopied));
  EXPECT_TRUE(createDirectories(dirToBeCopied));
  EXPECT_TRUE(exists(dirToBeCopied));
  EXPECT_TRUE(isDirectory(dirToBeCopied));

  // Copy to a directory
  std::string dirCopied = "dir_copied";
  EXPECT_FALSE(exists(dirCopied));

  EXPECT_TRUE(copyDirectory(dirToBeCopied, dirCopied)) << dirToBeCopied <<
      " -> " << dirCopied;
  EXPECT_TRUE(exists(dirCopied));
  EXPECT_TRUE(isDirectory(dirCopied));

  // Copy to an existing directory - should overwrite
  EXPECT_TRUE(copyDirectory(dirToBeCopied, dirCopied));
  EXPECT_TRUE(exists(dirCopied));
  EXPECT_TRUE(isDirectory(dirCopied));

  // Copy to a directory with non-existent intermediate paths
  std::string intermDirCopy = joinPaths("dir_interm", "dir_copied");
  EXPECT_FALSE(exists(intermDirCopy));

  EXPECT_TRUE(copyDirectory(dirToBeCopied, intermDirCopy));
  EXPECT_TRUE(exists(intermDirCopy));
  EXPECT_TRUE(isDirectory(intermDirCopy));

  // Copy recursively a directory with a directory and a file in it
  std::string subDir = joinPaths(dirToBeCopied, "sub_dir");
  EXPECT_FALSE(exists(subDir));
  EXPECT_TRUE(createDirectories(subDir));
  EXPECT_TRUE(exists(subDir));

  std::string subFile = joinPaths(subDir, "sub_file");
  EXPECT_FALSE(exists(subFile));
  EXPECT_TRUE(common::testing::createNewEmptyFile(subFile));
  EXPECT_TRUE(exists(subFile));

  std::string recDirCopied = "rec_dir_copied";
  EXPECT_TRUE(copyDirectory(dirToBeCopied, recDirCopied));
  EXPECT_TRUE(exists(recDirCopied));
  EXPECT_TRUE(isDirectory(recDirCopied));

  // Non-existent source directory
  EXPECT_FALSE(copyDirectory("fake_dir", dirCopied, FSWO_SUPPRESS_WARNINGS));
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, uniquePaths)
{
  // Directory
  std::string dir = "uniqueDirectory";
  EXPECT_FALSE(exists(dir));
  std::string dirRt = uniqueDirectoryPath(dir);
  EXPECT_EQ(dir, dirRt);

  ASSERT_TRUE(createDirectory(dir));
  EXPECT_TRUE(exists(dir));
  std::string dirExistingRt = uniqueDirectoryPath(dir);
  EXPECT_EQ(dirExistingRt, dir + "(1)");

  ASSERT_TRUE(createDirectory(dirExistingRt));
  EXPECT_TRUE(exists(dirExistingRt));
  std::string dirExistingRt2 = uniqueDirectoryPath(dir);
  EXPECT_EQ(dirExistingRt2, dir + "(2)");

  // File
  std::string newFile = joinPaths(dir, "uniqueFile");
  std::string newFileWithExt = newFile + ".txt";
  EXPECT_FALSE(exists(newFileWithExt));
  std::string fileRt = uniqueFilePath(newFile, "txt");
  EXPECT_EQ(newFileWithExt, fileRt);

  ASSERT_TRUE(common::testing::createNewEmptyFile(newFileWithExt));
  EXPECT_TRUE(exists(newFileWithExt));
  std::string fileExistingRt = uniqueFilePath(newFile, "txt");
  EXPECT_EQ(fileExistingRt, newFile + "(1).txt");

  ASSERT_TRUE(common::testing::createNewEmptyFile(fileExistingRt));
  EXPECT_TRUE(exists(fileExistingRt));
  std::string fileExistingRt2 = uniqueFilePath(newFile, "txt");
  EXPECT_EQ(fileExistingRt2, newFile + "(2).txt");

  // Cleanup
  EXPECT_TRUE(removeFile(newFileWithExt)) << newFileWithExt;
  EXPECT_TRUE(removeFile(fileExistingRt)) << fileExistingRt;
  EXPECT_TRUE(removeDirectory(dir)) << dir;
  EXPECT_TRUE(removeDirectory(dirExistingRt)) << dirExistingRt;
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, separator)
{
#ifndef _WIN32
  EXPECT_EQ("/", gz::common::separator(""));
#else
  EXPECT_EQ("\\", gz::common::separator(""));
#endif
}

/////////////////////////////////////////////////
TEST_F(FilesystemTest, empty)
{
#ifdef __APPLE__
  EXPECT_EQ(common::absPath(""), gz::common::cwd() + "/");
#else
  EXPECT_EQ(common::absPath(""), "");
#endif
}
