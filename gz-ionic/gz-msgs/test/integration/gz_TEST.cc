/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <filesystem>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
#include <gz/msgs/config.hh>
#include <gz/utils/ExtraTestMacros.hh>

#ifdef _MSC_VER
#    define popen _popen
#    define pclose _pclose
#endif

// Set from preprocessor defines
static constexpr const char * kMsgsVersion = GZ_MSGS_VERSION_FULL;
static constexpr const char * kExecutablePath = GZ_MSGS_EXECUTABLE_PATH;
static constexpr const char * kCompletionScriptPath =
  GZ_MSGS_COMPLETION_SCRIPT_PATH;

/////////////////////////////////////////////////
std::string make_exec_string(const std::string &_args)
{
  return std::string(kExecutablePath) + " " + _args;
}

/////////////////////////////////////////////////
std::string custom_exec_str(const std::string &_cmd)
{
  FILE *pipe = popen(_cmd.c_str(), "r");

  if (!pipe)
    return "ERROR";

  char buffer[128];
  std::string result;

  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }

  pclose(pipe);
  return result;
}

/////////////////////////////////////////////////
TEST(CmdLine, Version)
{
  auto output = custom_exec_str(make_exec_string("--version"));
  EXPECT_NE(std::string::npos, output.find(kMsgsVersion));
}

/////////////////////////////////////////////////
TEST(CmdLine, Help)
{
  {
    // Full argument
    auto output = custom_exec_str(make_exec_string("--help"));
    EXPECT_NE(std::string::npos, output.find("list"));
    EXPECT_NE(std::string::npos, output.find("info"));
  }

  {
    // Short flag
    auto output = custom_exec_str(make_exec_string("-h"));
    EXPECT_NE(std::string::npos, output.find("list"));
    EXPECT_NE(std::string::npos, output.find("info"));
  }

  {
    // Print help with no other args
    auto output = custom_exec_str(make_exec_string(""));
    EXPECT_NE(std::string::npos, output.find("list"));
    EXPECT_NE(std::string::npos, output.find("info"));
  }
}

/////////////////////////////////////////////////
TEST(CmdLine, MsgList)
{
  auto output = custom_exec_str(make_exec_string("--list"));
  EXPECT_NE(std::string::npos, output.find("gz.msgs.WorldControl"));
}

/////////////////////////////////////////////////
TEST(CmdLine, MsgInfo)
{
  // Underscore separated
  {
    auto output =
      custom_exec_str(make_exec_string("--info gz_msgs.WorldControl"));
    EXPECT_NE(std::string::npos, output.find("message WorldControl {"))
      << output;
  }

  // Period separated
  {
    auto output =
      custom_exec_str(make_exec_string("--info gz.msgs.WorldControl"));
    EXPECT_NE(std::string::npos, output.find("message WorldControl {"))
      << output;
  }

  // Multiple arguments
  {
    auto output = custom_exec_str(make_exec_string(
      "--info gz.msgs.WorldControl gz.msgs.Wrench"));
    EXPECT_NE(std::string::npos, output.find("message WorldControl {"));
    EXPECT_NE(std::string::npos, output.find("message Wrench {"));
  }
}

/////////////////////////////////////////////////
TEST(CmdLine, GZ_UTILS_TEST_DISABLED_ON_WIN32(MsgHelpVsCompletionFlags))
{
  // Flags in help message
  auto helpOutput = custom_exec_str(make_exec_string("--help"));

  // Equivalent to:
  // sh -c "bash -c \". /path/to/msgs.bash_completion.sh; _gz_msgs_flags\""
  std::string cmd = "bash -c \". "
    + std::string(kCompletionScriptPath)
    + "; _gz_msgs_flags\"";
  std::string scriptOutput = custom_exec_str(cmd);

  // Tokenize script output
  std::istringstream iss(scriptOutput);
  std::vector<std::string> flags((std::istream_iterator<std::string>(iss)),
    std::istream_iterator<std::string>());

  EXPECT_GT(flags.size(), 0u);

  // Match each flag in script output with help message
  for (const auto &flag : flags)
  {
    EXPECT_NE(std::string::npos, helpOutput.find(flag)) << helpOutput;
  }
}
