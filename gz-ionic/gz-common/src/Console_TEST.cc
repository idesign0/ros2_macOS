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
#include <stdlib.h>

#include "gz/common/Console.hh"
#include "gz/common/Filesystem.hh"
#include "gz/common/TempDirectory.hh"
#include "gz/common/Util.hh"

using namespace gz;
using namespace common;

const int g_messageRepeat = 4;

class Console_TEST : public ::testing::Test {
  protected: virtual void SetUp()
  {
    this->temp = std::make_unique<TempDirectory>(
        "test", "gz_common", true);
    ASSERT_TRUE(this->temp->Valid());
    common::setenv(GZ_HOMEDIR, this->temp->Path());
  }

  /// \brief Clear out all the directories we produced during this test.
  public: virtual void TearDown()
  {
    gzLogClose();
    EXPECT_TRUE(common::unsetenv(GZ_HOMEDIR));
  }

  /// \brief Temporary directory to run test in
  private: std::unique_ptr<common::TempDirectory> temp;
};

/////////////////////////////////////////////////
std::string GetLogContent(const std::string &_filename)
{
  // Get the absolute path
  std::string path;
  EXPECT_TRUE(common::env(GZ_HOMEDIR, path));
  path = common::joinPaths(path, _filename);
  EXPECT_TRUE(common::exists(path));

  // Open the log file, and read back the string
  std::ifstream ifs(path.c_str(), std::ios::in);
  std::string loggedString;

  while (!ifs.eof())
  {
    std::string line;
    std::getline(ifs, line);
    loggedString += line;
  }

  return loggedString;
}

/////////////////////////////////////////////////
/// \brief Test Console::Init and Console::Log
TEST_F(Console_TEST, NoInitAndLog)
{
  // Log the string
  std::string logString = "this is a test";
  gzlog << logString << std::endl;

  // Get the absolute log file path
  std::string logPath = ".gz/auto_default.log";

  common::Console::Root().RawLogger().flush();
  // Expect to find the string in the log file
  EXPECT_TRUE(GetLogContent(logPath).find(logString) != std::string::npos);

  // Cleanup
  gzLogClose();
  std::string path;
  EXPECT_TRUE(common::env(GZ_HOMEDIR, path));
  path = common::joinPaths(path, logPath);

  // This is causing an issue on Windows as the resource is busy,
  // probably locked by spdlog.
  common::removeAll(path);
}

/////////////////////////////////////////////////
/// \brief Test Console::Init and Console::Log
TEST_F(Console_TEST, InitAndLog)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Log the string
  std::string logString = "this is a test";
  gzlog << logString << std::endl;

  // Get the absolute path
  std::string basePath;
  EXPECT_TRUE(common::env(GZ_HOMEDIR, basePath));
  basePath = common::joinPaths(basePath, path);

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  common::Console::Root().RawLogger().flush();
  // Expect to find the string in the log file
  EXPECT_TRUE(GetLogContent(logPath).find(logString) != std::string::npos);

  // Cleanup
  gzLogClose();

  // This is causing an issue on Windows as the resource is busy,
  // probably locked by spdlog.
  common::removeAll(basePath);
}

//////////////////////////////////////////////////
/// \brief Test Console::Log with \n characters
TEST_F(Console_TEST, LogSlashN)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a log test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzlog << logString << " _n__ " << i << '\n';
  }

  common::Console::Root().RawLogger().flush();
  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::Log with std::endl
TEST_F(Console_TEST, LogStdEndl)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a log test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzlog << logString << " endl " << i << std::endl;
  }

  common::Console::Root().RawLogger().flush();
  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorWarn with \n characters
TEST_F(Console_TEST, ColorWarnSlashN)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a warning test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzwarn << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorWarn with std::endl
TEST_F(Console_TEST, ColorWarnStdEndl)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a warning test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzwarn << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorDbg with \n characters
TEST_F(Console_TEST, ColorDbgSlashN)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a dbg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzdbg << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorDbg with std::endl
TEST_F(Console_TEST, ColorDbgStdEndl)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a dbg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzdbg << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorMsg with \n characters
TEST_F(Console_TEST, ColorMsgSlashN)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a msg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzmsg << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorMsg with std::endl
TEST_F(Console_TEST, ColorMsgStdEndl)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a msg test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzmsg << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorErr with \n characters
TEST_F(Console_TEST, ColorErrSlashN)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is an error test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzerr << logString << " _n__ " << i << '\n';
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " _n__ " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

//////////////////////////////////////////////////
/// \brief Test Console::ColorErr with std::endl
TEST_F(Console_TEST, ColorErrStdEndl)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is an error test";

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    gzerr << logString << " endl " << i << std::endl;
  }

  std::string logContent = GetLogContent(logPath);

  for (int i = 0; i < g_messageRepeat; ++i)
  {
    std::ostringstream stream;
    stream << logString << " endl " << i;
    EXPECT_TRUE(logContent.find(stream.str()) != std::string::npos);
  }
}

/////////////////////////////////////////////////
/// \brief Test Console::ColorMsg
TEST_F(Console_TEST, ColorMsg)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is a msg test";

  gzmsg << logString << std::endl;

  std::string logContent = GetLogContent(logPath);

  EXPECT_TRUE(logContent.find(logString) != std::string::npos);
}

/////////////////////////////////////////////////
/// \brief Test Console::ColorErr
TEST_F(Console_TEST, ColorErr)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  // Get the absolute log file path
  std::string logPath = common::joinPaths(path, "test.log");

  std::string logString = "this is an error test";

  gzerr << logString << std::endl;

  std::string logContent = GetLogContent(logPath);

  EXPECT_TRUE(logContent.find(logString) != std::string::npos);
}

/////////////////////////////////////////////////
/// \brief Test Console::Verbosity
TEST_F(Console_TEST, Verbosity)
{
  EXPECT_EQ(1, common::Console::Verbosity());

  common::Console::SetVerbosity(2);
  EXPECT_EQ(2, common::Console::Verbosity());

  common::Console::SetVerbosity(-1);
  EXPECT_EQ(2, common::Console::Verbosity());

  common::Console::SetVerbosity(1000);
  EXPECT_EQ(5, common::Console::Verbosity());
}

/////////////////////////////////////////////////
/// \brief Test Console::Prefix
TEST_F(Console_TEST, Prefix)
{
  // Max verbosity
  common::Console::SetVerbosity(4);

  // Path to log file
  auto path = common::uuid();

  gzLogInit(path, "test.log");
  std::string logPath = common::joinPaths(path, "test.log");

  // Check default prefix
  EXPECT_EQ(common::Console::Prefix(), "");

  // Set new prefix
  common::Console::SetPrefix("**test** ");
  EXPECT_EQ(common::Console::Prefix(), "**test** ");

  // Use the console
  gzerr << "error" << std::endl;
  gzwarn << "warning" << std::endl;
  gzmsg << "message" << std::endl;
  gzdbg << "debug" << std::endl;

  // Get the logged content
  std::string logContent = GetLogContent(logPath);

  // Check
  EXPECT_TRUE(logContent.find("**test** error") != std::string::npos);
  EXPECT_TRUE(logContent.find("**test** warning") != std::string::npos);
  EXPECT_TRUE(logContent.find("**test** message") != std::string::npos);
  EXPECT_TRUE(logContent.find("**test** debug") != std::string::npos);

  // Reset
  common::Console::SetPrefix("");
  EXPECT_EQ(common::Console::Prefix(), "");
}

/////////////////////////////////////////////////
/// \brief Test Console::LogDirectory
TEST_F(Console_TEST, LogDirectory)
{
  // Create a unique directory path
  auto path = common::uuid();

  // Initialize logging
  gzLogInit(path, "test.log");

  std::string logDir = gzLogDirectory();

  // Get the absolute path
  std::string absPath;
  EXPECT_TRUE(common::env(GZ_HOMEDIR, absPath));
  absPath = common::joinPaths(absPath, path);

  EXPECT_EQ(logDir, absPath);
}

/////////////////////////////////////////////////
/// \brief Test Console::Init and Console::Log
/// This specifically tests with an unset HOME variable
TEST_F(Console_TEST, NoInitAndLogNoHome)
{
  gzLogClose();
  EXPECT_TRUE(common::unsetenv(GZ_HOMEDIR));
  // This should not throw
  gzlog << "this is a test" << std::endl;
}
