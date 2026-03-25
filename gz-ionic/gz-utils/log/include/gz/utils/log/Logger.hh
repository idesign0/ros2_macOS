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
#ifndef GZ_UTILS_LOG_LOGGER_HH_
#define GZ_UTILS_LOG_LOGGER_HH_

#if !defined(SPDLOG_ACTIVE_LEVEL)
  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <gz/utils/config.hh>
#include <gz/utils/ImplPtr.hh>
#include <gz/utils/log/Export.hh>

namespace gz
{
namespace utils
{
namespace log
{
inline namespace GZ_UTILS_VERSION_NAMESPACE {

/// \brief Gazebo console and file logging class.
/// This will configure spdlog with a sane set of defaults for logging to the
/// console as well as a file.
class GZ_UTILS_LOG_VISIBLE Logger
{
  /// \brief Class constructor.
  /// \param[in] _loggerName Logger name.
  public: explicit Logger(const std::string &_loggerName);

  /// \brief Set the log destination filename.
  /// \param[in] _filename Log file name.
  public: void SetLogDestination(const std::string &_filename);

  /// \brief Get the log destination filename.
  /// \return Log file name.
  public: std::string LogDestination() const;

  /// \brief Access the underlying spdlog logger.
  /// \return The spdlog logger.
  public: [[nodiscard]] spdlog::logger &RawLogger() const;

  /// \brief Access the underlying spdlog logger, with ownership.
  /// \return The spdlog logger.
  public: [[nodiscard]] std::shared_ptr<spdlog::logger> RawLoggerPtr() const;

  /// \brief Set the severity level of the Console sink
  /// \param [in] _level Severity level
  public: void SetConsoleSinkLevel(spdlog::level::level_enum _level);

  /// \brief Implementation Pointer.
  GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
};
}  // namespace GZ_UTILS_LOG_VERSION_NAMESPACE
}  // namespace log
}  // namespace utils
}  // namespace gz

#endif  // GZ_UTILS_LOG_LOGGER_HH_
