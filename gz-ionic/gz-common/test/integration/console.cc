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

#include <gz/common/Console.hh>
#include <gz/common/testing/TestPaths.hh>

//////////////////////////////////////////////////
TEST(Console_TEST, LogInitAfterConsoleOut)
{
  std::string logFilename = "uri.log";
  auto tempDir = gz::common::testing::MakeTestTempDirectory();
  ASSERT_TRUE(tempDir->Valid());

  gz::common::setenv(GZ_HOMEDIR, tempDir->Path());
  std::string home;
  ASSERT_TRUE(gz::common::env(GZ_HOMEDIR, home));

  auto logDir = tempDir->Path();
  std::string logFile = gz::common::joinPaths(logDir, logFilename);

  gz::common::Console::SetVerbosity(4);

  // Before console is initialized, logs go to default file location
  gzerr << "Error before logging initialized" << std::endl;

  {
    auto defaultPath = gz::common::joinPaths(home,
        ".gz", "auto_default.log");

    std::ifstream t(defaultPath);
    std::string buffer((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

    EXPECT_NE(
        std::string::npos, buffer.find("Error before logging initialized"))
      << "Log file content[" << buffer << "]\n";
  }

  // Initialize the log file.
  gzLogInit(logDir, logFilename);

  // After consolie is initialized, logs go to designated location
  gzerr << "Error after logging initialized" << std::endl;

  {
    std::ifstream t(logFile);
    std::string buffer((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

    EXPECT_NE(
        std::string::npos, buffer.find("Error after logging initialized"))
      << "Log file content[" << buffer << "]\n";
  }
}
