
// Copyright (c) 2024-2026 Carologistics
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cx_utils/clips_env_context.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>

#include "rclcpp/logging.hpp"

namespace cx
{
constexpr const char * RED = "\033[31m";
constexpr const char * GREEN = "\033[32m";
constexpr const char * YELLOW = "\033[33m";
constexpr const char * BLUE = "\033[34m";
constexpr const char * MAGENTA = "\033[35m";
constexpr const char * CYAN = "\033[36m";
constexpr const char * WHITE = "\033[37m";
constexpr const char * BOLD = "\033[1m";
constexpr const char * RESET = "\033[0m";

CLIPSLogger::CLIPSLogger(const char * component, bool log_to_file, bool stdout_to_debug)
: component_(strdup(component)),
  logger_(rclcpp::get_logger(std::string(component_))),
  stdout_to_debug_(stdout_to_debug)
{
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);

  if (log_to_file) {
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time), "%Y-%m-%d-%H-%M-%S");

    std::string formatted_time = oss.str();
    std::string base_name = component_ ? std::string(component_) : "clips";
#if defined(HUMBLE) || defined(JAZZY)
    std::string log_dir = rclcpp::get_logging_directory().string();
#else
    std::string log_dir = rclcpp::get_log_directory().string();
#endif
    std::string log_filename = log_dir + "/" + base_name + "_" + formatted_time + ".log";
    clips_logger_ = spdlog::basic_logger_st(base_name, log_filename);
    std::string symlink_path = log_dir + "/" + base_name + "_latest.log";

    namespace fs = std::filesystem;
    try {
      if (fs::exists(symlink_path) || fs::is_symlink(symlink_path)) {
        fs::remove(symlink_path);
      }
      fs::create_symlink(log_filename, symlink_path);
    } catch (const std::exception & e) {
      std::cerr << "Failed to create symlink: " << e.what() << std::endl;
    }
  } else {
    // Disable the logger by setting the log level to a level that filters out
    // all messages
    clips_logger_ = spdlog::stdout_color_mt("console");
    clips_logger_->set_level(spdlog::level::off);
  }
}

CLIPSLogger::~CLIPSLogger()
{
  if (component_) {
    free(component_);
  }
}
void CLIPSLogger::log(const char * logical_name, const char * str)
{
  if (!str || str[0] == '\0') {
    return;
  }

  // Always buffer raw text only
  buffer_ += str;

  size_t pos;
  while ((pos = buffer_.find('\n')) != std::string::npos) {
    // Extract one complete line (without newline)
    std::string line = buffer_.substr(0, pos);

    // Remove processed part (+ newline)
    buffer_.erase(0, pos + 1);

    // Ignore stdin
    if (strcmp(logical_name, clips::STDIN) == 0) {
      continue;
    }

    // ------------------------
    // Apply color at emit time
    // ------------------------
    std::string colored_line = line;

    if (strcmp(logical_name, "red") == 0) {
      colored_line = std::string(cx::RED) + line + cx::RESET;
    } else if (strcmp(logical_name, "bold") == 0) {
      colored_line = std::string(cx::BOLD) + line + cx::RESET;
    } else if (strcmp(logical_name, "green") == 0) {
      colored_line = std::string(cx::GREEN) + line + cx::RESET;
    } else if (strcmp(logical_name, "yellow") == 0) {
      colored_line = std::string(cx::YELLOW) + line + cx::RESET;
    } else if (strcmp(logical_name, "blue") == 0) {
      colored_line = std::string(cx::BLUE) + line + cx::RESET;
    } else if (strcmp(logical_name, "magenta") == 0) {
      colored_line = std::string(cx::MAGENTA) + line + cx::RESET;
    } else if (strcmp(logical_name, "cyan") == 0) {
      colored_line = std::string(cx::CYAN) + line + cx::RESET;
    } else if (strcmp(logical_name, "white") == 0) {
      colored_line = std::string(cx::WHITE) + line + cx::RESET;
    }

    // ------------------------
    // ROS logging level routing
    // ------------------------
    if (
      strcmp(logical_name, "debug") == 0 || strcmp(logical_name, "logdebug") == 0 ||
      (stdout_to_debug_ && strcmp(logical_name, clips::STDOUT) == 0)) {
      RCLCPP_DEBUG(this->logger_, "%s", colored_line.c_str());

    } else if (
      strcmp(logical_name, "warn") == 0 || strcmp(logical_name, "logwarn") == 0 ||
      strcmp(logical_name, clips::STDWRN) == 0) {
      RCLCPP_WARN(this->logger_, "%s", colored_line.c_str());

    } else if (
      strcmp(logical_name, "error") == 0 || strcmp(logical_name, "logerror") == 0 ||
      strcmp(logical_name, clips::STDERR) == 0) {
      RCLCPP_ERROR(this->logger_, "%s", colored_line.c_str());

    } else {
      RCLCPP_INFO(this->logger_, "%s", colored_line.c_str());
    }

    // ------------------------
    // File logging (plain text)
    // ------------------------
    clips_logger_->info("{}", line);
  }
}

CLIPSEnvContext * CLIPSEnvContext::get_context(clips::Environment * env)
{
  using clips::environmentData;
  return static_cast<CLIPSEnvContext *>(GetEnvironmentData(env, USER_ENVIRONMENT_DATA));
}

CLIPSEnvContext * CLIPSEnvContext::get_context(std::shared_ptr<clips::Environment> & env)
{
  using clips::environmentData;
  return static_cast<CLIPSEnvContext *>(GetEnvironmentData(env.get(), USER_ENVIRONMENT_DATA));
}

}  // namespace cx
