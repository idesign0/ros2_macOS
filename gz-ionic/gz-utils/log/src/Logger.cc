/*
 * Copyright (C) 2024 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include <gz/utils/log/Logger.hh>
#include <gz/utils/log/SplitSink.hh>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/spdlog.h>

namespace gz::utils::log
{
namespace {
  /// \brief Default log format
  /// Example output
constexpr std::string_view kDefaultLogFormat{
    "%^(%Y-%m-%d %T.%e) [%l] [%s:%#] %v%$"};
}
/// \brief Private data for the Logger class.
class Logger::Implementation
{
  /// \brief Constructor.
  /// \param[in] _loggerName Logger name.
  public: explicit Implementation(const std::string &_loggerName)
    : consoleSink(std::make_shared<SplitConsoleSink>()),
      sinks(std::make_shared<spdlog::sinks::dist_sink_mt>()),
      formatter(std::make_unique<spdlog::pattern_formatter>(
            kDefaultLogFormat.data(),
            spdlog::pattern_time_type::local, std::string(""))),
      logger(std::make_shared<spdlog::logger>(_loggerName, sinks))
  {
  }

  /// \brief The console sink with stdout and stderr.
  std::shared_ptr<SplitConsoleSink> consoleSink;

  /// \brief The file sink for logging into a file.
  std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileSink {nullptr};

  /// \brief A sink distribution storing multiple sinks.
  std::shared_ptr<spdlog::sinks::dist_sink_mt> sinks {nullptr};

  /// \brief Common formatter for both all sinks
  std::unique_ptr<spdlog::pattern_formatter> formatter;

  /// \brief The underlying spdlog logger.
  std::shared_ptr<spdlog::logger> logger {nullptr};
};

/////////////////////////////////////////////////
Logger::Logger(const std::string &_loggerName)
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>(_loggerName))
{
  // Add the console sink by default.
  this->dataPtr->sinks->add_sink(this->dataPtr->consoleSink);

  // Configure the logger.
  this->dataPtr->logger->set_level(spdlog::level::trace);
  this->dataPtr->consoleSink->set_level(spdlog::level::err);
  this->dataPtr->logger->flush_on(spdlog::level::debug);

  this->dataPtr->logger->set_formatter(this->dataPtr->formatter->clone());
}

/////////////////////////////////////////////////
void Logger::SetLogDestination(const std::string &_filename)
{
  if (this->dataPtr->fileSink)
    this->dataPtr->sinks->remove_sink(this->dataPtr->fileSink);

  if (!_filename.empty())
  {
    try
    {
      this->dataPtr->fileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(_filename, true);
      this->dataPtr->fileSink->set_formatter(this->dataPtr->formatter->clone());
      this->dataPtr->fileSink->set_level(spdlog::level::trace);
      this->dataPtr->sinks->add_sink(this->dataPtr->fileSink);
    }
    catch (const std::exception &_e)
    {
      std::cerr << "Error creating log file: " << _e.what() << std::endl;
    }
  }
}

/////////////////////////////////////////////////
std::string Logger::LogDestination() const
{
  std::string logPath = "";
  if (this->dataPtr->fileSink)
    logPath = this->dataPtr->fileSink->filename();

  return logPath;
}

/////////////////////////////////////////////////
spdlog::logger &Logger::RawLogger() const
{
  return *this->dataPtr->logger;
}

/////////////////////////////////////////////////
std::shared_ptr<spdlog::logger> Logger::RawLoggerPtr() const
{
  return this->dataPtr->logger;
}

/////////////////////////////////////////////////
void Logger::SetConsoleSinkLevel(spdlog::level::level_enum _level)
{
  if (this->dataPtr->consoleSink)
  {
    this->dataPtr->consoleSink->set_level(_level);
  }
}

}  // namespace gz::utils::log
