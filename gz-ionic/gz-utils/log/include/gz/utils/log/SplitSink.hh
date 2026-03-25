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

#ifndef GZ_UTILS_LOG_SPLITSINK_HH_
#define GZ_UTILS_LOG_SPLITSINK_HH_

#include <memory>
#include <string>

#include <gz/utils/config.hh>
#include <gz/utils/ImplPtr.hh>
#include <gz/utils/log/Export.hh>

#include <spdlog/details/log_msg.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace gz
{
namespace utils
{
namespace log
{
inline namespace GZ_UTILS_VERSION_NAMESPACE {

/// \brief Logging sink for spdlog that logs in Gazebo-conventions.
///
/// This will route messages with severity (warn, err, critical) to stderr,
/// and all other levels (info, debug, trace) to stdout.
class GZ_UTILS_LOG_VISIBLE SplitConsoleSink : public spdlog::sinks::sink
{
  /// \brief Class constructor.
  public: SplitConsoleSink();

  /// \brief Class destructor.
  public: ~SplitConsoleSink() override = default;

  /// \brief Log a message.
  /// \param[in] _msg The message to log.
  public: void log(const spdlog::details::log_msg &_msg) override;

  /// \brief Flush messages.
  public: void flush() override;

  /// \brief Set the logging pattern.
  /// \param[in] _pattern The logging pattern.
  public: void set_pattern(const std::string &_pattern) override;

  /// \brief Set the new formatter.
  /// \param[in] _sinkFormatter The formatter.
  public: void set_formatter(std::unique_ptr<spdlog::formatter> _sinkFormatter)
    override;

  /// \brief Set the color mode.
  /// \param[in] _mode Color mode.
  public: void set_color_mode(spdlog::color_mode _mode);

  /// \brief Implementation Pointer.
  GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
};
}  // namespace GZ_UTILS_LOG_VERSION_NAMESPACE
}  // namespace log
}  // namespace utils
}  // namespace gz

#endif  // GZ_UTILS_LOG_SPLITSINK_HH__
