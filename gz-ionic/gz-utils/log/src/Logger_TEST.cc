/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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
#include <spdlog/logger.h>
#include <spdlog/sinks/sink.h>
#include <filesystem>
#include <fstream>
#include <gz/utils/log/Logger.hh>

/////////////////////////////////////////////////
std::string getLogContent(const std::string &_filename)
{
  // Open the log file, and read back the string
  std::ifstream ifs(_filename.c_str(), std::ios::in);
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
TEST(Logger, defaults)
{
  gz::utils::log::Logger logger("my_logger");

  // Logger defaults.
  EXPECT_TRUE(logger.RawLogger().should_log(spdlog::level::trace));
  EXPECT_EQ(spdlog::level::debug, logger.RawLogger().flush_level());
  EXPECT_EQ(spdlog::level::debug, logger.RawLoggerPtr()->flush_level());

  // Sink defaults.
  EXPECT_EQ(1u, logger.RawLogger().sinks().size());
  auto sink = logger.RawLogger().sinks().front();
  ASSERT_NE(nullptr, sink);
}

/////////////////////////////////////////////////
TEST(Logger, basicLogging)
{
  gz::utils::log::Logger logger("my_logger");

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  SPDLOG_LOGGER_TRACE(logger.RawLoggerPtr(), "trace\n");
  SPDLOG_LOGGER_DEBUG(logger.RawLoggerPtr(), "debug\n");
  SPDLOG_LOGGER_INFO(logger.RawLoggerPtr(), "info\n");
  SPDLOG_LOGGER_WARN(logger.RawLoggerPtr(), "warn\n");
  SPDLOG_LOGGER_ERROR(logger.RawLoggerPtr(), "error\n");
  SPDLOG_LOGGER_CRITICAL(logger.RawLoggerPtr(), "critical\n");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"trace", "debug", "info"})
    EXPECT_FALSE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn"})
    EXPECT_FALSE(stdErr.find(word) != std::string::npos);

  for (auto word : {"error", "critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST(Logger, consoleSinkLevel)
{
  gz::utils::log::Logger logger("my_logger");
  logger.SetConsoleSinkLevel(spdlog::level::critical);

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  SPDLOG_LOGGER_TRACE(logger.RawLoggerPtr(), "trace\n");
  SPDLOG_LOGGER_DEBUG(logger.RawLoggerPtr(), "debug\n");
  SPDLOG_LOGGER_INFO(logger.RawLoggerPtr(), "info\n");
  SPDLOG_LOGGER_WARN(logger.RawLoggerPtr(), "warn\n");
  SPDLOG_LOGGER_ERROR(logger.RawLoggerPtr(), "error\n");
  SPDLOG_LOGGER_CRITICAL(logger.RawLoggerPtr(), "critical\n");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"trace", "debug", "info"})
    EXPECT_FALSE(stdOut.find(word) != std::string::npos);

  for (auto word : {"warn", "error"})
    EXPECT_FALSE(stdErr.find(word) != std::string::npos);

  for (auto word : {"critical"})
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
}

/////////////////////////////////////////////////
TEST(Logger, fileLocation)
{
  gz::utils::log::Logger logger("my_logger");

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();

  SPDLOG_LOGGER_ERROR(logger.RawLoggerPtr(), "error\n");
  SPDLOG_LOGGER_CRITICAL(logger.RawLoggerPtr(), "critical\n");

  std::string stdOut = testing::internal::GetCapturedStdout();
  std::string stdErr = testing::internal::GetCapturedStderr();

  for (auto word : {"error", "critical"})
  {
    EXPECT_TRUE(stdErr.find(word) != std::string::npos);
    EXPECT_TRUE(stdErr.find("Logger_TEST.cc:") != std::string::npos);
  }
}

/////////////////////////////////////////////////
TEST(Logger, fileLogging)
{
  gz::utils::log::Logger logger("my_logger");

  std::filesystem::path logDir = std::filesystem::temp_directory_path();
  std::filesystem::path logFile = "my_log.txt";
  std::filesystem::path logPath = logDir / logFile;

  EXPECT_TRUE(logger.LogDestination().empty());
  logger.SetLogDestination(logPath.string());
  EXPECT_EQ(logPath.string(), logger.LogDestination());

  logger.RawLogger().trace("trace\n");
  logger.RawLogger().debug("debug\n");
  logger.RawLogger().info("info\n");
  logger.RawLogger().warn("warn\n");
  logger.RawLogger().error("error\n");
  logger.RawLogger().critical("critical\n");

  // Expect to find the string in the log file
  std::string logContent = getLogContent(logPath.string());
  for (auto word : {"trace", "debug", "info", "warn", "error", "critical"})
    EXPECT_TRUE(logContent.find(word) != std::string::npos);
}
