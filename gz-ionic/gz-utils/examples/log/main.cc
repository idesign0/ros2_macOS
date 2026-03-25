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

#include <filesystem>
#include <gz/utils/log/Logger.hh>

//////////////////////////////////////////////////
int main(int argc, char** argv)
{
  gz::utils::log::Logger logger("my_logger");
  logger.SetConsoleSinkLevel(spdlog::level::info);

  std::filesystem::path logDir = std::filesystem::temp_directory_path();
  std::filesystem::path logFile = "my_log.txt";
  std::filesystem::path logPath = logDir / logFile;

  logger.SetLogDestination(logPath);

  SPDLOG_LOGGER_TRACE(logger.RawLoggerPtr(), "trace\n");
  SPDLOG_LOGGER_DEBUG(logger.RawLoggerPtr(), "debug\n");
  SPDLOG_LOGGER_INFO(logger.RawLoggerPtr(), "info\n");
  SPDLOG_LOGGER_WARN(logger.RawLoggerPtr(), "warn\n");
  SPDLOG_LOGGER_ERROR(logger.RawLoggerPtr(), "error\n");
  SPDLOG_LOGGER_CRITICAL(logger.RawLoggerPtr(), "critical\n");
}
