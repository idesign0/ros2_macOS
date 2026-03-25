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
#include <memory>
#include <string>

#include <gz/utils/log/SplitSink.hh>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace gz::utils::log
{
/// \brief Private data for the SplitConsoleSink class.
class SplitConsoleSink::Implementation
{
  /// \brief Constructor.
  /// \param[in] _loggerName Logger name.
  public: Implementation()
  {
  }

  /// \brief Standard output.
  public: spdlog::sinks::stdout_color_sink_mt stdout_;

  /// \brief Standard error.
  public: spdlog::sinks::stderr_color_sink_mt stderr_;
};

/////////////////////////////////////////////////
SplitConsoleSink::SplitConsoleSink()
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
}

/////////////////////////////////////////////////
void SplitConsoleSink::log(const spdlog::details::log_msg &_msg)
{
   if (_msg.level == spdlog::level::warn      ||
       _msg.level == spdlog::level::err       ||
       _msg.level == spdlog::level::critical)
    {
      this->dataPtr->stderr_.log(_msg);
    }
    else
      this->dataPtr->stdout_.log(_msg);
}

/////////////////////////////////////////////////
void SplitConsoleSink::flush()
{
  this->dataPtr->stdout_.flush();
  this->dataPtr->stderr_.flush();
}

/////////////////////////////////////////////////
void SplitConsoleSink::set_pattern(const std::string &_pattern)
{
  this->dataPtr->stdout_.set_pattern(_pattern);
  this->dataPtr->stderr_.set_pattern(_pattern);
}

/////////////////////////////////////////////////
void SplitConsoleSink::set_formatter(
  std::unique_ptr<spdlog::formatter> _sinkFormatter)
{
  this->dataPtr->stdout_.set_formatter(_sinkFormatter->clone());
  this->dataPtr->stderr_.set_formatter(std::move(_sinkFormatter));
}

/////////////////////////////////////////////////
void SplitConsoleSink::set_color_mode(spdlog::color_mode _mode)
{
  this->dataPtr->stdout_.set_color_mode(_mode);
  this->dataPtr->stderr_.set_color_mode(_mode);
}

}  // namespace gz::utils::log
